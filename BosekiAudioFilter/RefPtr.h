#pragma once

template <typename T>
class RefPtr {
	T* ptr_;
public:
	RefPtr() : ptr_(nullptr) {}

	RefPtr(T* p) : ptr_(p) {
		if (!ptr_->AddRef()) ptr_ = nullptr;
	}

	~RefPtr() {
		if (ptr_) {
			ptr_->Release();
		}
		ptr_ = nullptr;
	}

	template <typename U>
	RefPtr(const RefPtr<U>& p) : ptr_(nullptr) {
		if (p.ptr_->AddRef()) ptr_ = p.ptr_;
	}

	template <typename U>
	RefPtr(RefPtr<U>&& p) : ptr_(nullptr) {
		std::swap(ptr_, p.ptr_);
	}

	template <typename U>
	RefPtr<T>& operator =(const RefPtr<U>& p) {
		if (this != &p) {
			if (p.ptr_->AddRef()) ptr_ = p.ptr_;
		}
		return *this;
	}

	template <typename U>
	RefPtr<T>& operator =(RefPtr<U>&& p) {
		if (this != &p) {
			std::swap(ptr_, p.ptr_);
		}
		return *this;
	}



	T* operator ->() const noexcept {
		return ptr_;
	}

	T& operator *() const noexcept {
		return *ptr_;
	}

	operator bool() const noexcept {
		return !!ptr_;
	}

	T* GetPtr() const noexcept {
		return ptr_;
	}

	T* Reset(T* new_ptr) {
		auto prev_ptr = ptr_;
		if (prev_ptr) prev_ptr->Release();
		ptr_ = new_ptr;
		if (!ptr_->AddRef()) ptr_ = nullptr;
		return prev_ptr;
	}

	template <typename U>
	void Swap(RefPtr<U>&& p) {
		std::swap(ptr_, p.ptr_);
	}
};

