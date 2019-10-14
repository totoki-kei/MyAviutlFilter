#include "stdafx.h"
#include "MemMap.h"

MemMap::Block::Block()
	: page(0)
	, position(0)
	, size(0)
	, ref_count(0)
	, mapped(nullptr)
{}

MemMap::Block::~Block() {
	Unlock();
}

bool MemMap::Block::AddRef() {
	return ++ref_count;
}

void MemMap::Block::Release() {
	--ref_count;
	if (ref_count == 0) {
		Unlock();
	}

}

bool MemMap::Block::Lock() {
	if (!mapped) {
		void* ptr = filemap->LockPage(page);
		mapped = (void*)((BYTE*)ptr + position);
	}

	return true;
}

void MemMap::Block::Unlock() {
	if (mapped) {
		filemap->UnlockPage(page);
		mapped = nullptr;
	}
}

void MemMap::Block::CopyTo(Block& dest) {
	dest.filemap = filemap;
	dest.page = page;
	dest.position = position;
	dest.size = size;

	// �ȉ��̓R�s�[���Ȃ�
	dest.ref_count = 0;
	dest.mapped = nullptr;
}

MemMap::FileMap::FileMap(DWORD size, DWORD allocation_glanularity)
	: ref_count_(0)
{
	DWORD filemap_size = CalcAlignment(size, allocation_glanularity);
	filemap_ = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, filemap_size, NULL);
	filemap_page_size_ = allocation_glanularity;
	filemap_page_count_ = filemap_size / allocation_glanularity;

	pages_.resize(filemap_page_count_);

	// �e�y�[�W�̋󂫃u���b�N���쐬
	for (DWORD i = 0; i < filemap_page_count_; i++) {
		auto p = std::unique_ptr<Block>(new Block());
		p->filemap.Reset(this);
		p->page = i;
		p->position = 0;
		p->size = allocation_glanularity;
		p->ref_count = 0;
		p->mapped = nullptr;
		block_list_.InsertTail(p.release());
	}
}

MemMap::FileMap::~FileMap() {
	while (auto block = block_list_.Head()) {
		delete block;
	}

	CloseHandle(filemap_);
}

RefPtr<MemMap::Block> MemMap::FileMap::AllocateNewBlock(size_t size) {
	// �A���C�������g����
	size = CalcAlignment(size);

	Block* free_block_first = nullptr;
	Block* free_block_last = nullptr;
	DWORD free_block_chain_size = 0;

	// �󂫃u���b�N��T��
	block_list_.ForEach(
		[&](LinkList::Node* node) {
			Block* block = reinterpret_cast<Block*>(node);
			if (block->ref_count > 0) {
				// �g�p���u���b�N
				free_block_first = nullptr;
				free_block_chain_size = 0;
				return false;
			}

			if (!free_block_first || free_block_first->page != block->page) {
				// �����u���b�N�܂����Ȃ�
				free_block_first = block;
				free_block_last = block;
				free_block_chain_size = block->size;
			}
			else {
				free_block_last = block;
				free_block_chain_size += block->size;
			}

			if (free_block_chain_size < size) return false;

			return true;
		}
	);

	if (!free_block_first) return {};
	if (free_block_chain_size < size) return {};

	// �󂫃u���b�N��A��
	while (free_block_first != free_block_last) {
		auto p = reinterpret_cast<Block*>(free_block_last->prev);
		p->size += free_block_last->size;
		delete free_block_last;
		free_block_last = p;
	}

	if (free_block_first->size == size) {
		// �󂫃u���b�N�����̂܂ܕԂ�
		return RefPtr<Block>(free_block_first);
	}
	else {
		// �󂫃u���b�N�𕪊�
		auto allocated_block = std::unique_ptr<Block>(new Block());
		free_block_first->CopyTo(*allocated_block); // �R�s�[
		allocated_block->size = size;
		free_block_first->size -= size;
		free_block_first->position += size;

		auto ret = RefPtr<Block>(allocated_block.release());
		block_list_.InsertBefore(free_block_first, ret.GetPtr());
		return ret;
	}
}

bool MemMap::FileMap::AddRef() {
	return ++ref_count_;
}

void MemMap::FileMap::Release() {
	--ref_count_;
}

void* MemMap::FileMap::LockPage(DWORD page_index) {
	auto& page = pages_[page_index];
	if (!page.mapped) {
		LARGE_INTEGER offset;
		offset.QuadPart = (LONGLONG)filemap_page_size_ * page_index;
		page.mapped = MapViewOfFile(filemap_, FILE_MAP_ALL_ACCESS, offset.HighPart, offset.LowPart, filemap_page_size_);
	}
	page.lock_count++;
	return page.mapped;
}

void MemMap::FileMap::UnlockPage(DWORD page_index) {
	auto& page = pages_[page_index];
	if (--page.lock_count == 0) {
		UnmapViewOfFile(page.mapped);
		page.mapped = nullptr;
	}

}

// �v���C�x�[�g�R���X�g���N�^�F�������}�b�v���ƑΉ�����G���g���ւ̎Q�Ƃ��i�[����
//Handle(const RefPtr<Block>& block) : block_(block) { }

MemMap::Handle::Handle(RefPtr<Block>&& block) : block_(std::move(block)) { }

// �p�u���b�N�R���X�g���N�^�F�f�t�H���g�R���X�g���N�^

MemMap::Handle::Handle() { }

DWORD MemMap::Handle::GetSize() const { return block_->size; }

MemMap::MemMap(size_t initial_size)
	: map_size_(initial_size)
{
	GetSystemInfo(&sysinfo_);
	//map_ = std::unique_ptr<FileMap>(new FileMap(map_size_, sysinfo_.dwAllocationGranularity));
}

MemMap::~MemMap() { /* nop */ }

// �w��T�C�Y�̗̈���쐬����

MemMap::Handle MemMap::Allocate(size_t size) {
	if (size > sysinfo_.dwAllocationGranularity) return Handle{};

	FileMap* map = map_.get();
	while (map) {
		if (auto block = map->AllocateNewBlock(size)) {
			return Handle{ std::move(block) };
		}

		map = map->next_.get();
	}

	// ���̃t�@�C���}�b�v���X�g�ɋ󂫂�����
	// �V�����t�@�C���}�b�v���쐬
	auto new_filemap = std::unique_ptr<FileMap>(new FileMap(map_size_, sysinfo_.dwAllocationGranularity));
	new_filemap->next_ = std::move(map_);
	map_.swap(new_filemap);

	if (auto block = map_->AllocateNewBlock(size)) {
		return Handle{ std::move(block) };
	}

	return Handle{};
}
