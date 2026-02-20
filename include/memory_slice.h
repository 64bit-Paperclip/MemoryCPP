// ============================================================================
// MemoryCPP - High Performance Arena Allocator & Memory Utility Library
// ----------------------------------------------------------------------------
// File:        memory_slice.h
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


#ifndef __MEMORY_SLICE_H_GUARD
#define __MEMORY_SLICE_H_GUARD

#include <cstddef>      // size_t
#include <cstring>      // memset, memcpy


/// <summary>
/// Represents a non-owning view into a region of memory carved out of a MEMORY_POOL.
/// Does not allocate or free memory; lifetime is managed by the pool that produced it.
/// Copyable and movable since it carries no ownership semantics.
/// </summary>

class MEMORY_SLICE
{
    private:
        void* _Head = nullptr;
        size_t _SizeInBytes = 0;

    public:
        /// <summary>
        /// Constructs a slice from an existing pointer and size.
        /// The caller is responsible for ensuring the pointer remains valid for the lifetime of the slice.
        /// </summary>
        /// <param name="head">A pointer to the start of the memory region.</param>
        /// <param name="sizeInBytes">The size of the memory region in bytes.</param>
        explicit MEMORY_SLICE(void* head, size_t sizeInBytes) : _Head(head), _SizeInBytes(sizeInBytes)
        {
            assert(sizeInBytes > 0 && "MEMORY_SLICE: size cannot be zero!");
        }

        ~MEMORY_SLICE() = default;
        MEMORY_SLICE(const MEMORY_SLICE&) = default;
        MEMORY_SLICE& operator=(const MEMORY_SLICE&) = default;
        MEMORY_SLICE(MEMORY_SLICE&&) = default;
        MEMORY_SLICE& operator=(MEMORY_SLICE&&) = default;

        /// <summary>
        /// Returns a pointer to the start of the memory region.
        /// </summary>
        /// <returns>A void pointer to the head of the slice.</returns>
        [[nodiscard]] inline void* GetHead() const noexcept { return _Head; }

        /// <summary>
        /// Returns the size of the memory region in bytes.
        /// </summary>
        /// <returns>The size of the slice in bytes.</returns>
        [[nodiscard]] inline size_t GetSize() const noexcept { return _SizeInBytes; }

        /// <summary>
        /// Indicates whether the slice points to a null address.
        /// A null slice typically indicates a failed allocation from the pool.
        /// </summary>
        /// <returns>True if the head pointer is null; otherwise false.</returns>
        [[nodiscard]] inline bool  IsNullPtr() const noexcept { return _Head == nullptr; }

        /// <summary>
        /// Evaluates the slice in a boolean context.
        /// Allows natural usage such as if (slice) to check validity without explicitly calling IsNullPtr().
        /// </summary>
        /// <returns>True if the slice is non-null and valid; otherwise false.</returns>
        [[nodiscard]] explicit operator bool() const noexcept { return _Head != nullptr; }

        /// <summary>
        /// Compares the contents of this slice with another slice byte-for-byte.
        /// Slices of different sizes are never equal.
        /// Asserts in debug if either slice is null.
        /// </summary>
        /// <param name="other">The slice to compare against.</param>
        /// <returns>True if both slices are the same size and contain identical bytes; otherwise false.</returns>
        [[nodiscard]] bool Equals(const MEMORY_SLICE& other) const noexcept
        {
            assert(_Head != nullptr && "Equals: cannot compare a null slice!");
            assert(other._Head != nullptr && "Equals: cannot compare against a null slice!");

            if (_SizeInBytes != other._SizeInBytes) return false;

            return memcmp(_Head, other._Head, _SizeInBytes) == 0;
        }

