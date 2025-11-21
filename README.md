# 🧩 Parallel Programming Labs

This repository contains comprehensive labs that explore parallel programming with OpenMP and Pthreads, focusing on parallel patterns, performance analysis, and optimization techniques.

## 📁 Repository Structure

```
├── lab1_helloworld.c    # OpenMP HelloWorld with data-sharing clauses
├── lab2_reduction.c     # Array sum using various reduction strategies  
├── lab3_primes.c        # Parallel prime number generation
├── matrix_mult.c        # Pthreads parallel matrix multiplication with performance analysis
└── README.md            # Project documentation
```

## 🔧 Compilation Instructions

### Prerequisites

* GCC compiler with OpenMP and Pthreads support
* Linux/Unix environment (recommended)
* Python with matplotlib (for performance plotting)

### Build Commands

#### OpenMP Labs

```bash
gcc -fopenmp lab1_helloworld.c -o lab1
gcc -fopenmp -O2 lab2_reduction.c -o lab2
gcc -fopenmp -O3 lab3_primes.c -o lab3 -lm
```

#### Pthreads Matrix Multiplication

```bash
gcc -O3 -o matrix_mult matrix_mult.c -lpthread -lm
```

---

## 🧠 Lab 1: Data-Sharing Clauses in OpenMP

### Objective

Understand how OpenMP handles variable visibility and scope using clauses such as `private`, `firstprivate`, and `shared`.

### Key Takeaways

* **Private:** Each thread gets its own uninitialized copy.
* **Firstprivate:** Each thread gets an initialized copy from the original variable.
* **Shared:** All threads access the same memory location.
* **Default behavior:** Variables outside parallel regions are shared unless otherwise specified.

### Learning Outcome

Grasp how data sharing impacts correctness and synchronization in parallel regions.

---

## ⚙️ Lab 2: Reduction Operations and Performance

### Objective

Compare various techniques for computing the sum of an array in parallel.

### Reduction Methods

* **Reduction Clause:** Fastest, optimized by OpenMP runtime.
* **Critical Section:** Safe but introduces moderate synchronization overhead.
* **Atomic Operations:** Simple but slow for large workloads.
* **Manual Reduction:** Requires explicit management of thread-local results.

### Performance Summary

| Method    | Speedup | Efficiency |
| --------- | ------- | ---------- |
| Reduction | 1.00x   | 100%       |
| Critical  | 0.74x   | 74%        |
| Atomic    | 0.15x   | 15%        |
| Manual    | 0.82x   | 82%        |

### Insights

* Built-in reduction is most efficient for scalable parallelism.
* Synchronization-heavy methods like atomic degrade performance.
* The performance gap widens as data size increases.

---

## 🔢 Lab 3: Parallel Prime Number Generation

### Objective

Implement and evaluate a parallel approach for generating prime numbers using OpenMP.

### Approach

* Estimate upper bound using the prime number theorem.
* Use dynamic scheduling to handle irregular workloads efficiently.
* Collect results sequentially to preserve order.

### Performance Results

| Prime Count | Sequential Time | Parallel Time | Speedup | Efficiency |
| ----------: | --------------- | ------------- | ------- | ---------- |
|          10 | 0.000005s       | 0.007079s     | 0.00x   | 0%         |
|         100 | 0.000153s       | 0.007785s     | 0.02x   | 2%         |
|       1,000 | 0.078218s       | 0.006898s     | 11.34x  | 283%*      |
|      10,000 | 0.270429s       | 0.035050s     | 7.72x   | 193%*      |

*Efficiency >100% indicates super-linear speedup due to cache effects

### Observations

* Parallel overhead dominates for small inputs.
* Dynamic scheduling ensures balanced workloads.
* Excellent scalability for large computations.

---

## 🧮 Lab 4: Parallel Matrix Multiplication with Pthreads

### Objective

Implement and analyze parallel matrix multiplication using Pthreads, exploring different scheduling strategies, chunk sizes, and thread counts.

### Features

* Dynamic argument parsing for flexible testing configurations
* Multiple scheduling strategies: Static and dynamic workload distribution
* Configurable chunk sizes for load balancing optimization
* Comprehensive performance analysis across matrix sizes and thread counts

