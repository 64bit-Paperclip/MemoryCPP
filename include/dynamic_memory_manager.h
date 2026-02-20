// ============================================================================
// MemoryCPP - High Performance Arena Allocator & Memory Utility Library
// ----------------------------------------------------------------------------
// File:        dynamic_memory_manager.h
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

#ifndef __DYNAMIC_MEMORY_MANAGER_H_GUARD
#define __DYNAMIC_MEMORY_MANAGER_H_GUARD

#include <assert.h>
#include "memory_pool.h"

/// <summary>
/// A dynamic memory pool manager that owns a fixed number of pool slots, determined at compile time,
/// but allows pools to be created and destroyed at runtime as needed.
/// Unlike FIXED_MEMORY_MANAGER, pools are created and deleted independently, making
/// this suitable for systems where pool sizes or lifetimes are not known upfront.
/// Pool slots that have not been created are null and will assert in debug if accessed.
/// Not copyable. Not movable. Not thread-safe.
/// </summary>
template<size_t Count>
class DYNAMIC_MEMORY_MANAGER
{
    private:

        MEMORY_POOL* _Pools[Count];      // Fixed array of Pool objects. Lives inside the Manager's memory footprint.
        size_t _ActiveCount = 0;

    public:

        
        DYNAMIC_MEMORY_MANAGER()
        {
            for (size_t i = 0; i < Count; ++i) {
                _Pools[i] = nullptr;
            }
        }

        ~DYNAMIC_MEMORY_MANAGER()
        {
            for (size_t i = 0; i < Count; ++i) {
                if (_Pools[i] != nullptr) {
                    delete _Pools[i];
                }
            }
        }

        DYNAMIC_MEMORY_MANAGER(const DYNAMIC_MEMORY_MANAGER&) = delete;
        DYNAMIC_MEMORY_MANAGER& operator=(const DYNAMIC_MEMORY_MANAGER&) = delete;
        DYNAMIC_MEMORY_MANAGER(DYNAMIC_MEMORY_MANAGER&&) = delete;
        DYNAMIC_MEMORY_MANAGER& operator=(DYNAMIC_MEMORY_MANAGER&&) = delete;
        

        /// <summary>
        /// Returns the total number of pools this manager holds.
        /// This is a compile-time constant.
        /// </summary>
        [[nodiscard]] static constexpr size_t MaxPoolCount() noexcept
        {
            return Count;
        }

        /// <summary>
        /// Returns the number of pools currently allocated (non-null).
        /// This is a runtime calculation.
        /// </summary>
        [[nodiscard]] size_t ActivePoolCount() const noexcept { return _ActiveCount; }

        /// <summary>
        /// Allocates a new MEMORY_POOL on the heap at the specified index.
        /// Will not overwrite an existing pool; call DeletePool first if replacement is intended.
        /// In debug builds, this is treated as a hard error and will assert immediately, as calling
        /// CreatePool on an occupied slot or out of bounds index is a programming mistake that should
        /// never happen in correct code where indexs are known ahead of time.
        /// In release builds, asserts are stripped so the return value must be checked. This is why
        /// the function is marked nodiscard. 
        /// </summary>
        /// <param name="index">The index at which to create the pool.</param>
        /// <param name="poolSize">The size in bytes of the new pool.</param>
        /// <returns>
        /// True if the pool was successfully created;
        /// false if the index is out of bounds or a pool already exists at that index.
        /// </returns>
        [[nodiscard]] bool CreatePool(size_t index, size_t poolSize)
        {
            assert(index < Count && "CreatePool: Index out of bounds!");
            assert(_Pools[index] == nullptr && "CreatePool: Pool already exists at this index, call DeletePool first!");

            if (index >= Count || _Pools[index] != nullptr)
                return false;

            _Pools[index] = new MEMORY_POOL(poolSize);
            ++_ActiveCount;
            return true;
        }

        /// <summary>
        /// Explicitly deletes the pool at the specified index and nulls the pointer.
        /// Asserts in debug if the index is out of bounds.
        /// Calling this on an uninitialized slot is a safe no-op at runtime.
        /// </summary>
        void DeletePool(size_t index)
        {
            assert(index < Count && "Pool index out of bounds!");
            if (_Pools[index] != nullptr) {
                delete _Pools[index];
                _Pools[index] = nullptr;
                --_ActiveCount;
            }
        }

        /// <summary>
        /// Returns a reference to the pool. 
        /// Note: Caller must ensure the pool exists (is not nullptr).
        /// </summary>
        [[nodiscard]] inline MEMORY_POOL& GetPool(size_t index) noexcept
        {
            assert(index < Count && "Pool index out of bounds!");
            assert(_Pools[index] != nullptr && "Attempted to access a null pool!");
            return *_Pools[index];
        }

        /// <summary>
        /// Checks if a pool has been allocated at the given index.
        /// </summary>
        [[nodiscard]] inline bool PoolExists(size_t index) const noexcept
        {
            return (index < Count) && (_Pools[index] != nullptr);
        }



        /// <summary>
        /// Resets the pool at the specified index.
        /// Asserts if the pool has not been created yet.
        /// </summary>
        void ResetPool(size_t index) noexcept
        {
            assert(index < Count && "DynamicPool: Index out of bounds!");
            assert(_Pools[index] != nullptr && "DynamicPool: Cannot reset a null pool!");

            _Pools[index]->Reset();
        }

        /// <summary>
        /// Resets all currently active pools, making their memory available for reuse.
        /// Does not call destructors on any allocated objects.
        /// Silently skips null slots, unlike ResetPool which asserts on null access.
        /// </summary>
        void ResetAll() noexcept
        {
            for (size_t i = 0; i < Count; ++i)
            {
                if (_Pools[i] != nullptr)
                    _Pools[i]->Reset();
            }
        }

        /// <summary>
        /// Swaps the pools at the two specified indices.
        /// Not thread-safe; no other thread should be accessing either pool during this operation.
        /// Asserts in debug if either index is out of bounds.
        /// </summary>
        /// <param name="indexA">The index of the first pool.</param>
        /// <param name="indexB">The index of the second pool.</param>
        void SwapPools(size_t indexA, size_t indexB) noexcept
        {
            assert(indexA < Count && "SwapPools: indexA out of bounds!");
            assert(indexB < Count && "SwapPools: indexB out of bounds!");

            MEMORY_POOL* temp = _Pools[indexA];
            _Pools[indexA] = _Pools[indexB];
            _Pools[indexB] = temp;
        }
};


#endif