        /// <summary>
        /// Reinterprets the head of the slice as a pointer to the specified type.
        /// The caller is responsible for ensuring the slice is large enough to hold T
        /// and that the pointer is correctly aligned for T.
        /// </summary>
        /// <typeparam name="T">The type to reinterpret the slice as.</typeparam>
        /// <returns>A typed pointer to the head of the slice, or nullptr if the slice is null.</returns>
        template <typename T>
        [[nodiscard]] T* As() const noexcept
        {
            assert(_Head != nullptr && "As: cannot cast a null slice!");
            assert(_SizeInBytes >= sizeof(T) && "As: slice is too small to hold type T!");
            return static_cast<T*>(_Head);
        }

        /// <summary>
        /// Determines whether the given pointer falls within this slice's memory region.
        /// </summary>
        /// <param name="ptr">The pointer to check.</param>
        /// <returns>True if the pointer is within the slice's bounds; otherwise false.</returns>
        [[nodiscard]] bool Contains(void* ptr) const noexcept
        {
            uintptr_t p = reinterpret_cast<uintptr_t>(ptr);
            uintptr_t start = reinterpret_cast<uintptr_t>(_Head);
            uintptr_t end = start + _SizeInBytes;
            return (p >= start && p < end);
        }

        /// <summary>
        /// Fills the entire slice with the specified byte value.
        /// Asserts in debug if the slice is null.
        /// </summary>
        /// <param name="value">The byte value to fill with.</param>
        void Fill(unsigned char value) noexcept
        {
            assert(_Head != nullptr && "Fill: cannot fill a null slice!");
            memset(_Head, value, _SizeInBytes);
        }

        /// <summary>
        /// Returns a reference to a T at the specified byte offset within the slice.
        /// Asserts in debug if the slice is null or the offset would exceed the slice bounds.
        /// The caller is responsible for ensuring the pointer is correctly aligned for T.
        /// The returned reference is only valid for the lifetime of the slice.
        /// </summary>
        /// <typeparam name="T">The type to reinterpret the memory as.</typeparam>
        /// <param name="byteOffset">The byte offset from the head. Defaults to 0.</param>
        /// <returns>A reference to T at the given offset.</returns>
        template <typename T>
        [[nodiscard]] T& Get(size_t byteOffset = 0) const noexcept
        {
            assert(_Head != nullptr && "Get: cannot dereference a null slice!");
            assert(byteOffset + sizeof(T) <= _SizeInBytes && "Get: offset would exceed slice bounds!");

            return *reinterpret_cast<T*>(static_cast<unsigned char*>(_Head) + byteOffset);
        }

        /// <summary>
        /// Checks whether the head of the slice meets the specified alignment requirement.
        /// Alignment must be a non-zero power of two.
        /// </summary>
        /// <param name="alignment">The alignment to check against in bytes.</param>
        /// <returns>True if the head pointer satisfies the alignment requirement; otherwise false.</returns>
        [[nodiscard]] bool IsAligned(size_t alignment) const noexcept
        {
            return (reinterpret_cast<uintptr_t>(_Head) & (alignment - 1)) == 0;
        }


        /// <summary>
        /// Returns a pointer offset by the specified number of bytes from the head of the slice.
        /// Returns nullptr if the offset is out of bounds.
        /// </summary>
        /// <param name="bytes">The number of bytes to offset from the head.</param>
        /// <returns>A pointer to the offset address, or nullptr if the offset exceeds the slice size.</returns>
        [[nodiscard]] void* Offset(size_t bytes) const noexcept
        {
            if (bytes >= _SizeInBytes)
                return nullptr;
            
            // Cast to unsigned char* to ensure 1-byte pointer arithmetic
            return static_cast<unsigned char*>(_Head) + bytes;
        }