### Usage Examples

```bash
# Basic test with default sizes
./matrix_mult

# Test specific matrix sizes
./matrix_mult -s 128,256,512

# Test with specific thread counts and chunk sizes
./matrix_mult -s 512,1024 -t 2,4,8 -c 8,16,32

# Compare scheduling strategies
./matrix_mult --schedule static,dynamic -v

# Comprehensive test with all combinations
./matrix_mult -a -v

# CSV output for data analysis
./matrix_mult -s 256,512 -t 1,4,8 > results.csv

# Show help
./matrix_mult -h
```

### Command Line Options

| Option        | Description                     | Default      |
| ------------- | ------------------------------- | ------------ |
| -s, --sizes   | Matrix sizes (comma-separated)  | 256,512,1024 |
| -t, --threads | Thread counts (comma-separated) | 1,2,4,8      |
| -c, --chunk   | Chunk sizes (comma-separated)   | 1,16,64      |
| --schedule    | Schedule types: static,dynamic  | static       |
| -a, --all     | Run comprehensive test          | false        |
| -v, --verbose | Verbose output                  | false        |
| -h, --help    | Show help message               | -            |

### Performance Insights

**Expected Scaling Behavior**

* Small matrices (128, 256): Poor scaling due to overhead dominance
* Medium matrices (512, 1024): Good scaling with near-linear speedup
* Large matrices (2048): Excellent scaling with computation dominance

**Scheduling Strategy Impact**

* Static: Lower overhead, best for uniform workloads and large matrices
* Dynamic: Better load balancing, best for irregular workloads and smaller matrices

**Chunk Size Optimization**

* Small chunks: Better load balancing but higher overhead
* Large chunks: Lower overhead but potential load imbalance
* Optimal: `chunk_size = max(1, n/(4*num_threads))`

**Sample Results**

```
Matrix 512x512 with 1 threads: 0.3567 seconds
Matrix 512x512 with 4 threads: 0.0945 seconds (3.77x speedup)
Matrix 512x512 with 8 threads: 0.0678 seconds (5.26x speedup)

Matrix 1024x1024 with 1 threads: 2.8912 seconds  
Matrix 1024x1024 with 4 threads: 0.7432 seconds (3.89x speedup)
Matrix 1024x1024 with 8 threads: 0.4123 seconds (7.01x speedup)
```

**Key Findings**

* Strong Scaling: Performance improves with more threads but efficiency decreases due to parallel overhead
* Memory Bound: Large matrices become memory-bandwidth limited rather than compute-bound
* Amdahl's Law: Perfect scaling is impossible due to sequential components in thread management

**Optimal Configurations**

* Small matrices: 4 threads, static, chunk=8
* Medium matrices: 8-16 threads, static, chunk=16-32
* Large matrices: 16-32 threads, static, chunk=32-64

---

## 🧩 Execution Examples

```bash
# OpenMP Labs
./lab1
./lab2
export OMP_NUM_THREADS=4 && ./lab3

# Pthreads Matrix Multiplication
./matrix_mult -s 512,1024 -t 2,4,8 -v
./matrix_mult -a > comprehensive_results.txt
```

---

## 📚 Learning Outcomes

<<<<<<< HEAD
1. **Understand OpenMP fundamentals** — parallel regions, data sharing, and work distribution.
2. **Analyze performance metrics** such as speedup, scalability, and efficiency.
3. **Differentiate between regular and irregular workloads** and select the right scheduling strategy.
=======
* Understand parallel programming fundamentals – OpenMP directives, Pthreads API, and synchronization
* Analyze performance metrics – speedup, scalability, efficiency, and overhead
* Compare parallelization strategies – data sharing, reduction operations, scheduling
* Optimize parallel applications – load balancing, chunk sizing, memory access patterns
* Differentiate between regular and irregular workloads and select appropriate parallelization approaches

---

## 🔍 Further Exploration

* Experiment with different matrix multiplication algorithms (Strassen, block decomposition)
* Test on systems with different core counts and memory hierarchies
* Compare OpenMP and Pthreads implementations for the same problem
* Explore GPU acceleration with CUDA or OpenCL for matrix operations
>>>>>>> ab9aca8 (added pthreads matrix multiplication code)
