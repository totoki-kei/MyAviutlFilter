#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <deque>
#include <algorithm>

#define NOMINMAX
#include <windows.h>

#include "SystemInfo.h"

template <typename KeyT, typename ValueT, typename KeyHashT, size_t Size = 1024>
class CacheControl
{
	std::unordered_map<KeyT, ValueT, KeyHashT> dictionary_;
	std::list<KeyT> history_;

	void InsertOrBringToFront(const KeyT& key) {
		if (auto it = std::find(history_.begin(), history_.end(), key); it != history_.end()) {
			history_.erase(it);
		}

		history_.push_front(key);
		while (history_.size() > Size) {
			dictionary_.erase(history_.back());
			history_.pop_back();
		}
	}

public:

	bool Get(const KeyT& key, ValueT& out_value) {
		InsertOrBringToFront(key);
		if (auto it = dictionary_.find(key); it != dictionary_.end()) {
			out_value = it->second;
			return true;
		}

		return false;
	}

	bool Set(const KeyT& key, const ValueT& value) {
		InsertOrBringToFront(key);
		if (auto it = dictionary_.find(key); it != dictionary_.end()) {
			it->second = value;
			return false;
		}

		dictionary_.insert({ key, value });
		return true;
	}

};


