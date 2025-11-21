#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <getopt.h>

#define MAX_SIZE 2048
#define MAX_THREADS 32

typedef struct {
    int thread_id;
    int num_threads;
    int chunk_size;
    int schedule_type; // 0: static, 1: dynamic
    int n;
    double **A;
    double **B;
    double **C;
} thread_data_t;

// Configuration structure
typedef struct {
    int sizes[10];
    int num_sizes;
    int threads[10];
    int num_threads;
    int chunk_sizes[10];
    int num_chunk_sizes;
    int schedule_types[2]; // 0=static, 1=dynamic
    int num_schedule_types;
    int verbose;
    int test_all;
} config_t;

double **allocate_matrix(int n) {
    double **matrix = (double **)malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++) {
        matrix[i] = (double *)malloc(n * sizeof(double));
    }
    return matrix;
}

void free_matrix(double **matrix, int n) {
    for (int i = 0; i < n; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void initialize_matrix(double **matrix, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = (double)rand() / RAND_MAX;
        }
    }
}

void sequential_mm(double **A, double **B, double **C, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            C[i][j] = 0.0;
            for (int k = 0; k < n; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void *parallel_mm(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    int n = data->n;
    
    if (data->schedule_type == 0) { // Static scheduling
        for (int i = data->thread_id * data->chunk_size; i < n; i += data->num_threads * data->chunk_size) {
            for (int ii = i; ii < i + data->chunk_size && ii < n; ii++) {
                for (int j = 0; j < n; j++) {
                    data->C[ii][j] = 0.0;
                    for (int k = 0; k < n; k++) {
                        data->C[ii][j] += data->A[ii][k] * data->B[k][j];
                    }
                }
            }
        }
    } else { // Dynamic scheduling
        int next_row = 0;
        pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
        
        while (1) {
            int start_row;
            
            pthread_mutex_lock(&mutex);
            start_row = next_row;
            next_row += data->chunk_size;
            pthread_mutex_unlock(&mutex);
            
            if (start_row >= n) break;
            
            for (int i = start_row; i < start_row + data->chunk_size && i < n; i++) {
                for (int j = 0; j < n; j++) {
                    data->C[i][j] = 0.0;
                    for (int k = 0; k < n; k++) {
                        data->C[i][j] += data->A[i][k] * data->B[k][j];
                    }
                }
            }
        }
        pthread_mutex_destroy(&mutex);
    }
    return NULL;
}

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
}

void run_experiment(int n, int num_threads, int chunk_size, int schedule_type, int verbose) {
    double **A = allocate_matrix(n);
    double **B = allocate_matrix(n);
    double **C = allocate_matrix(n);
    
    initialize_matrix(A, n);
    initialize_matrix(B, n);
    
    pthread_t threads[MAX_THREADS];
    thread_data_t thread_data[MAX_THREADS];
    
    double start_time = get_time();
    
    if (num_threads == 1) {
        sequential_mm(A, B, C, n);
    } else {
        for (int i = 0; i < num_threads; i++) {
            thread_data[i].thread_id = i;
            thread_data[i].num_threads = num_threads;
            thread_data[i].chunk_size = chunk_size;
            thread_data[i].schedule_type = schedule_type;
            thread_data[i].n = n;
            thread_data[i].A = A;
            thread_data[i].B = B;
            thread_data[i].C = C;
            pthread_create(&threads[i], NULL, parallel_mm, &thread_data[i]);
        }
        
        for (int i = 0; i < num_threads; i++) {
            pthread_join(threads[i], NULL);
        }
    }
    
    double end_time = get_time();
    double execution_time = end_time - start_time;
    
    if (verbose) {
        printf("Size: %4d, Threads: %2d, Chunk: %3d, Schedule: %s, Time: %.4f sec\n",
               n, num_threads, chunk_size, 
               schedule_type == 0 ? "Static" : "Dynamic", execution_time);
    } else {
        printf("%d,%d,%d,%s,%.4f\n", n, num_threads, chunk_size,
               schedule_type == 0 ? "static" : "dynamic", execution_time);
    }
    
    free_matrix(A, n);
    free_matrix(B, n);
    free_matrix(C, n);
}

void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    printf("Options:\n");
    printf("  -s, --sizes SIZE1,SIZE2,...    Matrix sizes (comma-separated, default: 256,512,1024)\n");
    printf("  -t, --threads T1,T2,...        Thread counts (comma-separated, default: 1,2,4,8)\n");
    printf("  -c, --chunk C1,C2,...          Chunk sizes (comma-separated, default: 1,16,64)\n");
    printf("  --schedule TYPE1,TYPE2         Schedule types: static,dynamic (default: static)\n");
    printf("  -a, --all                      Run comprehensive test (all combinations)\n");
    printf("  -v, --verbose                  Verbose output\n");
    printf("  -h, --help                     Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s -s 512,1024 -t 4,8\n", program_name);
    printf("  %s --sizes 256,512,1024 --threads 2,4,8 --chunk 8,16\n", program_name);
    printf("  %s -a -v                        # Run all tests with verbose output\n", program_name);
}

void parse_comma_separated(const char *str, int *array, int *count) {
    char *copy = strdup(str);
    char *token = strtok(copy, ",");
    *count = 0;
    
    while (token != NULL && *count < 10) {
        array[(*count)++] = atoi(token);
        token = strtok(NULL, ",");
    }
    
    free(copy);
}