        /// <summary>
        /// Copies a region of bytes from another slice into this slice at the specified offsets.
        /// Asserts in debug if either slice is null or the operation would exceed either slice's bounds.
        /// </summary>
        /// <param name="other">The slice to copy from.</param>
        /// <param name="srcOffset">The byte offset into the source slice to copy from.</param>
        /// <param name="dstOffset">The byte offset into this slice to copy into.</param>
        /// <param name="size">The number of bytes to copy.</param>
        /// <returns>True if the copy succeeded; false if the operation would exceed either slice's bounds.</returns>
        [[nodiscard]] bool CopyFrom(const MEMORY_SLICE& other, size_t srcOffset, size_t dstOffset, size_t size) noexcept
        {
            assert(_Head != nullptr && "CopyFrom: cannot copy into a null slice!");
            assert(other._Head != nullptr && "CopyFrom: cannot copy from a null slice!");
            assert(srcOffset + size <= other._SizeInBytes && "CopyFrom: read would exceed source bounds!");
            assert(dstOffset + size <= _SizeInBytes && "CopyFrom: write would exceed destination bounds!");

            if (srcOffset + size > other._SizeInBytes)
                return false;

            if (dstOffset + size > _SizeInBytes)
                return false;

            std::memcpy( static_cast<unsigned char*>(_Head) + dstOffset, static_cast<const unsigned char*>(other._Head) + srcOffset, size );

            return true;
        }

        /// <summary>
        /// Carves out a sub-region of this slice starting at the given offset.
        /// Returns a null slice if the offset or size would exceed the bounds of this slice.
        /// </summary>
        /// <param name="offset">The byte offset from the head to start the subslice.</param>
        /// <param name="size">The size of the subslice in bytes.</param>
        /// <returns>A new slice representing the requested region, or a null slice if out of bounds.</returns>
        [[nodiscard]] MEMORY_SLICE Subslice(size_t offset, size_t size) const noexcept
        {
            void* newHead = Offset(offset);
            if (!newHead || (offset + size > _SizeInBytes)) {
                return MEMORY_SLICE(nullptr, 0); // Return null slice on overflow
            }
            return MEMORY_SLICE(newHead, size);
        }


        /// <summary>
        /// Copies a value of type T from the slice at the specified byte offset into the provided reference.
        /// Asserts in debug if the slice is null or the read would exceed the slice bounds.
        /// </summary>
        /// <typeparam name="T">The type to read.</typeparam>
        /// <param name="out">The reference to write the result into.</param>
        /// <param name="byteOffset">The byte offset from the head to read from. Defaults to 0.</param>
        template <typename T>
        void Read(T& out, size_t byteOffset = 0) const noexcept
        {
            assert(_Head != nullptr && "Read: cannot read from a null slice!");
            assert(byteOffset + sizeof(T) <= _SizeInBytes && "Read: read would exceed slice bounds!");

            std::memcpy(&out, static_cast<const unsigned char*>(_Head) + byteOffset, sizeof(T));
        }

        /// <summary>
        /// Writes a value of type T into the slice at the specified byte offset.
        /// Asserts in debug if the write would exceed the slice bounds.
        /// Returns false at runtime if the write would exceed the slice bounds.
        /// </summary>
        /// <typeparam name="T">The type to write.</typeparam>
        /// <param name="value">The value to write.</param>
        /// <param name="byteOffset">The byte offset from the head to write to. Defaults to 0.</param>
        /// <returns>True if the write succeeded; false if it would exceed the slice bounds.</returns>
        template <typename T>
        [[nodiscard]] bool Write(const T& value, size_t byteOffset = 0) noexcept
        {
            assert(_Head != nullptr && "Write: cannot write to a null slice!");
            assert(byteOffset + sizeof(T) <= _SizeInBytes && "Write: write would exceed slice bounds!");

            if (byteOffset + sizeof(T) > _SizeInBytes) return false;

            std::memcpy(static_cast<unsigned char*>(_Head) + byteOffset, &value, sizeof(T));
            return true;
        }

        /// <summary>
        /// Zeroes out the entire slice.
        /// Asserts in debug if the slice is null.
        /// </summary>
        void Zero() noexcept
        {
            assert(_Head != nullptr && "Zero: cannot zero a null slice!");
            std::memset(_Head, 0, _SizeInBytes);
        }



        //--------------------------------------------------------------------------------
        // Bit Operations
        //--------------------------------------------------------------------------------

