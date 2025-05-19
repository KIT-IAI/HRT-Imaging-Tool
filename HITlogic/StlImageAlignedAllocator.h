/*******************************************************************************
SPDX-License-Identifier: GPL-2.0-or-later
Copyright 2010-2025 Karlsruhe Institute of Technology (KIT)
Contact: stephan.allgeier∂kit.edu,
         Institute of Automation and Applied Informatics

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, 51 Franklin Street,
Fifth Floor, Boston, MA 02110-1301, USA.
*******************************************************************************/



#pragma once
//#include "StlImage.h"
//#include <limits>
/*
This is a solution to some of our weird slowdown problems. The ones that are due to the low fragmentation heap (LFH)
in windows can be fixed with something like this. Just make sure to use an private heap and NEVER allocate something
smaller or equal to 16kb so the LFH "optimization" doesn't get activated. For certain allocation patterns
it makes things a lot slower. This just wraps this process in a nice allocator for use with STL-Containers.



QUOTE from https://developercommunity.visualstudio.com/content/problem/552439/mallocfree-dramatic-performance-slowdown.html

The Universal C Runtime malloc() call uses the Windows OS Heap APIs (ex: HeapAlloc) directly.

I've spoken to the Windows OS Heap team about this issue. They are aware of this issue, but this is a property of the Low Fragmentation Heap,
which Windows will use by default (see: https://docs.microsoft.com/en-us/windows/desktop/memory/low-fragmentation-heap). In optimizing for
allocations under 16k, additional overhead is incurred for large allocations. Note that disabling the Low Fragmentation Heap is not recommended,
as it will cause allocations to be much slower in normal situations.

The Windows OS Heap APIs use an alternative implementation (Segment Heap) for UWP applications. This implementation does not exhibit this
slowdown - if you are able to build and deploy as a UWP application instead of a Desktop application, this will solve the issue. Unfortunately,
the Segment Heap cannot be enabled for Desktop applications at this time.

If UWP is not an option, I recommend creating a private heap that you can use for only your large (>16k) allocations. If allocations under 16k
are avoided on this private heap, the Low Fragmentation Heap optimizations for small allocations won't kick in, and you will not experience the
slowdown you are reporting. To do this, you will need to call HeapCreate() to create a new heap, and replace malloc() calls that request over 16k
with a call to HeapAlloc() specifying your private heap, and replace the corresponding free() with HeapFree(), also specifying your private heap.

QUOTE END
*/
template <class T, size_t Alignment>
struct StlImageAlignedAllocator
{
	typedef T value_type;

	StlImageAlignedAllocator() = default;
	template <class U> constexpr StlImageAlignedAllocator(const StlImageAlignedAllocator <U, Alignment>&) noexcept {}

	// member classes/structs/unions
	template<typename U>
	struct rebind {
		// types
		typedef StlImageAlignedAllocator< U, Alignment > other;
	};

	[[nodiscard]] T* allocate(std::size_t n)
	{
		if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
		{
			throw std::bad_alloc();
		}
		int index = /*(GetCurrentThreadId()>>2)*/GetCurrentProcessorNumber() % 24;
		std::scoped_lock<std::mutex> lock(StlImage<T>::OurHackyMemoryStore[index].m);
		//NEVER EVER allocate <= 16KB, see above
		if (auto p = static_cast<T*>(HeapAlloc(StlImage<T>::OurHackyMemoryStore[index].heap, /*HEAP_ZERO_MEMORY*/0, std::max(32ull * 1024ull, n * sizeof(T) + 2 * sizeof(void*) + Alignment - 1))))
		{
			//Align the pointer to a certain alignment for later optimized AVX/AVX2 vector instructions. Save the original Pointer below the returned one for freeing later.
			void* ptr = (void*)(((uintptr_t)((char*)p + sizeof(void*) * 2 + Alignment - 1)) & ~(Alignment - 1ull));
			((void**)ptr)[-2] = (void*)index;
			((void**)ptr)[-1] = p;
			return (T*)ptr;
		}

		throw std::bad_alloc();
	}

	void deallocate(T* p, std::size_t n) noexcept
	{
		//get the base pointer to this memory chunk back for freeing.
		void* p1 = ((void**)p)[-1];
		int index = (int)(((void**)p)[-2]);
		std::scoped_lock<std::mutex> lock(StlImage<T>::OurHackyMemoryStore[index].m);
		HeapFree(StlImage<T>::OurHackyMemoryStore[index].heap, 0, p1);
	}

};

template <class T, size_t Alignment1, class U, size_t Alignment2>
bool operator==(const StlImageAlignedAllocator <T, Alignment1>&, const StlImageAlignedAllocator <U, Alignment2>&) { return std::is_same<T, U>::value; }
template <class T, size_t Alignment1, class U, size_t Alignment2>
bool operator!=(const StlImageAlignedAllocator <T, Alignment1>&, const StlImageAlignedAllocator <U, Alignment2>&) { return !std::is_same<T, U>::value; }

