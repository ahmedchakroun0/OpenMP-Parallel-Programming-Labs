# 🧩 OpenMP Parallel Programming Labs

This repository contains three comprehensive labs that explore parallel programming with OpenMP, focusing on data sharing, reduction operations, and parallel prime number generation.

Each lab demonstrates a core OpenMP concept, analyzes performance, and highlights best practices for parallel efficiency.

## 📁 Repository Structure

```
├── lab1_helloworld.c    # OpenMP HelloWorld with data-sharing clauses
├── lab2_reduction.c     # Array sum using various reduction strategies
├── lab3_primes.c        # Parallel prime number generation
└── README.md            # Project documentation
```

## 🔧 Compilation Instructions

### Prerequisites
- GCC compiler with OpenMP support
- Linux/Unix environment (recommended)

### Build Commands

```bash
# Lab 1: Data-sharing demonstration
gcc -fopenmp lab1_helloworld.c -o lab1

# Lab 2: Reduction performance comparison
gcc -fopenmp -O2 lab2_reduction.c -o lab2

# Lab 3: Prime number generation
gcc -fopenmp -O3 lab3_primes.c -o lab3 -lm
```

## 🧠 Lab 1: Data-Sharing Clauses in OpenMP

### Objective
Understand how OpenMP handles variable visibility and scope using clauses such as `private`, `firstprivate`, and `shared`.

### Key Takeaways
- **Private**: Each thread gets its own uninitialized copy.
- **Firstprivate**: Each thread gets an initialized copy from the original variable.
- **Shared**: All threads access the same memory location.
- **Default behavior**: Variables outside parallel regions are shared unless otherwise specified.

### Learning Outcome
Grasp how data sharing impacts correctness and synchronization in parallel regions.

## ⚙️ Lab 2: Reduction Operations and Performance

### Objective
Compare various techniques for computing the sum of an array in parallel.

### Reduction Methods
1. **Reduction Clause** – Fastest, optimized by OpenMP runtime.
2. **Critical Section** – Safe but introduces moderate synchronization overhead.
3. **Atomic Operations** – Simple but slow for large workloads.
4. **Manual Reduction** – Requires explicit management of thread-local results.

### Performance Summary

| Method     | Speedup | Efficiency |
|------------|---------|------------|
| Reduction  | 1.00x   | 100%       |
| Critical   | 0.74x   | 74%        |
| Atomic     | 0.15x   | 15%        |
| Manual     | 0.82x   | 82%        |

### Insights
- Built-in reduction is most efficient for scalable parallelism.
- Synchronization-heavy methods like atomic degrade performance.
- The performance gap widens as data size increases.

## 🔢 Lab 3: Parallel Prime Number Generation

### Objective
Implement and evaluate a parallel approach for generating prime numbers using OpenMP.

### Approach
1. Estimate upper bound using the prime number theorem.
2. Use dynamic scheduling to handle irregular workloads efficiently.
3. Collect results sequentially to preserve order.

### Performance Results

| Prime Count | Sequential Time | Parallel Time | Speedup | Efficiency |
|-------------|-----------------|---------------|---------|------------|
| 10          | 0.000005s       | 0.007079s     | 0.00x   | 0%         |
| 100         | 0.000153s       | 0.007785s     | 0.02x   | 2%         |
| 1,000       | 0.078218s       | 0.006898s     | 11.34x  | 283%*      |
| 10,000      | 0.270429s       | 0.035050s     | 7.72x   | 193%*      |
| 100,000     | —               | —             | —       | —          |

**Efficiency >100% indicates super-linear speedup due to cache effects*

### Observations
- Parallel overhead dominates for small inputs.
- Dynamic scheduling ensures balanced workloads.
- Excellent scalability for large computations.

## 🧩 Execution Examples

```bash
# Lab 1
./lab1

# Lab 2
./lab2

# Lab 3 (with 4 threads)
export OMP_NUM_THREADS=4
./lab3
```

## 📚 Learning Outcomes

1. **Understand OpenMP fundamentals** — parallel regions, data sharing, and work distribution.
2. **Analyze performance metrics** such as speedup, scalability, and efficiency.
3. **Differentiate between regular and irregular workloads** and select the right scheduling strategy.
