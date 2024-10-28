 #include <mpi.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <caliper/cali.h>
#include <adiak.hpp>
#include <string>

#define MASTER 0

void bitonicCompare(std::vector<int>& arr, int i, int j, bool dir) {
    if (dir == (arr[i] > arr[j])) {
        std::swap(arr[i], arr[j]);
    }
}

// Bitonic merge function
void bitonicMerge(std::vector<int>& arr, int low, int cnt, bool dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        for (int i = low; i < low + k; i++) {
            bitonicCompare(arr, i, i + k, dir);
        }
        bitonicMerge(arr, low, k, dir);
        bitonicMerge(arr, low + k, k, dir);
    }
}

// Bitonic sort function
void bitonicSort(std::vector<int>& arr, int low, int cnt, bool dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        bitonicSort(arr, low, k, true);  // Sort in ascending order
        bitonicSort(arr, low + k, k, false);  // Sort in descending order
        bitonicMerge(arr, low, cnt, dir);  // Merge the result
    }
}

// Parallel bitonic sort with MPI
void parallelBitonicSort(std::vector<int>& local_arr, int world_rank, int world_size) {
    int local_size = local_arr.size();
    
    // First, sort local arrays
    bitonicSort(local_arr, 0, local_size, true);
    
    // Then perform parallel merging stages
    for (int step = world_size/2; step > 0; step /= 2) {
        // Calculate if this process should sort ascending or descending
        int group = world_rank / (2 * step);
        bool ascending = (group % 2 == 0);
        
        // Find partner process
        int partner = world_rank ^ step;
        
        // Exchange data with partner
        std::vector<int> partner_arr(local_size);
        MPI_Sendrecv(local_arr.data(), local_size, MPI_INT, 
                     partner, 0,
                     partner_arr.data(), local_size, MPI_INT, 
                     partner, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // Merge the arrays
        std::vector<int> merged(local_size * 2);
        if (world_rank < partner) {
            // If I'm the lower rank, I should keep the min/max values based on ascending
            for (int i = 0; i < local_size; i++) {
                if (ascending) {
                    merged[i] = std::min(local_arr[i], partner_arr[i]);
                    merged[i + local_size] = std::max(local_arr[i], partner_arr[i]);
                } else {
                    merged[i] = std::max(local_arr[i], partner_arr[i]);
                    merged[i + local_size] = std::min(local_arr[i], partner_arr[i]);
                }
            }
            // Keep the first half
            std::copy(merged.begin(), merged.begin() + local_size, local_arr.begin());
        } else {
            // If I'm the higher rank, I should keep the opposite values
            for (int i = 0; i < local_size; i++) {
                if (ascending) {
                    merged[i] = std::min(local_arr[i], partner_arr[i]);
                    merged[i + local_size] = std::max(local_arr[i], partner_arr[i]);
                } else {
                    merged[i] = std::max(local_arr[i], partner_arr[i]);
                    merged[i + local_size] = std::min(local_arr[i], partner_arr[i]);
                }
            }
            // Keep the second half
            std::copy(merged.begin() + local_size, merged.end(), local_arr.begin());
        }
    }
}


void data_init_runtime(std::vector<int>& arr, int input_size, int input_type) {
    arr.resize(input_size);
    int num_unsorted = 0;

    switch (input_type) {
        case 0: // Sorted
            for (int i = 0; i < input_size; i++) {
                arr[i] = i;
            }
            break;
        case 1: // Reverse sorted
            for (int i = 0; i < input_size; i++) {
                arr[i] = input_size - i;
            }
            break;
        case 2: // Random
            srand(42);
            for (int i = 0; i < input_size; i++) {
                arr[i] = rand() % input_size;
            }
            break;
        case 3: // 1% perturbed (mostly sorted with 1% random swaps)
            // Initialize sorted array
            for (int i = 0; i < input_size; i++) {
                arr[i] = i;
            }
            // Calculate 1% of the array size
            num_unsorted = input_size / 100;

            srand(static_cast<unsigned>(time(0))); // Seed for random swaps
            for (int i = 0; i < num_unsorted; i++) {
                int idx1 = rand() % input_size;
                int idx2 = rand() % input_size;
                std::swap(arr[idx1], arr[idx2]); // Randomly swap two elements
            }
            break;
    }
}

bool correctness_check(const std::vector<int>& arr) {
    return std::is_sorted(arr.begin(), arr.end());
}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);
    CALI_MARK_BEGIN("main");
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    adiak::init(NULL);
    adiak::launchdate();
    adiak::libraries();
    adiak::cmdline();
    adiak::clustername();
    adiak::value("algorithm", "bitonic");
    adiak::value("programming_model", "mpi");
    adiak::value("data_type", "int");
    adiak::value("size_of_data_type", sizeof(int));

    if (argc != 3) {
        if (world_rank == MASTER) {
            std::cerr << "Usage: " << argv[0] << " <input_size> <input_type>" << std::endl;
            std::cerr << "Input types: 0 - Sorted, 1 - Sorted with 1% Not Sorted, 2 - Reverse Sorted, 3 - Random" << std::endl;
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int input_size = std::atoi(argv[1]);
    int input_type = std::atoi(argv[2]);

    adiak::value("input_size", input_size);
    adiak::value("input_type", input_type);
    adiak::value("num_procs", world_size);
    adiak::value("scalability", "strong");
    adiak::value("group_num", 17);
    adiak::value("implementation_source", "handwritten");

    std::vector<int> arr(input_size);
    if (world_rank == MASTER) {
        CALI_MARK_BEGIN("data_init_runtime");
        data_init_runtime(arr, input_size, input_type);
        CALI_MARK_END("data_init_runtime");
    }

    int local_size = input_size / world_size;
    std::vector<int> local_arr(local_size);

    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Scatter(arr.data(), local_size, MPI_INT, local_arr.data(), local_size, MPI_INT, MASTER, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    MPI_Barrier(MPI_COMM_WORLD);

    double start_time = MPI_Wtime();
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    parallelBitonicSort(local_arr, world_rank, world_size);
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");
    double end_time = MPI_Wtime();

    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    MPI_Gather(local_arr.data(), local_size, MPI_INT, arr.data(), local_size, MPI_INT, MASTER, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    if (world_rank == MASTER) {
        CALI_MARK_BEGIN("correctness_check");
        bool is_sorted = correctness_check(arr);
        CALI_MARK_END("correctness_check");
        std::cout << std::endl;
        if (is_sorted) {
            std::cout << "Array is correctly sorted." << std::endl;
        } else {
            std::cout << "Array is not correctly sorted." << std::endl;
        }
    }

    double local_time = end_time - start_time;
    double max_time, min_time, avg_time, total_time;

    MPI_Reduce(&local_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, MASTER, MPI_COMM_WORLD);
    MPI_Reduce(&local_time, &min_time, 1, MPI_DOUBLE, MPI_MIN, MASTER, MPI_COMM_WORLD);
    MPI_Reduce(&local_time, &total_time, 1, MPI_DOUBLE, MPI_SUM, MASTER, MPI_COMM_WORLD);

    if (world_rank == MASTER) {
        avg_time = total_time / world_size;
        std::cout << "Min time: " << min_time << " seconds" << std::endl;
        std::cout << "Max time: " << max_time << " seconds" << std::endl;
        std::cout << "Avg time: " << avg_time << " seconds" << std::endl;
        std::cout << "Total time: " << total_time << " seconds" << std::endl;
    }
    CALI_MARK_END("main");
    MPI_Finalize();


    return 0;
}