        /// <summary>
        /// Returns the value of the bit at the specified bit index.
        /// Asserts in debug if the bit index is out of range.
        /// </summary>
        /// <param name="bitIndex">The zero-based index of the bit to read.</param>
        /// <returns>True if the bit is set; false if it is clear.</returns>
        [[nodiscard]] bool GetBit(size_t bitIndex) const noexcept
        {
            assert(_Head != nullptr && "GetBit: cannot read from a null slice!");
            assert(bitIndex < _SizeInBytes * 8 && "GetBit: bit index out of range!");

            size_t byteIndex = bitIndex / 8;
            size_t bitOffset = bitIndex % 8;
            return (static_cast<unsigned char*>(_Head)[byteIndex] >> bitOffset) & 1;
        }

        /// <summary>
        /// Sets the bit at the specified bit index to 1.
        /// Asserts in debug if the bit index is out of range.
        /// </summary>
        /// <param name="bitIndex">The zero-based index of the bit to set.</param>
        void SetBit(size_t bitIndex) noexcept
        {
            assert(_Head != nullptr && "SetBit: cannot write to a null slice!");
            assert(bitIndex < _SizeInBytes * 8 && "SetBit: bit index out of range!");

            size_t byteIndex = bitIndex / 8;
            size_t bitOffset = bitIndex % 8;
            static_cast<unsigned char*>(_Head)[byteIndex] |= (1u << bitOffset);
        }

        /// <summary>
        /// Clears the bit at the specified bit index to 0.
        /// Asserts in debug if the bit index is out of range.
        /// </summary>
        /// <param name="bitIndex">The zero-based index of the bit to clear.</param>
        void ClearBit(size_t bitIndex) noexcept
        {
            assert(_Head != nullptr && "ClearBit: cannot write to a null slice!");
            assert(bitIndex < _SizeInBytes * 8 && "ClearBit: bit index out of range!");

            size_t byteIndex = bitIndex / 8;
            size_t bitOffset = bitIndex % 8;
            static_cast<unsigned char*>(_Head)[byteIndex] &= ~(1u << bitOffset);
        }

        /// <summary>
        /// Toggles the bit at the specified bit index.
        /// Asserts in debug if the bit index is out of range.
        /// </summary>
        /// <param name="bitIndex">The zero-based index of the bit to toggle.</param>
        void ToggleBit(size_t bitIndex) noexcept
        {
            assert(_Head != nullptr && "ToggleBit: cannot write to a null slice!");
            assert(bitIndex < _SizeInBytes * 8 && "ToggleBit: bit index out of range!");

            size_t byteIndex = bitIndex / 8;
            size_t bitOffset = bitIndex % 8;
            static_cast<unsigned char*>(_Head)[byteIndex] ^= (1u << bitOffset);
        }

        /// <summary>
        /// Returns whether the bit at the specified bit index is set.
        /// Equivalent to GetBit but communicates intent more clearly in boolean contexts.
        /// Asserts in debug if the bit index is out of range.
        /// </summary>
        /// <param name="bitIndex">The zero-based index of the bit to check.</param>
        /// <returns>True if the bit is set; false if it is clear.</returns>
        [[nodiscard]] bool IsBitSet(size_t bitIndex) const noexcept
        {
            return GetBit(bitIndex);
        }

        /// <summary>
        /// Returns whether the specified bit index falls within the valid range of this slice.
        /// Useful for bounds checking before performing bit operations without triggering an assert.
        /// </summary>
        /// <param name="bitIndex">The zero-based bit index to check.</param>
        /// <returns>True if the bit index is within range; otherwise false.</returns>
        [[nodiscard]] bool IsBitInRange(size_t bitIndex) const noexcept
        {
            return _Head != nullptr && bitIndex < _SizeInBytes * 8;
        }

        /// <summary>
        /// Returns the total number of addressable bits in this slice.
        /// </summary>
        /// <returns>The size of the slice in bits.</returns>
        [[nodiscard]] size_t GetBitCount() const noexcept
        {
            return _SizeInBytes * 8;
        }
};
#endif