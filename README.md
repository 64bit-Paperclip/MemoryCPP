
# MemoryManagerCPP

A modular, owner-based memory pool system for C++ that focuses on deterministic resource management. Designed to be lightweight, efficient, and high-performance with zero-allocation overhead after initialization. Provides ultra-fast bump allocation with an emphasis on cache coherency and hardware prefetcher optimization.

---

## Key Features

* **Zero External Dependencies / No-STL:** This library does not pull in heavy headers like `<vector>` or `<memory>`, keeping binary bloat to a minimum and ensuring lightning-fast compile times. It uses strictly `<malloc.h>` for initial memory reservations and `<assert.h>` for asserts.
* **Zero Runtime Allocation:** Once initialized, the system requires no further interaction with the OS heap. This prevents heap fragmentation and ensures your application's memory footprint remains predictable.
* **RAII & Type-Safe Slicing:** Uses `MEMORY_BLOCK` ownership to prevent leaks and `MEMORY_SLICE` views to provide bounded access to allocated segments, combining raw speed with modern C++ safety patterns.
* **Deterministic Resource Management:** Provides a "Zero-Jitter" environment. By eliminating the search-and-fit overhead of standard allocators, you achieve consistent, $O(1)$ allocation latency.

---

## Performance and Hardware Optimization

### 1. High-Density Spatial Locality
Standard allocators scatter objects across the heap, forcing the CPU to jump between disparate memory pages. This system enforces strict data contiguity. By keeping application memory in tight, organized blocks, you maximize **Cache Hit Rates** and significantly reduce **TLB (Translation Lookaside Buffer)** pressure.

### 2. Elimination of Cache Line Misses
Modern CPUs load data in 64-byte lines. When you use the `TakeArray` or sequential `Take<T>` calls, objects are placed side-by-side. Accessing the first object automatically pulls the next several objects into the L1 cache, ensuring the data is ready before the CPU even requests it.



### 3. Hardware Prefetcher Optimization
The linear bump-allocation strategy ensures that memory is served in a predictable, sequential fashion. This allows the CPU's hardware prefetcher to anticipate data requirements, keeping the execution pipeline saturated and eliminating costly memory stalls.

### 4. 8-Byte Alignment
The `TakeSlice` method enforces an 8-byte alignment:
$$alignedReq = (sizeInBytes + 7) \& \sim 7$$
This ensures that every allocation is "hot-ready" for the CPU, avoiding the massive performance penalties associated with unaligned memory access and ensuring compatibility with SIMD instructions.

---

## Memory Architecture

The system is built on a clear ownership hierarchy:

1. **MEMORY_MANAGER**: Orchestrates multiple pools. Uses templates to ensure contiguous pool storage and compile-time address resolution.
2. **MEMORY_POOL**: The high-performance Linear Allocator (Memory Arena). Memory is handed out via pointer increments.
3. **MEMORY_BLOCK**: The RAII owner of the raw heap pointer. Handles the `malloc` and `free` lifecycle.
4. **MEMORY_SLICE**: A lightweight, non-owning view representing a portion of a block.

---

## Technical Specifications

### Complexity Analysis
| Operation | Complexity | Description |
| :--- | :--- | :--- |
| **Allocation** | $O(1)$ | A single pointer addition and boundary check. |
| **Deallocation** | N/A | Individual deallocation is not supported. |
| **Reset** | $O(1)$ | Resets a single integer offset to zero. |
| **Pool Access** | $O(1)$ | Compile-time indexing via templates. |

### Memory Contiguity
Unlike many managers that store pointers to pools, `MEMORY_MANAGER<Count>` stores an array of `MEMORY_POOL` objects internally:
`MEMORY_POOL _Pools[Count];`
This ensures that even the metadata for your different memory zones is stored contiguously, further reducing the chance of a cache miss when switching between different allocation arenas.

---

### Deep Dive: MEMORY_MANAGER Architecture

The **MEMORY_MANAGER** is designed to provide zero-overhead orchestration for multiple memory arenas. By leveraging C++ templates and variadic parameter packs, it achieves performance characteristics impossible with standard runtime collections.

#### 1. In-Place Contiguous Storage
Most managers store an array of pointers to allocators, which adds a layer of indirection (pointer chasing). In this implementation, the **MEMORY_POOL** objects are stored inline within the manager's memory footprint as a fixed-size array.

This layout ensures that when the **MEMORY_MANAGER** is loaded into the CPU cache, the metadata for all managed pools (such as their current offsets and block heads) is likely to be loaded in the same cache line. This minimizes latency when application logic switches between different memory zones, such as moving from UI allocation to Physics allocation in a single update loop.

#### 2. Compile-Time Access via Template Indexing
The manager provides a template-based accessor that eliminates the runtime cost of array indexing. Because the index is a template parameter, it is known during compilation rather than at execution.

**Performance Benefits**:
* **Constant Folding**: The compiler evaluates the memory offset (the size of a `MEMORY_POOL` multiplied by the index) during the compilation phase.
* **Zero Runtime Lookup**: In the final assembly, the call site does not "search" for the pool. It uses a direct memory address or a fixed offset from the manager's starting pointer baked directly into the CPU instruction.
* **Static Validation**: A `static_assert` validates the index during the build. Unlike a standard array access that might require a runtime bounds check, this system fails to compile if an invalid index is used, ensuring both safety and speed.

#### 3. Variadic Constructor Expansion
The manager uses variadic templates to initialize the array of pools. This uses a Braced-Init-List Expansion to unpack the sizes provided in the constructor directly into the array elements. 

This ensures that even the initialization phase is free of temporary allocations, heap fragmentation, or intermediate containers, maintaining the library's strict "Zero-Jitter" philosophy from the moment the manager is instantiated.

#### 4. Comparison of Access Methods

* Template Access (GetPool<0>): Zero lookup cost. Compile-time safety. Resulting assembly is a direct pointer offset.
* Runtime Access (GetPool(i)): Low cost. Requires pointer arithmetic and a runtime offset calculation.

---

## Quick Start

### 1. Initializing the Manager
The `MEMORY_MANAGER` requires the number of pools as a template argument. Pass the sizes for each pool into the constructor.

```cpp
#include "memory_manager.h"

// 3 Pools: 1MB Graphics, 512KB Physics, 256KB Audio
MEMORY_MANAGER<3> EngineMemory(1048576, 524288, 262144);