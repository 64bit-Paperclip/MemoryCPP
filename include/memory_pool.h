// ============================================================================
// MemoryCPP - High Performance Arena Allocator & Memory Utility Library
// ----------------------------------------------------------------------------
// File:        memory_pool.h
// Author:      Jason Penick
// Website:     630Studios.com
// Created:     2026
//
// Copyright (c) 2026 Jason Penick. All rights reserved.
//
// This software is provided under the terms outlined in LICENSE.txt
// See README.md for full documentation and usage examples
// ============================================================================


#pragma once

#ifndef __MEMORY_POOL_H_GUARD
#define __MEMORY_POOL_H_GUARD

#include <cstddef>              // size_t
#include "memory_block.h"   
#include "memory_slice.h"


/// <summary>
/// A linear allocator (arena) that carves memory out of a single contiguous block.
/// Allocation is O(1), just a pointer increment. There is no per-object deallocation;
/// call Reset() to free everything at once, which makes this ideal for temporary or
/// per-frame allocations where the lifetime of all objects is known upfront.
/// All allocations are 8-byte aligned internally.
/// Not copyable. Not thread-safe.
/// </summary>
class MEMORY_POOL
{
    private:
        MEMORY_BLOCK _Block;            // Allocated Memory Block
        size_t _NextOffset = 0;         // Current allocation position
        size_t _MaxBytesUsed = 0;       // Maxiumum Allocation over lifetime


    public:

        MEMORY_POOL() = default;
        MEMORY_POOL(size_t sizeInBytes) : _Block(sizeInBytes) { }

        MEMORY_POOL(const MEMORY_POOL&) = delete;             // Prevent copies
        MEMORY_POOL& operator=(const MEMORY_POOL&) = delete;  // Prevent copies

        ~MEMORY_POOL() = default;

        [[nodiscard]] size_t Size() const noexcept { return _Block.GetSize(); }
        [[nodiscard]] size_t GetMaxBytesUsed() const noexcept { return _MaxBytesUsed; }
        [[nodiscard]] size_t BytesUsed() const noexcept { return _NextOffset; }

        /// <summary>
        /// Determines whether the given pointer belongs to this pool's memory block.
        /// </summary>
        /// <param name="ptr">The pointer to check.</param>
        /// <returns>
        /// True if the pointer falls within the currently allocated region of this pool;
        /// otherwise false.
        /// </returns>
        inline bool Owns(void* ptr) const noexcept
        {
            auto p = reinterpret_cast<uintptr_t>(ptr);
            auto head = reinterpret_cast<uintptr_t>(_Block.GetHead());
            return p >= head && p < head + _NextOffset;
        }

        /// <summary>
        /// Carves out a slice of memory of the specified size.
        /// </summary>
        /// <param name="sizeInBytes">The number of bytes requested.</param>
        /// <returns>
        /// A pointer to the start of the memory slice if successful; 
        /// otherwise returns nullptr if there is insufficient room remaining.
        /// </returns>
        inline MEMORY_SLICE TakeSlice(size_t sizeInBytes)
        {
            assert(sizeInBytes > 0 && "TakeSlice: cannot request 0 bytes");     

            const size_t alignedReq = (sizeInBytes + 7) & ~7;                   // Round up the request to 8-byte alignment to keep the next slice aligned

            if (_NextOffset + alignedReq > _Block.GetSize())                    // Verify we have enough room remaining in the block
            {
                return MEMORY_SLICE(nullptr, 0);
            }

            void* ptr = static_cast<char*>(_Block.GetHead()) + _NextOffset;     // Calculate the address at the current offset
            _NextOffset += alignedReq;                                          // Advance the offset for the next call

            return MEMORY_SLICE(ptr, sizeInBytes);
        }

