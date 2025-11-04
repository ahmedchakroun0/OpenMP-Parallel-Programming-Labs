#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <time.h>

// Check if a number is prime
int is_prime(int n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;
    
    int limit = (int)sqrt(n);
    for (int i = 3; i <= limit; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

// Find primes sequentially
void find_primes_sequential(int target_count, int *primes, double *elapsed_time) {
    clock_t start = clock();
    
    int count = 0;
    int num = 2;
    
    while (count < target_count) {
        if (is_prime(num)) {
            primes[count++] = num;
        }
        num++;
    }
    
    clock_t end = clock();
    *elapsed_time = ((double)(end - start)) / CLOCKS_PER_SEC;
}

// Estimate upper bound for nth prime number
int estimate_nth_prime(int n) {
    if (n < 6) return 15;
    double log_n = log(n);
    return (int)(n * (log_n + log(log_n) + 2));
}

// Find primes in parallel using OpenMP
void find_primes_parallel(int target_count, int *primes, double *elapsed_time) {
    double start = omp_get_wtime();
    
    int upper_bound = estimate_nth_prime(target_count);
    int *is_prime_array = (int*)calloc(upper_bound + 1, sizeof(int));
    
    // Mark primes in parallel
    #pragma omp parallel for schedule(dynamic)
    for (int i = 2; i <= upper_bound; i++) {
        if (is_prime(i)) {
            is_prime_array[i] = 1;
        }
    }
    
    // Collect primes sequentially (to maintain order)
    int count = 0;
    for (int i = 2; i <= upper_bound && count < target_count; i++) {
        if (is_prime_array[i]) {
            primes[count++] = i;
        }
    }
    
    free(is_prime_array);
    
    double end = omp_get_wtime();
    *elapsed_time = end - start;
}

// Display results
void display_results(int target_count, int *primes, double seq_time, double par_time) {
    printf("\n============================================================\n");
    printf("Finding %d prime numbers\n", target_count);
    printf("============================================================\n");
    printf("Sequential time: %.6f seconds\n", seq_time);
    printf("Parallel time:   %.6f seconds\n", par_time);
    printf("Speedup:         %.2fx\n", seq_time / par_time);
    
    if (target_count <= 10) {
        printf("Primes: ");
        for (int i = 0; i < target_count; i++) {
            printf("%d ", primes[i]);
        }
        printf("\n");
    } else {
        printf("First 5 primes: ");
        for (int i = 0; i < 5; i++) {
            printf("%d ", primes[i]);
        }
        printf("\nLast 5 primes:  ");
        for (int i = target_count - 5; i < target_count; i++) {
            printf("%d ", primes[i]);
        }
        printf("\n");
    }
}

int main() {
    int test_sizes[] = {10, 100, 1000, 10000, 100000};
    int num_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);
    
    // Get number of threads
    int num_threads = omp_get_max_threads();
    printf("Using OpenMP with %d threads\n", num_threads);
    
    for (int i = 0; i < num_tests; i++) {
        int target = test_sizes[i];
        
        // Allocate memory for primes
        int *primes_seq = (int*)malloc(target * sizeof(int));
        int *primes_par = (int*)malloc(target * sizeof(int));
        
        double seq_time, par_time;
        
        // Sequential execution
        find_primes_sequential(target, primes_seq, &seq_time);
        
        // Parallel execution
        find_primes_parallel(target, primes_par, &par_time);
        
        // Display results
        display_results(target, primes_par, seq_time, par_time);
        
        // Verify results match
        int match = 1;
        for (int j = 0; j < target; j++) {
            if (primes_seq[j] != primes_par[j]) {
                match = 0;
                break;
            }
        }
        printf("Results match: %s\n", match ? "YES" : "NO");
        
        free(primes_seq);
        free(primes_par);
    }
    
    return 0;
}

/*
 * Compilation instructions:
 * 
 * With GCC:
 * gcc -fopenmp -O3 -lm parallel_primes.c -o parallel_primes
 * 
 * With Clang:
 * clang -fopenmp -O3 -lm parallel_primes.c -o parallel_primes
 * 
 * Execution:
 * ./parallel_primes
 * 
 * To set number of threads:
 * export OMP_NUM_THREADS=4
 * ./parallel_primes
 */