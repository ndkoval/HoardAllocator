#ifndef BASE_STACK_H
#define BASE_STACK_H

#include <assert.h>
#include "Superblock.h"

namespace hoard {

template<typename T>
class BaseStack {

public:
	BaseStack() : head_(nullptr), size_(0) {
	}

	BaseStack(const BaseStack &) = delete;
	BaseStack & operator=(const BaseStack &) = delete;

	void Push(T *t) {
		SetNext(t, head_);
		SetPrev(t, nullptr);
    if(head_) {
      SetPrev(head_, t);
    }
		head_ = t;
    ++size_;
	}

	T *Pop() {
		assert(!IsEmpty());
		T *t = head_;
		head_ = Next(head_);
		SetNext(t, nullptr);
    SetPrev(t, nullptr);
    --size_;
		return t;
	}

	T * Top() {
		return head_;
	}

	bool IsEmpty() {
		return head_ == nullptr;
	}

  void Reset() {
    head_ = nullptr;
    size_ = 0;
  }

  bool Contains(T * element) {
    auto * current = head_;
    while (current != nullptr) {
      if (element == current) {
        return true;
      }
      current = Next(current);
    }
    return false;
  }

  size_t Size() {
    return size_;
  }

  void CheckInvariantsOrDie() {
    size_t current_size = 0;
    auto * current = head_;
    while (current != nullptr) {
      ++current_size;
      current = Next(current);
    }
    assert(current_size == size_);
  }

protected:
	virtual void SetNext(T *t, T *value) = 0;

	virtual void SetPrev(T *t, T *value) = 0;

	virtual T *Next(T *t) = 0;

	T *head_;
  size_t size_;
};

}
#endif // BASE_STACK_H