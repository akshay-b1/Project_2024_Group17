#include <mpi.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <caliper/cali.h>
#include <adiak.hpp>

#define MASTER 0

void bitonicCompare(std::vector<int>& arr, int i, int j, bool dir) {
    if (dir == (arr[i] > arr[j])) {
        std::swap(arr[i], arr[j]);
    }
}

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

void bitonicSort(std::vector<int>& arr, int low, int cnt, bool dir) {
    if (cnt > 1) {
        int k = cnt / 2;
        bitonicSort(arr, low, k, true);
        bitonicSort(arr, low + k, k, false);
        bitonicMerge(arr, low, cnt, dir);
    }
}

void parallelBitonicSort(std::vector<int>& local_arr, int world_rank, int world_size) {
    CALI_CXX_MARK_FUNCTION;
    int local_size = local_arr.size();

    for (int step = 2; step <= world_size * local_size; step *= 2) {
        for (int substep = step / 2; substep > 0; substep /= 2) {
            for (int i = 0; i < local_size; i++) {
                int j = i ^ substep;
                int proc = j / local_size;

                if (proc == world_rank) {
                    bitonicCompare(local_arr, i % local_size, j % local_size, (i / step) % 2 == 0);
                } else {
                    int partner_rank = world_rank ^ (substep / local_size);
                    int send_val = local_arr[i];
                    int recv_val;

                    CALI_MARK_BEGIN("comm");
                    MPI_Sendrecv(&send_val, 1, MPI_INT, partner_rank, 0,
                                 &recv_val, 1, MPI_INT, partner_rank, 0,
                                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    CALI_MARK_END("comm");

                    if ((i / step) % 2 == 0) {
                        local_arr[i] = std::min(send_val, recv_val);
                    } else {
                        local_arr[i] = std::max(send_val, recv_val);
                    }
                }
            }
        }
    }
}

void data_init_runtime(std::vector<int>& arr, int input_size, int input_type) {
    CALI_CXX_MARK_FUNCTION;
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
    }
}

bool correctness_check(const std::vector<int>& arr) {
    CALI_CXX_MARK_FUNCTION;
    return std::is_sorted(arr.begin(), arr.end());
}

int main(int argc, char** argv) {
    CALI_CXX_MARK_FUNCTION;

    MPI_Init(&argc, &argv);

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

    MPI_Finalize();
    return 0;
}