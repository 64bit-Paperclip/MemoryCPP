#pragma once

#ifndef __MEMORY_BLOCK_H_GUARD
#define __MEMORY_BLOCK_H_GUARD

#include <cstddef>      // size_t
#include <malloc.h>     // malloc, free
#include <cassert>      // assert

/// <summary>
/// A lightweight RAII wrapper around a single heap-allocated memory block.
/// Owns the memory for its entire lifetime — allocates on construction and frees on destruction.
/// Intended to be used as the backing storage for higher-level allocators such as MEMORY_POOL.
/// Not copyable or movable; ownership is strict and non-transferable.
/// </summary>
class MEMORY_BLOCK
{
    private:
        void* _Head = nullptr;
        size_t _SizeInBytes = 0;
    public:

    /// <summary>
    /// Allocates a contiguous block of memory of the specified size on the heap.
    /// Asserts on failure, as a null block is considered an unrecoverable error.
    /// </summary>
    /// <param name="sizeInBytes">The number of bytes to allocate.</param>
    explicit MEMORY_BLOCK(size_t sizeInBytes) : _Head(malloc(sizeInBytes)), _SizeInBytes(sizeInBytes)
    {
        assert(_Head != nullptr && "MEMORY_BLOCK: malloc failed");

    }

    /// <summary>
    /// Releases the allocated memory block back to the heap.
    /// Only frees if the block is non-null, making it safe even if construction failed.
    /// </summary>
    ~MEMORY_BLOCK()
    {
        if (_Head)
            free(_Head);
    }

    MEMORY_BLOCK(const MEMORY_BLOCK&) = delete;
    MEMORY_BLOCK& operator=(const MEMORY_BLOCK&) = delete;
    MEMORY_BLOCK(MEMORY_BLOCK&&) = delete;
    MEMORY_BLOCK& operator=(MEMORY_BLOCK&&) = delete;

    /// <summary>
    /// Returns a pointer to the start of the allocated memory block.
    /// The caller is responsible for ensuring the pointer is not used after the block is destroyed.
    /// </summary>
    /// <returns>A void pointer to the head of the allocated block.</returns>
    [[nodiscard]] inline void* GetHead() const noexcept { return _Head;}

    /// <summary>
    /// Returns the size of the allocated memory block as requested at construction time.
    /// </summary>
    /// <returns>The size of the block in bytes.</returns>
    [[nodiscard]] inline size_t GetSize() const noexcept { return _SizeInBytes;}

    /// <summary>
    /// Indicates whether the underlying memory block is null.
    /// A null block typically means allocation failed or the block was never initialized.
    /// </summary>
    /// <returns>True if the block is null; otherwise false.</returns>
    [[nodiscard]] inline bool  IsNullPtr() const noexcept { return _Head == nullptr; }

    /// <summary>
    /// Evaluates the block in a boolean context.
    /// Allows natural usage such as if (block) to check validity without explicitly calling IsNullPtr().
    /// </summary>
    /// <returns>True if the block is non-null and valid; otherwise false.</returns>
    [[nodiscard]] explicit operator bool() const noexcept { return _Head != nullptr; }


};

#endif
