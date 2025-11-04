#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>
#include <math.h>

// Function to initialize array with random values
void initialize_array(double *array, long long size) {
    #pragma omp parallel for
    for (long long i = 0; i < size; i++) {
        array[i] = (double)rand() / RAND_MAX * 1000.0; // Values between 0-1000
    }
}

// Sequential sum for verification
double sequential_sum(double *array, long long size) {
    double sum = 0.0;
    for (long long i = 0; i < size; i++) {
        sum += array[i];
    }
    return sum;
}

// Method 1: Reduction clause
double reduction_sum(double *array, long long size, double *computation_time) {
    double sum = 0.0;
    double start_time = omp_get_wtime();
    
    #pragma omp parallel for reduction(+:sum)
    for (long long i = 0; i < size; i++) {
        sum += array[i];
    }
    
    *computation_time = omp_get_wtime() - start_time;
    return sum;
}

// Method 2: Critical section
double critical_sum(double *array, long long size, double *computation_time) {
    double sum = 0.0;
    double start_time = omp_get_wtime();
    
    #pragma omp parallel
    {
        double local_sum = 0.0;
        
        #pragma omp for
        for (long long i = 0; i < size; i++) {
            local_sum += array[i];
        }
        
        #pragma omp critical
        {
            sum += local_sum;
        }
    }
    
    *computation_time = omp_get_wtime() - start_time;
    return sum;
}

// Method 3: Atomic operations
double atomic_sum(double *array, long long size, double *computation_time) {
    double sum = 0.0;
    double start_time = omp_get_wtime();
    
    #pragma omp parallel for
    for (long long i = 0; i < size; i++) {
        #pragma omp atomic
        sum += array[i];
    }
    
    *computation_time = omp_get_wtime() - start_time;
    return sum;
}

// Method 4: Manual reduction with private arrays
double manual_reduction_sum(double *array, long long size, double *computation_time) {
    double sum = 0.0;
    double start_time = omp_get_wtime();
    
    #pragma omp parallel
    {
        int num_threads = omp_get_num_threads();
        int thread_id = omp_get_thread_num();
        
        // Each thread computes its local sum
        double local_sum = 0.0;
        long long chunk_size = size / num_threads;
        long long start = thread_id * chunk_size;
        long long end = (thread_id == num_threads - 1) ? size : start + chunk_size;
        
        for (long long i = start; i < end; i++) {
            local_sum += array[i];
        }
        
        // Critical section to combine results
        #pragma omp critical
        {
            sum += local_sum;
        }
    }
    
    *computation_time = omp_get_wtime() - start_time;
    return sum;
}

// Method 5: Using locks (for comparison)
double lock_sum(double *array, long long size, double *computation_time) {
    double sum = 0.0;
    omp_lock_t lock;
    omp_init_lock(&lock);
    
    double start_time = omp_get_wtime();
    
    #pragma omp parallel
    {
        double local_sum = 0.0;
        
        #pragma omp for
        for (long long i = 0; i < size; i++) {
            local_sum += array[i];
        }
        
        omp_set_lock(&lock);
        sum += local_sum;
        omp_unset_lock(&lock);
    }
    
    *computation_time = omp_get_wtime() - start_time;
    omp_destroy_lock(&lock);
    return sum;
}

void print_results(const char* method, double time, double base_time, double sum, double expected_sum) {
    double error = fabs(sum - expected_sum) / expected_sum * 100.0;
    double speedup = base_time / time;
    
    printf("| %-20s | %10.6f | %8.2fx | %12.2f | %8.4f%% |\n", 
           method, time, speedup, sum, error);
}

