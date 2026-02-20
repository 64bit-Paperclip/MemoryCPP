
# MemoryManagerCPP

A high-performance C++ memory utility library and arena allocator with zero STL dependencies. Built around O(1) bump allocation and a rich slice type covering typed access, bit manipulation, sub-slicing, overlap detection, and raw copy utilities. Includes fixed and dynamic pool managers for full lifecycle control.

## Class Overview

Each class in this library is designed to be useful on its own, not just as part of the full stack.

**MEMORY_BLOCK** is a minimal RAII wrapper around a single heap allocation. Allocates on 
construction, frees on destruction. Useful anywhere you want a scoped, non-copyable heap 
buffer without pulling in smart pointers.

**MEMORY_SLICE** is a lightweight non-owning view into any region of memory with a rich 
utility API; typed access, sub-slicing, bounds checking, bit manipulation, copying, 
zeroing, and overlap detection. It has no dependency on the allocator that produced it 
and works equally well with stack memory, global buffers, or anything else.

**MEMORY_POOL** is a linear allocator backed by a `MEMORY_BLOCK`. Hands out memory via 
pointer increment with typed helpers for single objects and arrays or as a `MEMORY_SLICE`. Useful standalone 
wherever you need fast, deterministic allocation with a known lifetime.

**FIXED_MEMORY_MANAGER** orchestrates a compile-time fixed collection of pools stored 
contiguously inside its own footprint. No heap allocation beyond the pools themselves.

**DYNAMIC_MEMORY_MANAGER** manages a fixed number of pool slots at compile time but 
allows pools to be created and destroyed independently at runtime. Suitable for systems 
where pool sizes or lifetimes are not known upfront.

**MEMORY_MANAGER** is a thin facade over both managers, providing a single unified 
interface when you need both fixed and dynamic pools in the same system.

## Performance and Hardware Optimization

### Cache and Prefetcher Efficiency
Standard allocators scatter objects across the heap, causing cache misses, TLB pressure, and unpredictable memory access patterns. Because this system serves all allocations from a single contiguous block in sequential order, objects end up side by side in memory. This maximizes cache line utilization, reduces TLB pressure, and gives the CPU's hardware prefetcher a predictable access pattern to work with, keeping the pipeline saturated and avoiding memory stalls.

### Default 8-Byte Alignment
`TakeSlice` enforces 8-byte alignment on every allocation:

$$alignedReq = (sizeInBytes + 7) \& \sim 7$$

This ensures every allocation is cache-ready, avoids unaligned access penalties. For stricter alignment requirements (e.g. 16-byte SIMD), use `TakeAlignedSlice`.

## Technical Specifications

### Complexity Analysis

| Operation | Complexity | Notes |
| :--- | :--- | :--- |
| Allocation | O(1) | Single pointer addition and boundary check |
| Deallocation | N/A | Individual deallocation is not supported |
| Reset | O(1) | Resets a single integer offset to zero |
| Pool Access (compile-time) | O(1) | Direct address baked in at compile time |
| Pool Access (runtime) | O(1) | Single pointer arithmetic |
| Active Pool Count | O(1) | Cached counter maintained by create/delete |



## Design Notes

### Thread Safety
Pools are isolated by design. Each pool owns its own independent block of memory with 
no shared state between pools, so multiple threads operating on different pools 
concurrently is naturally safe without any synchronization.

Sharing a single pool across threads is not safe without external locking. The library 
provides no built-in synchronization, and this is intentional. Thread safety requirements 
are highly application-specific and imposing locks at the allocator level would add 
overhead to every allocation regardless of whether concurrency is needed. 


### No Individual Slice Deallocation
Individual slices cannot be freed. Once a slice is taken from a pool, that memory belongs 
to the pool until Reset() is called. Reset does not return memory to the OS; it simply 
resets the internal offset back to zero, making the entire block available for reuse. 
The underlying allocation lives for the lifetime of the pool.

This is a deliberate tradeoff. The system is designed for workloads where groups of 
allocations share a lifetime, a frame, a level, a request. When that lifetime ends, 
reset the pool and reuse the block.

At the pool level, `DYNAMIC_MEMORY_MANAGER` does support creating and destroying entire 
pools independently at runtime. But within a pool, slices are bump-allocated and live 
and die together. If you need individual object lifetimes, a different allocator strategy 
is more appropriate for that data.


### Debug vs Release Behavior
In debug builds, invalid operations assert immediately and loudly. These are programming 
mistakes and the library treats them as such. The assert is the safety net and the 
expectation is that correct code never triggers one.

In release builds, asserts are stripped and the library trusts the programmer. It does 
not add runtime overhead to compensate for mistakes that should not exist in correct code. 
Validation and bounds checking is the caller's responsibility.

Where a failure is a legitimate runtime condition rather than a programming mistake, the 
library will fail gracefully and signal the result in a way the caller cannot ignore. The 
distinction matters: a mistake that should never happen in correct code gets an assert, 
a condition outside the caller's control gets a graceful fallback.

The goal of this library is to never throw an exception or produce undefined behavior 
under correct usage. However, it does not achieve this by spending cycles validating 
every call at runtime. If the caller does not do their job, exceptions or crashes may 
occur. That is a feature, not a gap.
