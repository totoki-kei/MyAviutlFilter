#pragma once

#include <Windows.h>
#include <memory>
#include <vector>
#include "RefPtr.h"
#include "LinkList.h"

class MemMap {
	static const size_t Alignment = sizeof(int);

	template <typename TLeft, typename TRight>
	static constexpr auto CalcAlignment(TLeft address, TRight alignment) -> auto{
		return alignment * ((address + alignment - 1) / alignment);
	}
	template <typename T>
	static constexpr auto CalcAlignment(T address) -> auto{
		return CalcAlignment(address, Alignment);
	}


	struct Block;
	struct Page;
	struct FileMap;

	struct Block : public LinkList::Node {
		RefPtr<FileMap> filemap; // 所属しているファイルマップ
		DWORD page; // ブロックが存在するページ番号
		DWORD position; // ページ内オフセット(アライン済み)
		DWORD size; // ブロックのサイズ(アライン済み)
		DWORD ref_count; // ブロックを参照しているハンドルの数 これが0の場合は未使用の空きブロックと見なす
		PVOID mapped;

		Block();

		~Block();

		bool AddRef();

		void Release();

		bool Lock();

		void Unlock();

		void CopyTo(Block& dest);
	};

	struct Page {
		DWORD used; // このページの領域のうち使用済みデータ量
		DWORD lock_count;
		PVOID mapped;

	};

	struct FileMap {
		HANDLE filemap_;
		DWORD filemap_page_size_;
		DWORD filemap_page_count_; // ページ数
		std::unique_ptr<FileMap> next_;

		std::vector<Page> pages_; // 現在のファイルマップのページのリスト
		LinkList block_list_; // メモリブロックのリスト

		DWORD ref_count_;

		FileMap(DWORD size, DWORD allocation_glanularity /* = sysinfo_.dwAllocationGranularity */);

		~FileMap();

		RefPtr<Block> AllocateNewBlock(size_t size);

		bool AddRef();

		void Release();

		void* LockPage(DWORD page_index);

		void UnlockPage(DWORD page_index);
	};


	std::unique_ptr<FileMap> map_;
	DWORD map_size_;
	SYSTEM_INFO sysinfo_;


public:
	class Handle;

	template <typename T>
	class Pointer {
		friend class Handle;
		RefPtr<Block> block_;

		Pointer(const RefPtr<Block>& block) : block_(block) {
			if (!block_->Lock()) block_.Reset(nullptr);
		}

		Pointer(RefPtr<Block>&& block) : block_(std::move(block)) {
			// moveの場合はロック状態を継承するため追加でロックしない
		}

	public:
		Pointer() {}
		Pointer(const Pointer& p) : Pointer(p.block_) { }
		Pointer(Pointer&& p) : Pointer(std::move(p.block_)) {}
		~Pointer() { }

		Pointer& operator = (const Pointer& p) {
			if (this != &p) {
				this->block_ = p.block_;
				if (this->block_) {
					if (!this->block_->Lock()) block_.Reset();
				}
			}
			return *this;
		}
		Pointer& operator = (Pointer&& p) {
			if (this != &p) {
				this->block_.Swap(p.block_);
			}
			return *this;
		}


		T& operator *() {
			return *static_cast<T*>(block_->mapped);
		}

		T* operator ->() {
			return static_cast<T*>(block_->mapped);
		}

		operator bool() const {
			return (block_ && block_->mapped);
		}

		operator T* () const {
			return static_cast<T*>(block_->mapped);
		}
	};

	template <>
	class Pointer<void> {
		friend class Handle;
		RefPtr<Block> block_;

		Pointer(const RefPtr<Block>& block) : block_(block) {
			if (!block_->Lock()) block_.Reset(nullptr);
		}

		Pointer(RefPtr<Block>&& block) : block_(std::move(block)) {
			// moveの場合はロック状態を継承するため追加でロックしない
		}

	public:
		Pointer() {}
		Pointer(const Pointer& p) : Pointer(p.block_) { }
		Pointer(Pointer&& p) : Pointer(std::move(p.block_)) {}
		~Pointer() { }

		Pointer& operator = (const Pointer& p) {
			if (this != &p) {
				this->block_ = p.block_;
				if (this->block_) {
					if (!this->block_->Lock()) block_.Reset(nullptr);
				}
			}
			return *this;
		}
		Pointer& operator = (Pointer&& p) {
			if (this != &p) {
				this->block_.Swap(std::move(p.block_));
			}
			return *this;
		}


		operator bool() const {
			return (block_ && block_->mapped);
		}

		operator void* () const {
			return block_->mapped;
		}
	};

	class Handle {
		friend class MemMap;

		RefPtr<Block> block_;

		// プライベートコンストラクタ：メモリマップ情報と対応するエントリへの参照を格納する
		//Handle(const RefPtr<Block>& block) : block_(block) { }
		Handle(RefPtr<Block>&& block);


	public:
		// パブリックコンストラクタ：デフォルトコンストラクタ
		Handle();

		// 領域に指定された型でアクセスする
		template <typename T>
		Pointer<T> Lock() {
			if (sizeof(T) > block_->size) return Pointer<T>();
			return Pointer<T>(this->block_);
		}

		template <>
		Pointer<void> Lock() {
			return Pointer<void>(this->block_);
		}

		DWORD GetSize() const;
	};

	//using KeyType = TKey;

	MemMap(size_t initial_size);

	~MemMap();

	// 指定サイズの領域を作成する
	Handle Allocate(size_t size);

	// 指定された型のサイズの領域を作成する
	template <typename T>
	Handle Allocate() {
		return Allocate(sizeof(T));
	}

	// 指定された型の配列のサイズで領域を作成する
	template <typename T, int Length>
	Handle Allocate() {
		return Allocate(sizeof(T[Length]));
	}
};
