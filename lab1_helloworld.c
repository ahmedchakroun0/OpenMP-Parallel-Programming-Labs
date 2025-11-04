#include <stdio.h>
#include <omp.h>

int main() {
    int a = 10, b = 20, c = 30;
    int thread_id;
    
    printf("=== OpenMP HelloWorld with Different Clauses ===\n\n");
    
    // Initial values
    printf("Before parallel region:\n");
    printf("a = %d, b = %d, c = %d\n\n", a, b, c);
    
    printf("=== 1. DEFAULT (SHARED) BEHAVIOR ===\n");
    #pragma omp parallel private(thread_id)
    {
        thread_id = omp_get_thread_num();
        if (thread_id == 0) {
            printf("Number of threads: %d\n", omp_get_num_threads());
        }
        printf("Thread %d: Hello World! (a=%d, b=%d, c=%d)\n", 
               thread_id, a, b, c);
        
        // Modify variables
        a += thread_id;
        b += thread_id;
        c += thread_id;
        
        printf("Thread %d after modification: a=%d, b=%d, c=%d\n", 
               thread_id, a, b, c);
    }
    printf("After parallel region: a=%d, b=%d, c=%d\n\n", a, b, c);
    
    // Reset values
    a = 10; b = 20; c = 30;
    
    printf("=== 2. WITH PRIVATE CLAUSE ===\n");
    #pragma omp parallel private(a, thread_id) shared(b, c)
    {
        thread_id = omp_get_thread_num();
        printf("Thread %d: a=%d (private), b=%d (shared), c=%d (shared)\n", 
               thread_id, a, b, c);
        
        // Modify variables
        a = 100 + thread_id;  // Each thread gets its own copy
        b += thread_id;       // Shared - race condition!
        c += thread_id;       // Shared - race condition!
        
        printf("Thread %d after modification: a=%d, b=%d, c=%d\n", 
               thread_id, a, b, c);
    }
    printf("After parallel region: a=%d, b=%d, c=%d\n\n", a, b, c);
    
    // Reset values
    a = 10; b = 20; c = 30;
    
    printf("=== 3. WITH FIRSTPRIVATE CLAUSE ===\n");
    #pragma omp parallel firstprivate(a) private(thread_id) shared(b, c)
    {
        thread_id = omp_get_thread_num();
        printf("Thread %d: a=%d (firstprivate), b=%d (shared), c=%d (shared)\n", 
               thread_id, a, b, c);
        
        // Modify variables
        a += thread_id;       // Each thread gets initialized copy
        b += thread_id;       // Shared - race condition!
        c += thread_id;       // Shared - race condition!
        
        printf("Thread %d after modification: a=%d, b=%d, c=%d\n", 
               thread_id, a, b, c);
    }
    printf("After parallel region: a=%d, b=%d, c=%d\n\n", a, b, c);
    
    // Reset values
    a = 10; b = 20; c = 30;
    
    printf("=== 4. MIXED CLAUSES ===\n");
    #pragma omp parallel private(a) firstprivate(b) shared(c) private(thread_id)
    {
        thread_id = omp_get_thread_num();
        printf("Thread %d: a=%d (private), b=%d (firstprivate), c=%d (shared)\n", 
               thread_id, a, b, c);
        
        // Modify variables
        a = 50 + thread_id;   // Private - uninitialized
        b += thread_id;       // Firstprivate - initialized copy
        c += thread_id;       // Shared - race condition!
        
        printf("Thread %d after modification: a=%d, b=%d, c=%d\n", 
               thread_id, a, b, c);
    }
    printf("After parallel region: a=%d, b=%d, c=%d\n\n", a, b, c);
    
    printf("=== 5. VARIABLES INSIDE/OUTSIDE REGION ===\n");
    int outside_var = 100;
    printf("Before parallel: outside_var = %d\n", outside_var);
    
    #pragma omp parallel private(thread_id)
    {
        int inside_var = 200 + omp_get_thread_num();
        thread_id = omp_get_thread_num();
        
        printf("Thread %d: outside_var = %d, inside_var = %d\n", 
               thread_id, outside_var, inside_var);
        
        outside_var += thread_id;  // Modifying shared variable
        inside_var += thread_id;   // Local to each thread
        
        printf("Thread %d after mod: outside_var = %d, inside_var = %d\n", 
               thread_id, outside_var, inside_var);
    }
    printf("After parallel: outside_var = %d\n", outside_var);
    // inside_var is not accessible here - it's local to parallel region
    
    return 0;
}