int main() {
    printf("================================================================================\n");
    printf("               OPENMP REDUCTION PERFORMANCE COMPARISON\n");
    printf("================================================================================\n\n");
    
    // Test different array sizes
    long long sizes[] = {1000, 10000, 100000, 1000000, 10000000, 50000000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    const int num_trials = 3;
    
    // Set number of threads
    omp_set_num_threads(8);
    printf("Number of threads: %d\n", omp_get_max_threads());
    printf("Number of trials per method: %d\n\n", num_trials);
    
    for (int s = 0; s < num_sizes; s++) {
        long long size = sizes[s];
        printf("ARRAY SIZE: %'lld elements\n", size);
        printf("Memory usage: ~%.2f MB\n", (size * sizeof(double)) / (1024.0 * 1024.0));
        
        // Allocate and initialize array
        double *array = (double*)malloc(size * sizeof(double));
        if (array == NULL) {
            printf("Memory allocation failed for size %lld\n", size);
            continue;
        }
        
        initialize_array(array, size);
        
        // Get reference sequential sum
        double seq_start = omp_get_wtime();
        double expected_sum = sequential_sum(array, size);
        double seq_time = omp_get_wtime() - seq_start;
        
        printf("Sequential sum: %.2f (Time: %.6f seconds)\n", expected_sum, seq_time);
        printf("--------------------------------------------------------------------------------\n");
        printf("| Method               | Time (s)   | Speedup  | Result       | Error     |\n");
        printf("--------------------------------------------------------------------------------\n");
        
        double times[5] = {0}; // reduction, critical, atomic, manual, lock
        double sums[5];
        
        // Run multiple trials for each method
        for (int trial = 0; trial < num_trials; trial++) {
            double time;
            
            // Test each method
            sums[0] = reduction_sum(array, size, &time);
            times[0] += time;
            
            sums[1] = critical_sum(array, size, &time);
            times[1] += time;
            
            sums[2] = atomic_sum(array, size, &time);
            times[2] += time;
            
            sums[3] = manual_reduction_sum(array, size, &time);
            times[3] += time;
            
            sums[4] = lock_sum(array, size, &time);
            times[4] += time;
        }
        
        // Calculate average times
        for (int i = 0; i < 5; i++) {
            times[i] /= num_trials;
        }
        
        // Use reduction time as baseline for speedup calculation
        double baseline_time = times[0];
        
        // Print results for each method
        print_results("Reduction", times[0], baseline_time, sums[0], expected_sum);
        print_results("Critical Section", times[1], baseline_time, sums[1], expected_sum);
        print_results("Atomic", times[2], baseline_time, sums[2], expected_sum);
        print_results("Manual", times[3], baseline_time, sums[3], expected_sum);
        print_results("Lock", times[4], baseline_time, sums[4], expected_sum);
        
        printf("--------------------------------------------------------------------------------\n\n");
        
        free(array);
    }
    
    // Additional analysis: Thread scaling for medium-sized array
    printf("\nTHREAD SCALING ANALYSIS (Array size: 10,000,000)\n");
    printf("==================================================\n");
    
    long long test_size = 10000000;
    double *test_array = (double*)malloc(test_size * sizeof(double));
    initialize_array(test_array, test_size);
    
    int thread_counts[] = {1, 2, 4, 8, 16};
    int num_threads = sizeof(thread_counts) / sizeof(thread_counts[0]);
    
    printf("Threads | Reduction  | Critical   | Atomic     | Manual     | Speedup\n");
    printf("--------|------------|------------|------------|------------|--------\n");
    
    for (int t = 0; t < num_threads; t++) {
        omp_set_num_threads(thread_counts[t]);
        
        double red_time, crit_time, atomic_time, manual_time;
        
        reduction_sum(test_array, test_size, &red_time);
        critical_sum(test_array, test_size, &crit_time);
        atomic_sum(test_array, test_size, &atomic_time);
        manual_reduction_sum(test_array, test_size, &manual_time);
        
        double single_thread_time = (t == 0) ? red_time : 0;
        double speedup = (t == 0) ? 1.0 : single_thread_time / red_time;
        
        printf("   %2d   | %8.6f  | %8.6f  | %8.6f  | %8.6f  | %6.2fx\n",
               thread_counts[t], red_time, crit_time, atomic_time, manual_time, speedup);
    }
    
    free(test_array);
    
    printf("\nCONCLUSIONS:\n");
    printf("============\n");
    printf("1. REDUCTION is fastest - optimized private copies + efficient merging\n");
    printf("2. CRITICAL is good for medium-sized arrays with local accumulation\n");
    printf("3. ATOMIC is slowest for large arrays due to high contention\n");
    printf("4. MANUAL reduction offers flexibility but requires more code\n");
    printf("5. Performance differences become significant with larger arrays\n");
    
    return 0;
}