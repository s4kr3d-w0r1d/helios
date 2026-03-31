# Helios: High-Frequency Trading Matching Engine

Helios is an ultra-low latency **Limit Order Book (LOB) matching engine** written in **C++20**.  
It is engineered with deep awareness of modern CPU architecture, aggressively bypassing standard OS abstractions to achieve **deterministic, sub-microsecond execution times**.

> Disclaimer: This is an educational project demonstrating low-level systems programming and hardware-aware optimization. It does NOT connect to real financial exchanges.

---

## Core Architectural Optimizations

Helios is built on four pillars of extreme performance:

### 1. Lock-Free Concurrency

- **SPSC Ring Buffer**  
  Custom Single-Producer Single-Consumer queue between Gateway and Engine.

- **Explicit Memory Semantics**  
  Uses `memory_order_acquire` / `memory_order_release` instead of `seq_cst`.

- **False Sharing Elimination**  
  `alignas(64)` separation of atomic indices to avoid cache-line contention (MESI protocol).

---

### 2. User-Space Memory Management

- **Zero OS Heap Allocations**  
  No `malloc/new` → avoids `brk/mmap` overhead.

- **Custom O(1) Memory Arena**  
  Contiguous memory with predictable allocation.

- **Placement New Overloading**  
  Orders directly allocated via arena with zero overhead free-list.

---

### 3. Cache-Local Data Structures

- **Contiguous Price Levels**  
  Uses `std::vector` instead of tree structures for perfect prefetching.

- **Intrusive Lists**  
  Zero-allocation FIFO queues at each price level.

- **O(1) Cancellations**  
  Direct pointer access via `unordered_map`.

---

### 4. Kernel Bypass & CPU Isolation

- **Thread Affinity**  
  `pthread_setaffinity_np` pins threads to isolated cores.

- **Real-Time Scheduling**  
  `SCHED_FIFO` avoids preemption by Linux CFS.

- **Hardware Telemetry**  
  Uses `__rdtsc()` for cycle-accurate latency measurement.

---

## Project Structure

```
helios/
├── CMakeLists.txt
├── include/
│   ├── concurrency/    # Lock-free SPSC queue
│   ├── engine/         # LOB and matching logic
│   ├── memory/         # Arena + Intrusive List
│   └── utils/          # Thread pinning, RDTSC
├── src/
│   ├── engine/         
│   ├── utils/          
│   ├── main.cpp        # Engine + Gateway threads
│   └── benchmark.cpp   # SPSC throughput test
├── tests/              # GoogleTest suite
└── scripts/            # Build scripts
```

---

## Prerequisites

- **OS**: Linux (Ubuntu / Fedora / Debian)
- **Compiler**: GCC / Clang (C++20 support)
- **Build System**: CMake ≥ 3.14
- **Libraries**: pthread

---

## Building the Engine

### Maximum Performance Build

```bash
./scripts/build_release.sh
```

- Enables `-O3`
- Link Time Optimization (LTO)

---

### Standard Build

```bash
./scripts/build.sh
```

- Generates `compile_commands.json`
- Useful for debugging / tooling

---

## Running the System

### 1. Matching Engine

> Requires root for real-time scheduling

```bash
# Default: 10M orders
sudo ./build/hft_engine

# Custom workload
sudo ./build/hft_engine 5000000
```

---

### 2. Test Suite

```bash
./scripts/build_and_test.sh
```

Validates:

- Memory correctness
- FIFO matching
- Thread safety

---

### 3. Lock-Free Queue Benchmark

```bash
./build/run_benchmark
```

---

## Telemetry & Benchmarking

Helios uses calibrated **Time Stamp Counter (TSC)** to measure end-to-end latency.

### Example Output

```
=== Starting HFT Matching Engine (Isolated Core Mode) ===

[Telemetry] Calibrating CPU Time Stamp Counter (TSC)...
[Telemetry] CPU Frequency Calibrated: 3.20 GHz
[System] Engine thread elevated to SCHED_FIFO Real-Time priority.

[Engine] Processed 10000000 orders in X.XX ms
[Engine] Throughput: XXXXXXX ops/sec

=== Engine Latency Histogram (Nanoseconds) ===
----------------------------------------------
 Metric    | Cycles       | Time (ns)
----------------------------------------------
 Min       | ...
 p50       | ...
 p90       | ...
 p99       | ...
 p99.9     | ...
 Max       | ...
----------------------------------------------
```

---

## Key Takeaways

- Designed with **mechanical sympathy**
- Eliminates OS bottlenecks wherever possible
- Fully deterministic execution path
- Focus on **cache locality, lock-free design, and hardware-level timing**

---

## Future Improvements

- NUMA-aware memory allocation
- SIMD-based matching optimizations
- Kernel bypass networking (DPDK / AF_XDP)
- Persistent memory logging