void init_default_config(config_t *config) {
    // Default sizes
    config->sizes[0] = 256;
    config->sizes[1] = 512;
    config->sizes[2] = 1024;
    config->num_sizes = 3;
    
    // Default threads
    config->threads[0] = 1;
    config->threads[1] = 2;
    config->threads[2] = 4;
    config->threads[3] = 8;
    config->num_threads = 4;
    
    // Default chunk sizes
    config->chunk_sizes[0] = 1;
    config->chunk_sizes[1] = 16;
    config->chunk_sizes[2] = 64;
    config->num_chunk_sizes = 3;
    
    // Default schedule types
    config->schedule_types[0] = 0; // static
    config->num_schedule_types = 1;
    
    config->verbose = 0;
    config->test_all = 0;
}

int parse_arguments(int argc, char *argv[], config_t *config) {
    init_default_config(config);
    
    static struct option long_options[] = {
        {"sizes", required_argument, 0, 's'},
        {"threads", required_argument, 0, 't'},
        {"chunk", required_argument, 0, 'c'},
        {"schedule", required_argument, 0, 'd'}, // 'd' for schedule
        {"all", no_argument, 0, 'a'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int c;
    while ((c = getopt_long(argc, argv, "s:t:c:avh", long_options, NULL)) != -1) {
        switch (c) {
            case 's':
                parse_comma_separated(optarg, config->sizes, &config->num_sizes);
                break;
            case 't':
                parse_comma_separated(optarg, config->threads, &config->num_threads);
                break;
            case 'c':
                parse_comma_separated(optarg, config->chunk_sizes, &config->num_chunk_sizes);
                break;
            case 'd': // schedule
                {
                    char *copy = strdup(optarg);
                    char *token = strtok(copy, ",");
                    config->num_schedule_types = 0;
                    
                    while (token != NULL && config->num_schedule_types < 2) {
                        if (strcmp(token, "static") == 0) {
                            config->schedule_types[config->num_schedule_types++] = 0;
                        } else if (strcmp(token, "dynamic") == 0) {
                            config->schedule_types[config->num_schedule_types++] = 1;
                        }
                        token = strtok(NULL, ",");
                    }
                    free(copy);
                }
                break;
            case 'a':
                config->test_all = 1;
                break;
            case 'v':
                config->verbose = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                exit(0);
            default:
                return -1;
        }
    }
    return 0;
}

void run_comprehensive_test(config_t *config) {
    if (config->verbose) {
        printf("=== Comprehensive Parallel Matrix Multiplication Test ===\n");
        printf("Matrix sizes: ");
        for (int i = 0; i < config->num_sizes; i++) {
            printf("%d ", config->sizes[i]);
        }
        printf("\nThread counts: ");
        for (int i = 0; i < config->num_threads; i++) {
            printf("%d ", config->threads[i]);
        }
        printf("\nChunk sizes: ");
        for (int i = 0; i < config->num_chunk_sizes; i++) {
            printf("%d ", config->chunk_sizes[i]);
        }
        printf("\nSchedule types: ");
        for (int i = 0; i < config->num_schedule_types; i++) {
            printf("%s ", config->schedule_types[i] == 0 ? "static" : "dynamic");
        }
        printf("\n\n");
    } else {
        printf("size,threads,chunk,schedule,time\n");
    }
    
    for (int s = 0; s < config->num_sizes; s++) {
        int size = config->sizes[s];
        
        if (config->verbose) {
            printf("--- Matrix Size: %dx%d ---\n", size, size);
        }
        
        for (int sch = 0; sch < config->num_schedule_types; sch++) {
            int schedule_type = config->schedule_types[sch];
            
            if (config->verbose) {
                printf("Schedule: %s\n", schedule_type == 0 ? "Static" : "Dynamic");
            }
            
            for (int c = 0; c < config->num_chunk_sizes; c++) {
                int chunk = config->chunk_sizes[c];
                
                if (config->verbose) {
                    printf("  Chunk Size: %d\n", chunk);
                }
                
                for (int t = 0; t < config->num_threads; t++) {
                    int threads = config->threads[t];
                    run_experiment(size, threads, chunk, schedule_type, config->verbose);
                }
                
                if (config->verbose) {
                    printf("\n");
                }
            }
        }
    }
}

void run_quick_test(config_t *config) {
    if (config->verbose) {
        printf("=== Quick Parallel Matrix Multiplication Test ===\n");
        printf("Testing basic configurations...\n\n");
    } else {
        printf("size,threads,chunk,schedule,time\n");
    }
    
    for (int s = 0; s < config->num_sizes; s++) {
        int size = config->sizes[s];
        
        // Test with default chunk size (16) and static scheduling
        for (int t = 0; t < config->num_threads; t++) {
            int threads = config->threads[t];
            run_experiment(size, threads, 16, 0, config->verbose);
        }
    }
}

int main(int argc, char *argv[]) {
    config_t config;
    
    if (parse_arguments(argc, argv, &config) != 0) {
        fprintf(stderr, "Error parsing arguments. Use -h for help.\n");
        return 1;
    }
    
    if (config.test_all) {
        run_comprehensive_test(&config);
    } else {
        run_quick_test(&config);
    }
    
    return 0;
}