        /// <summary>
        /// Carves out a slice of memory of the specified size at the specified alignment.
        /// Alignment must be a non-zero power of two.
        /// Padding bytes are inserted before the allocation as needed to satisfy the alignment
        /// requirement. The internal offset is always advanced by a multiple of 8 bytes to ensure
        /// subsequent TakeSlice calls remain correctly aligned regardless of the requested alignment
        /// or how much padding was required.
        /// </summary>
        /// <param name="sizeInBytes">The number of bytes requested.</param>
        /// <param name="alignment">The required alignment in bytes. Must be a non-zero power of two.</param>
        /// <returns>
        /// A slice pointing to the aligned memory region if successful;
        /// otherwise a null slice if there is insufficient room remaining to satisfy
        /// both the alignment padding and the requested size.
        /// </returns>
        inline MEMORY_SLICE TakeAlignedSlice(size_t sizeInBytes, size_t alignment)
        {
            assert(sizeInBytes > 0 && "TakeAlignedSlice: cannot request 0 bytes");
            assert(alignment > 0 && (alignment & (alignment - 1)) == 0 && "TakeAlignedSlice: alignment must be a non-zero power of two");

            uintptr_t raw = reinterpret_cast<uintptr_t>(_Block.GetHead()) + _NextOffset;
            uintptr_t aligned = (raw + (alignment - 1)) & ~(alignment - 1);
            size_t    padding = aligned - raw;                                          // Bytes skipped to reach requested alignment

            size_t totalAdvance = padding + sizeInBytes;                                // Total bytes consumed
            totalAdvance = (totalAdvance + 7) & ~7;                              // Round total advance up to 8-byte alignment

            if (_NextOffset + totalAdvance > _Block.GetSize())
                return MEMORY_SLICE(nullptr, 0);

            _NextOffset += totalAdvance;

            return MEMORY_SLICE(reinterpret_cast<void*>(aligned), sizeInBytes);
        }

        /// <summary>
        /// Allocates a single object of type T and constructs it in place with the provided arguments.
        /// </summary>
        /// <typeparam name="T">The type to allocate and construct.</typeparam>
        /// <param name="args">Constructor arguments forwarded to T.</param>
        /// <returns>
        /// A pointer to the constructed object if successful;
        /// otherwise returns nullptr if there is insufficient room remaining.
        /// </returns>
        template<typename T, typename... Args>
        inline T* Take(Args&&... args)
        {
            MEMORY_SLICE slice = TakeSlice(sizeof(T));      // Get A Slice

            if (slice.IsNullPtr())                          // Validate the slice
                return nullptr;

            T* ptr = static_cast<T*>(slice.GetHead());      // Get The Typed Pointer
            new (ptr) T(args...);                           // Call constructor with any number of arguments

            return ptr;
        }

        /// <summary>
        /// Allocates a contiguous array of count objects of type T and default-constructs each one.
        /// </summary>
        /// <typeparam name="T">The type to allocate and construct.</typeparam>
        /// <param name="count">The number of elements to allocate.</param>
        /// <returns>
        /// A pointer to the first element if successful;
        /// otherwise returns nullptr if count is zero or there is insufficient room remaining.
        /// </returns>
        template<typename T>
        inline T* TakeArray(size_t count)
        {
            if (count == 0) return nullptr;

            MEMORY_SLICE slice = TakeSlice(sizeof(T) * count);

            if (slice.IsNullPtr())
                return nullptr;

            T* ptr = static_cast<T*>(slice.GetHead());
        
            for (size_t i = 0; i < count; ++i)  // Default construct each element in the array
            {
                new (&ptr[i]) T();
            }

            return ptr;
        }


        /// <summary>
        /// Resets the pool, making all previously allocated memory available for reuse.
        /// Does not call destructors on any allocated objects.
        /// </summary>
        inline void Reset() noexcept
        {
            if (_NextOffset > _MaxBytesUsed)
                _MaxBytesUsed = _NextOffset;

            _NextOffset = 0;
        }


};


#endif