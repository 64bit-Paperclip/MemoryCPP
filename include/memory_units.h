#pragma once


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