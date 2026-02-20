// ============================================================================
// MemoryCPP - High Performance Arena Allocator & Memory Utility Library
// ----------------------------------------------------------------------------
// File:        memory_manager.h
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

#ifndef __MEMORY_MANAGER_H_GUARD
#define __MEMORY_MANAGER_H_GUARD


#include "fixed_memory_manager.h"
#include "dynamic_memory_manager.h"
#include "memory_units.h"

template<size_t FixedCount, size_t DynamicCount>
class MEMORY_MANAGER
{
	
	private:
		FIXED_MEMORY_MANAGER<FixedCount> _FixedManager;
		DYNAMIC_MEMORY_MANAGER<DynamicCount> _DynamicManager;


	public:

        /// <summary>
        /// The constructor takes the sizes for the Fixed pools.
        /// The Dynamic pools remain null until CreatePool is called.
        /// </summary>
        template<typename... Args>
        MEMORY_MANAGER(Args... sizes) : _FixedManager(sizes...) // Perfect forwarding of the pack to the member constructor
        {
            // Use FixedCount here, as that is the template parameter name
            static_assert(sizeof...(Args) == FixedCount, "Number of size parameters must match Fixed Pool Count!");
        }

        ~MEMORY_MANAGER() = default;

        MEMORY_MANAGER(const MEMORY_MANAGER&) = delete;
        MEMORY_MANAGER& operator=(const MEMORY_MANAGER&) = delete;
        MEMORY_MANAGER(MEMORY_MANAGER&&) = delete;
        MEMORY_MANAGER& operator=(MEMORY_MANAGER&&) = delete;

        

        // ----------------------------------------------------------------
        //  Fixed Pool Access
        // ----------------------------------------------------------------

        /// <summary>
        /// Returns a reference to the fixed pool at the specified compile-time index.
        /// Bounds-checked at compile time.
        /// </summary>
        template<size_t Index>
        [[nodiscard]] inline MEMORY_POOL& GetFixedPool() noexcept
        {
            return _FixedManager.GetPool<Index>();
        }

        /// <summary>
        /// Returns a reference to the fixed pool at the specified runtime index.
        /// Asserts in debug if the index is out of bounds.
        /// </summary>
        [[nodiscard]] inline MEMORY_POOL& GetFixedPool(size_t index) noexcept
        {
            return _FixedManager.GetPool(index);
        }

        /// <summary>
        /// Resets the fixed pool at the specified compile-time index.
        /// Does not call destructors on any allocated objects.
        /// </summary>
        template<size_t Index>
        inline void ResetFixedPool() noexcept
        {
            _FixedManager.ResetPool<Index>();
        }

        /// <summary>
        /// Resets the fixed pool at the specified runtime index.
        /// Asserts in debug if the index is out of bounds.
        /// Does not call destructors on any allocated objects.
        /// </summary>
        inline void ResetFixedPool(size_t index) noexcept
        {
            _FixedManager.ResetPool(index);
        }

        /// <summary>
        /// Resets all fixed pools.
        /// Does not call destructors on any allocated objects.
        /// </summary>
        inline void ResetAllFixed() noexcept
        {
            _FixedManager.ResetAll();
        }

        // ----------------------------------------------------------------
                //  Dynamic Pool Access
                // ----------------------------------------------------------------

                /// <summary>
                /// Allocates a new dynamic pool at the specified index.
                /// Asserts in debug if the slot is occupied or out of bounds.
                /// Returns false in release if the slot is occupied or out of bounds.
                /// </summary>
        [[nodiscard]] inline bool CreateDynamicPool(size_t index, size_t poolSize) noexcept
        {
            return _DynamicManager.CreatePool(index, poolSize);
        }

        /// <summary>
        /// Destroys the dynamic pool at the specified index.
        /// Safe no-op if the slot is null.
        /// </summary>
        inline void DeleteDynamicPool(size_t index) noexcept
        {
            _DynamicManager.DeletePool(index);
        }

        /// <summary>
        /// Returns a reference to the dynamic pool at the specified index.
        /// Asserts in debug if the slot is null or out of bounds.
        /// </summary>
        [[nodiscard]] inline MEMORY_POOL& GetDynamicPool(size_t index) noexcept
        {
            return _DynamicManager.GetPool(index);
        }

        /// <summary>
        /// Checks whether a dynamic pool exists at the specified index.
        /// </summary>
        [[nodiscard]] inline bool DynamicPoolExists(size_t index) const noexcept
        {
            return _DynamicManager.PoolExists(index);
        }

        /// <summary>
        /// Swaps the dynamic pools at the two specified indices.
        /// Not thread-safe. Safe even if one or both slots are null.
        /// </summary>
        inline void SwapDynamicPools(size_t indexA, size_t indexB) noexcept
        {
            _DynamicManager.SwapPools(indexA, indexB);
        }

        /// <summary>
        /// Resets the dynamic pool at the specified index.
        /// Asserts in debug if the pool does not exist.
        /// Does not call destructors on any allocated objects.
        /// </summary>
        inline void ResetDynamicPool(size_t index) noexcept
        {
            _DynamicManager.ResetPool(index);
        }

        /// <summary>
        /// Resets all active dynamic pools.
        /// Silently skips null slots.
        /// Does not call destructors on any allocated objects.
        /// </summary>
        inline void ResetAllDynamic() noexcept
        {
            _DynamicManager.ResetAll();
        }

        // ----------------------------------------------------------------
        //  Bulk Operations
        // ----------------------------------------------------------------

        /// <summary>
        /// Resets all fixed and dynamic pools.
        /// Does not call destructors on any allocated objects.
        /// </summary>
        inline void ResetAll() noexcept
        {
            _FixedManager.ResetAll();
            _DynamicManager.ResetAll();
        }

        /// <summary>
        /// Returns the number of fixed pool slots.
        /// </summary>
        [[nodiscard]] static constexpr size_t GetFixedCapacity() noexcept { return FixedCount; }

        /// <summary>
        /// Returns the number of dynamic pool slots.
        /// </summary>
        [[nodiscard]] static constexpr size_t GetDynamicCapacity() noexcept { return DynamicCount; }

        /// <summary>
        /// Returns the number of currently active dynamic pools.
        /// </summary>
        [[nodiscard]] inline size_t GetActiveDynamicCount() const noexcept
        {
            return _DynamicManager.GetActiveCount();
        }
};


#endif