#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <deque>
#include <algorithm>

#define NOMINMAX
#include <windows.h>

#include "SystemInfo.h"

template <typename TKey, typename TValue, size_t Size = sizeof(TValue) * 1024>
class CacheControl
{
	struct Entry {
		DWORD position;
		DWORD last_used;
	};
public:
	CacheControl()
		: filemap_(NULL)
		, current_map_start_(0)
		, current_map_blocks_(0)
		, current_map_base_(nullptr)
		, last_used_counter_(0)
		, data_ptr_(nullptr)
	{
		auto block_size = g_sysinfo.GetAllocationGranularity();
		auto filemap_size = block_size * ((Size + block_size - 1) / block_size);
		filemap_ = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, filemap_size, NULL);
	}

	~CacheControl() {
		if (current_map_base_) {
			UnmapViewOfFile(current_map_base_);
		}
		CloseHandle(filemap_);
	}

	HRESULT SetData(const TKey& key, const TValue& value) {
		HRESULT hr = MapToData(key, true);
		if (SUCCEEDED(hr)) {
			*data_ptr_ = value;
		}

		return hr;
	}

	HRESULT SetData(const TKey& key, TValue&& value) {
		HRESULT hr = MapToData(key, true);
		if (SUCCEEDED(hr)) {
			*data_ptr_ = std::move(value);
		}

		return hr;
	}

	HRESULT GetData(const TKey& key, TValue& out_value) {
		HRESULT hr = MapToData(key, false);
		if (hr == S_OK) {
			out_value = *data_ptr_;
		}
		return hr;
	}

private:
	HRESULT MapToData(const TKey& key, bool create_new) {
		auto it = offset_map_.find(key);
		if (it == offset_map_.end()) {
			// 存在しない
			if (create_new) {
				Entry ent = { AllocateNew(), 0 };
				auto insert_result = offset_map_.insert(std::make_pair(key, ent));
				it = insert_result.first;
			}
			else {
				return S_FALSE;
			}
		}

		const size_t block_size = g_sysinfo.GetAllocationGranularity();

		auto pos = it->second.position;
		it->second.last_used = last_used_counter_++;

		size_t pos_start = (pos / block_size) * block_size;
		size_t pos_end = ((pos + sizeof(TValue)) / block_size) * block_size;

		DWORD block_count = (DWORD)((sizeof(TValue) + block_size - 1) / block_size);
		if (pos_start != pos_end) block_count++;

		LARGE_INTEGER pos_li = {};
		pos_li.QuadPart = pos_start;

		if (current_map_base_ && pos_start == current_map_start_ && block_count == current_map_blocks_) {
			// 既存のマッピング領域中にデータが存在する：そのままdata_ptr_だけ差し替える
		}
		else {
			// 指定データを含む領域をマップ
			if (current_map_base_ && !UnmapViewOfFile(current_map_base_)) {
				return HRESULT_FROM_WIN32(::GetLastError());
			}
			current_map_base_ = MapViewOfFile(filemap_, FILE_MAP_WRITE, pos_li.HighPart, pos_li.LowPart, block_count * block_size);
			if (current_map_base_ == nullptr) {
				return HRESULT_FROM_WIN32(::GetLastError());
			}
			current_map_start_ = pos_start;
			current_map_blocks_ = block_count;
		}

		data_ptr_ = reinterpret_cast<TValue*>(reinterpret_cast<uintptr_t>(current_map_base_) + (pos - pos_start));
		return S_OK;
	}

	size_t AllocateNew() {
		size_t new_pos = offset_map_.size() * sizeof(TValue);
		if (new_pos + sizeof(TValue) > Size) {
			auto it = std::min_element(offset_map_.begin(), offset_map_.end(),
				[](const std::pair<const TKey, Entry>& a, const std::pair<const TKey, Entry>& b) {
				return a.second.last_used < b.second.last_used;
			}
			);
			new_pos = it->second.position;
			offset_map_.erase(it);
		}
		return new_pos;
	}


	HANDLE filemap_;
	size_t current_map_start_;
	DWORD current_map_blocks_;
	void* current_map_base_;
	DWORD last_used_counter_;

	TValue* data_ptr_;

	// キー => アドレス
	std::unordered_map<TKey, Entry> offset_map_;


};


