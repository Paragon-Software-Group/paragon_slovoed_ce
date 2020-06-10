#pragma once

#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <vector>

#include "util.h"

namespace sld {

// a simple typeless "bump the pointer" allocator
class BumpPtrAllocator final
{
	struct Page {
		char *data;
		// current offset inside the page's data
		size_t current;
	};

public:
	BumpPtrAllocator(size_t pageSize) : pageSize(pageSize)
	{
		// always preallocate a page
		allocatePage();
	}

	~BumpPtrAllocator()
	{
		for (Page &page : pages)
			free(page.data);
	}

	// non copyable
	BumpPtrAllocator(const BumpPtrAllocator&) = delete;
	BumpPtrAllocator& operator=(const BumpPtrAllocator&) = delete;

	void* allocate(size_t size, size_t alignment = 4)
	{
		Page *page = &pages.back();

		// align the block to the required alignment
		// this works because calloc() is guaranted to return memory "suitably aligned
		// to any object size"
		size_t current = div_round_up(page->current, alignment) * alignment;
		if (current + size > pageSize)
		{
			allocatePage();
			page = &pages.back();
			current = 0;
		}

		void *data = page->data + current;
		page->current = current + size;

		// check the alignement just in case
		assert((uintptr_t)data % alignment == 0);

		return data;
	}

private:
	void allocatePage()
	{
		Page page;
		page.data = (char*)calloc(1, pageSize);
		page.current = 0;
		pages.push_back(page);
	}

	const size_t pageSize;
	std::vector<Page> pages;
};

template <typename T>
class ArenaAllocator
{
	struct Page {
		T *data;
		size_t count;
	};

public:
	ArenaAllocator(size_t arenaSize) : arenaSize(arenaSize)
	{
		// always preallocate a page
		allocatePage();
	}

	~ArenaAllocator()
	{
		for (Page &page : pages)
		{
			for (size_t i : xrange(page.count))
				page.data[i].~T();
			free(page.data);
		}
	}

	// non copyable
	ArenaAllocator(const ArenaAllocator&) = delete;
	ArenaAllocator& operator=(const ArenaAllocator&) = delete;

	template <typename... Args>
	T* create(Args&&... args)
	{
		Page *page = &pages.back();
		if (page->count == arenaSize)
		{
			allocatePage();
			page = &pages.back();
		}

		T* ptr = &page->data[page->count++];
		new (ptr) T(std::forward<Args>(args)...);
		return ptr;
	}

private:
	void allocatePage()
	{
		Page page;
		page.data = (T*)malloc(arenaSize * sizeof(T));
		page.count = 0;
		pages.push_back(page);
	}

	const size_t arenaSize;
	std::vector<Page> pages;
};

} // namespace sld
