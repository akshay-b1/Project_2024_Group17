#include <mpi.h>
#include <caliper/cali.h>
#include <adiak.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

#define MASTER 0

// Radix Sort Function
void radixSort(std::vector<int>& arr) {
    int max_element = *std::max_element(arr.begin(), arr.end());
    for (int exp = 1; max_element / exp > 0; exp *= 10) {
        std::vector<int> output(arr.size());
        int count[10] = {0};
        
        // Count occurrences
        for (int i = 0; i < arr.size(); i++)
            count[(arr[i] / exp) % 10]++;
        
        // Cumulative count
        for (int i = 1; i < 10; i++)
            count[i] += count[i - 1];
        
        // Build output array
        for (int i = arr.size() - 1; i >= 0; i--) {
            output[count[(arr[i] / exp) % 10] - 1] = arr[i];
            count[(arr[i] / exp) % 10]--;
        }
        
        // Copy to original array
        arr = output;
    }
}

int main(int argc, char** argv) {
    // Initialize MPI
    MPI_Init(&argc, &argv);
    
    adiak::init((void*)MPI_COMM_WORLD);
    adiak::launchdate();
    adiak::libraries();
    adiak::cmdline();
    adiak::clustername();

    // Variables to handle input size, process rank, and number of processors
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Check arguments for input size and type
    if (argc < 3) {
        if (rank == MASTER) {
            printf("Usage: %s <input_size> <input_type>\n", argv[0]);
            printf("Input types: 0 - Sorted, 1 - Sorted with 1%% Not Sorted, 2 - Reverse Sorted, 3 - Random\n");
        }
        MPI_Finalize();
        return 1;
    }

    // Setting input size and type
    int input_size = std::stoi(argv[1]);
    int input_type = std::stoi(argv[2]);
    std::vector<int> arr(input_size);

    // Generate different input types based on input_type argument
    int num_unsorted = 0;

    switch (input_type) {
        case 0:
            // Sorted
            for (int i = 0; i < input_size; i++) {
                arr[i] = i;
            }
            break;
        case 1:
            // Sorted with 1% not sorted
            for (int i = 0; i < input_size; i++) {
                arr[i] = i;
            }
            num_unsorted = input_size / 100;  // Assign after declaration
            for (int i = 0; i < num_unsorted; i++) {
                int idx1 = rand() % input_size;
                int idx2 = rand() % input_size;
                std::swap(arr[idx1], arr[idx2]);
            }
            break;
        case 2:
            // Reverse sorted
            for (int i = 0; i < input_size; i++) {
                arr[i] = input_size - i;
            }
            break;
        case 3:
            // Random
            srand(42);  // Seed for reproducibility
            for (int i = 0; i < input_size; i++) {
                arr[i] = rand() % input_size;
            }
            break;
        default:
            if (rank == MASTER) {
                printf("Invalid input type specified.\n");
            }
            MPI_Finalize();
            return 1;
    }

    
    // Collect Adiak metadata for input size and type
    adiak::value("input_size", input_size);
    adiak::value("data_type", "int");
    adiak::value("programming_model", "mpi");
    adiak::value("algorithm", "radix_sort");
    adiak::value("num_procs", size);
    adiak::value("implementation_source", "handwritten");
    adiak::value("input_type", input_type);
    
    // Caliper Region: Main
    CALI_MARK_BEGIN("main");
    
    // Caliper Region: Data Initialization
    CALI_MARK_BEGIN("data_init_runtime");
    // Input data is already initialized in array "arr"
    CALI_MARK_END("data_init_runtime");
    
    // Split data among processes (communication)
    int local_size = input_size / size;
    std::vector<int> local_arr(local_size);
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Scatter(arr.data(), local_size, MPI_INT, local_arr.data(), local_size, MPI_INT, MASTER, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");
    
    // Caliper Region: Computation - Radix Sort
    double comp_start = MPI_Wtime();
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    radixSort(local_arr);
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");
    double comp_end = MPI_Wtime();
    double comp_time = comp_end - comp_start;
    
    // Gather sorted sub-arrays back to the master process
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Gather(local_arr.data(), local_size, MPI_INT, arr.data(), local_size, MPI_INT, MASTER, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");
    
    // Caliper Region: Correctness Check
    CALI_MARK_BEGIN("correctness_check");
    if (rank == MASTER) {
        bool sorted = std::is_sorted(arr.begin(), arr.end());
        if (sorted) printf("Data is sorted correctly!\n");
        else printf("Data sorting failed.\n");
    }
    CALI_MARK_END("correctness_check");
    
    // Measure Performance Metrics
    double min_time, max_time, avg_time, total_time, variance;
    double local_total_time = comp_time;
    MPI_Reduce(&local_total_time, &min_time, 1, MPI_DOUBLE, MPI_MIN, MASTER, MPI_COMM_WORLD);
    MPI_Reduce(&local_total_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, MASTER, MPI_COMM_WORLD);
    MPI_Reduce(&local_total_time, &total_time, 1, MPI_DOUBLE, MPI_SUM, MASTER, MPI_COMM_WORLD);
    avg_time = total_time / size;
    double local_diff = (local_total_time - avg_time) * (local_total_time - avg_time);
    double sum_diff;
    MPI_Reduce(&local_diff, &sum_diff, 1, MPI_DOUBLE, MPI_SUM, MASTER, MPI_COMM_WORLD);
    variance = sum_diff / size;
    
    if (rank == MASTER) {
        printf("Min time/rank: %f seconds\n", min_time);
        printf("Max time/rank: %f seconds\n", max_time);
        printf("Avg time/rank: %f seconds\n", avg_time);
        printf("Total time: %f seconds\n", total_time);
        printf("Variance time/rank: %f seconds^2\n", variance);
    }
    
    // End Main Caliper Region
    CALI_MARK_END("main");
    
    // Finalize MPI
    MPI_Finalize();
    return 0;
}