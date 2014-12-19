#include <cstring>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <algorithm>

#include "utils.h"
#include "Internals.h"
#include "FreeSuperblockManager.h"
#include "GlobalHeap.h"
#include "LocalHeap.h"
#include "superblock.h"

namespace {

using namespace hoard;


int _CheckPageSize() {
	hoard::trace("Page size is: ", hoard::kRealPageSize);

	assert(hoard::kPageSize == hoard::kRealPageSize && "change kPageSize and recompile lib for your machine");
	return hoard::kPageSize == hoard::kRealPageSize;
}


bool is_inited;

std::mutex init_mutex;
std::mutex big_alloc_mutex;

struct HoardState {

	hoard::AllocFreeHashMap big_allocates_map;

	hoard::FreeSuperblockManager globalFreeSuperblockManager;

	hoard::GlobalHeap testGlobalHeap;

	hoard::LocalHeap testLocalHeap;

} state;


void Init() {
	if (!is_inited) {
		std::lock_guard<std::mutex> guard(init_mutex);
		if(!is_inited){
			_CheckPageSize();
			new (&state) HoardState();
			is_inited = true;
		}
	}
}

}

namespace hoard {


void *InternalAlloc(size_t size, size_t alignment) {
	Init();
	assert (IsValidAlignment(alignment));
	void *result;
	if (size >= kMaxBlockSize) {
		result = BigAlloc(size, alignment);
	} else {
		result = SmallAlloc(size, alignment);
	}
	return result;
}


void *SmallAlloc(size_t size, size_t alignment) {
	//TODO small allocations
	return BigAlloc(size, alignment);
}

void InternalFree(void *ptr) {
	Init();
	if (ptr == nullptr)
		return;

	//big allocations always have page size alignment
	if (((size_t) ptr % kPageSize) || !BigFree(ptr)) {
		SmallFree(ptr);
	}
}

void SmallFree(void *ptr) {
//	SuperblockHeader *superblock = SuperblockHeader::Get(ptr);
//	assert(superblock->valid());


	//TODO small allocations
	BigFree(ptr);
}

void *BigAlloc(size_t size, size_t alignment) {
	assert(IsValidAlignment(alignment)); //TODO remove before release
	void *result_ptr = mmapAligned(size, alignment);
	if (!result_ptr) {
		return nullptr;
	}

	std::lock_guard<std::mutex> lock(big_alloc_mutex);
	state.big_allocates_map.Add(result_ptr, size);
	return result_ptr;
}


//returns true if ptr was big allocation
bool BigFree(void *ptr) {
	std::lock_guard<std::mutex> lock(big_alloc_mutex);
	size_t size = state.big_allocates_map.Get(ptr);
	if (size == AllocFreeHashMap::kNoSuchKey) {
		return false;
	}
	auto unmap_result = munmap(ptr, size);
	if (unmap_result == 0) {
		state.big_allocates_map.Remove(ptr);
		return true;
	} else {
		hoard::println("Big free unmap failed on adress: ", ptr);
		std::abort();
	}
}

void *InternalRealloc(void *ptr, size_t size) { // TODO optimize realloc before release
	if (ptr == nullptr) {
		return InternalAlloc(size);
	}
	if (size == 0) {
		InternalFree(ptr);
		return nullptr;
	}
	void *result = InternalAlloc(size);
	if (result == nullptr) {
		return nullptr;
	}
	size_t old_size;
	{
		std::lock_guard<std::mutex> lock(big_alloc_mutex);
		old_size = state.big_allocates_map.Get(ptr);
	}
	memcpy(result, ptr, std::min(size, old_size));
	InternalFree(ptr);
	return result;
}

}// hoard
