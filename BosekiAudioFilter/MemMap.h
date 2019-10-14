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
		RefPtr<FileMap> filemap; // �������Ă���t�@�C���}�b�v
		DWORD page; // �u���b�N�����݂���y�[�W�ԍ�
		DWORD position; // �y�[�W���I�t�Z�b�g(�A���C���ς�)
		DWORD size; // �u���b�N�̃T�C�Y(�A���C���ς�)
		DWORD ref_count; // �u���b�N���Q�Ƃ��Ă���n���h���̐� ���ꂪ0�̏ꍇ�͖��g�p�̋󂫃u���b�N�ƌ��Ȃ�
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
		DWORD used; // ���̃y�[�W�̗̈�̂����g�p�ς݃f�[�^��
		DWORD lock_count;
		PVOID mapped;

	};

	struct FileMap {
		HANDLE filemap_;
		DWORD filemap_page_size_;
		DWORD filemap_page_count_; // �y�[�W��
		std::unique_ptr<FileMap> next_;

		std::vector<Page> pages_; // ���݂̃t�@�C���}�b�v�̃y�[�W�̃��X�g
		LinkList block_list_; // �������u���b�N�̃��X�g

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
			// move�̏ꍇ�̓��b�N��Ԃ��p�����邽�ߒǉ��Ń��b�N���Ȃ�
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
			// move�̏ꍇ�̓��b�N��Ԃ��p�����邽�ߒǉ��Ń��b�N���Ȃ�
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

		// �v���C�x�[�g�R���X�g���N�^�F�������}�b�v���ƑΉ�����G���g���ւ̎Q�Ƃ��i�[����
		//Handle(const RefPtr<Block>& block) : block_(block) { }
		Handle(RefPtr<Block>&& block);


	public:
		// �p�u���b�N�R���X�g���N�^�F�f�t�H���g�R���X�g���N�^
		Handle();

		// �̈�Ɏw�肳�ꂽ�^�ŃA�N�Z�X����
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

	// �w��T�C�Y�̗̈���쐬����
	Handle Allocate(size_t size);

	// �w�肳�ꂽ�^�̃T�C�Y�̗̈���쐬����
	template <typename T>
	Handle Allocate() {
		return Allocate(sizeof(T));
	}

	// �w�肳�ꂽ�^�̔z��̃T�C�Y�ŗ̈���쐬����
	template <typename T, int Length>
	Handle Allocate() {
		return Allocate(sizeof(T[Length]));
	}
};
