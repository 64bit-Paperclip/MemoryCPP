

#pragma once

#ifndef __FIXED_MEMORY_MANAGER_H_GUARD
#define __FIXED_MEMORY_MANAGER_H_GUARD

#include <cstddef>          // size_t
#include "memory_pool.h"


/// <summary>
/// Manages a fixed collection of memory pools whose count and sizes are all known at compile time.
/// All pools are allocated upfront and live inside the manager's own memory footprint — no heap allocation occurs.
/// Provides compile-time indexed access for zero-cost bounds checking and runtime access for flexibility.
/// Unlike DYNAMIC_MEMORY_MANAGER, pools cannot be created or destroyed independently; all slots are always alive.
/// Not copyable. Not movable. Not thread-safe.
/// </summary>
template<size_t Count>
class FIXED_MEMORY_MANAGER
{
    private:
        MEMORY_POOL _StaticPools[Count];      // Fixed array of Pool objects. Lives inside the Manager's memory footprint.

    public:
    
        /// <summary>
        /// Constructs the manager and initializes each pool with the provided sizes.
        /// The number of size arguments must exactly match the compile-time pool count.
        /// </summary>
        /// <typeparam name="Args">Variadic size arguments, one per pool.</typeparam>
        /// <param name="sizes">The size in bytes for each pool, in order.</param>
        template<typename... Args>
        FIXED_MEMORY_MANAGER(Args... sizes) : _StaticPools{ MEMORY_POOL(sizes)... }
        {
            static_assert(sizeof...(Args) == Count, "Number of sizes must match Pool Count!");      // This syntax initializes the _Pools array by expanding the "sizes" pack into the array initializer list.
        }

        ~FIXED_MEMORY_MANAGER() = default;
        FIXED_MEMORY_MANAGER(const FIXED_MEMORY_MANAGER&) = delete;
        FIXED_MEMORY_MANAGER& operator=(const FIXED_MEMORY_MANAGER&) = delete;
        FIXED_MEMORY_MANAGER(FIXED_MEMORY_MANAGER&&) = delete;
        FIXED_MEMORY_MANAGER& operator=(FIXED_MEMORY_MANAGER&&) = delete;

        /// <summary>
        /// Returns the total number of pools this manager holds.
        /// This is a compile-time constant.
        /// </summary>
        [[nodiscard]] static constexpr size_t MaxPoolCount() noexcept
        {
            return Count;
        }

        /// <summary>
        /// Returns the total number of Active pools this manager holds.
        /// In the case of this class, it is the same as MaxPoolCount.
        /// This is a compile-time constant.
        /// </summary>
        [[nodiscard]] static constexpr size_t ActivePoolCount() noexcept
        {
            return Count;
        }

        /// <summary>
        /// Returns a reference to the pool at the specified compile-time index.
        /// Index is bounds-checked at compile time.
        /// </summary>
        /// <typeparam name="Index">The index of the pool to retrieve.</typeparam>
        /// <returns>A reference to the pool at the given index.</returns>

        template<size_t Index>
        [[nodiscard]] inline MEMORY_POOL& GetPool() noexcept
        {
            static_assert(Index < Count, "Pool index out of bounds!");
            return _StaticPools[Index];
        }

        /// <summary>
        /// Returns a reference to the pool at the specified runtime index.
        /// Asserts in debug if the index is out of bounds.
        /// </summary>
        /// <param name="index">The index of the pool to retrieve.</param>
        /// <returns>A reference to the pool at the given index.</returns>
        [[nodiscard]] inline MEMORY_POOL& GetPool(size_t index) noexcept
        {
            assert(index < Count && "Pool index out of bounds!");
            return _StaticPools[index];
        }


        /// <summary>
        /// Resets the pool at the specified compile-time index.
        /// Does not call destructors on any allocated objects.
        /// </summary>
        /// <typeparam name="Index">The index of the pool to reset.</typeparam>
        template<size_t Index>
        void ResetPool() noexcept
        {
            static_assert(Index < Count, "Pool index out of bounds!");
            _StaticPools[Index].Reset();
        }

        /// <summary>
        /// Resets the pool at the specified runtime index.
        /// Does not call destructors on any allocated objects.
        /// No bounds checking is performed.
        /// </summary>
        /// <param name="index">The index of the pool to reset.</param>
        void ResetPool(size_t index) noexcept
        {
            assert(index < Count && "Pool index out of bounds!");
            _StaticPools[index].Reset();
        }

        /// <summary>
        /// Resets all managed pools, making their memory available for reuse.
        /// Does not call destructors on any allocated objects.
        /// </summary>
        // Bulk operations
        void ResetAll() noexcept
        {
            for (size_t i = 0; i < Count; ++i)
            {
                _StaticPools[i].Reset();
            }
        }

    
};

#endif
