// ============================================================================
// MemoryCPP - High Performance Arena Allocator & Memory Utility Library
// ----------------------------------------------------------------------------
// File:        memory_units.h
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

#ifndef __MEMORY_UNITS_H_GUARD
#define __MEMORY_UNITS_H_GUARD

#include <stddef.h>


namespace MemoryUnits
{
    constexpr size_t KBToBytes(size_t n) noexcept { return n * 1024; }
    constexpr size_t MBToBytes(size_t n) noexcept { return n * 1024 * 1024; }
    constexpr size_t GBToBytes(size_t n) noexcept { return n * 1024 * 1024 * 1024; }
}


#endif