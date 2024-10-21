#include <mpi.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <random>
#include <cstdlib>
#include <chrono>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>
#include <sstream>

void merge(std::vector<int>& arr, long long left, long long mid, long long right) {
    std::vector<int> temp(right - left + 1);
    int i = left, j = mid + 1, k = 0;

    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            temp[k++] = arr[i++];
        } else {
            temp[k++] = arr[j++];
        }
    }

    while (i <= mid) {
        temp[k++] = arr[i++];
    }

    while (j <= right) {
        temp[k++] = arr[j++];
    }

    for (int p = 0; p < k; p++) {
        arr[left + p] = temp[p];
    }
}

void mergeSort(std::vector<int>& arr, long long left, long long right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

void data_init_runtime(std::vector<int>& arr, long long input_size, const std::string& input_type) {
    CALI_MARK_BEGIN("data_init_runtime");
    arr.resize(input_size);
    if (input_type == "Sorted") {
        std::iota(arr.begin(), arr.end(), 0);
    } else if (input_type == "ReverseSorted") {
        std::iota(arr.rbegin(), arr.rend(), 0);
    } else if (input_type == "Random") {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, input_size - 1);
        for (int& num : arr) {
            num = dis(gen);
        }
    } else if (input_type == "1_perc_perturbed") {
        std::iota(arr.begin(), arr.end(), 0);
        int num_perturbed = input_size / 100;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, input_size - 1);
        for (int i = 0; i < num_perturbed; ++i) {
            int idx1 = dis(gen);
            int idx2 = dis(gen);
            std::swap(arr[idx1], arr[idx2]);
        }
    }
    CALI_MARK_END("data_init_runtime");
}

void parallelMergeSort(std::vector<int>& arr, int world_rank, int world_size) {    
    size_t total_size = (world_rank == 0) ? arr.size() : 0;
    MPI_Bcast(&total_size, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
    
    size_t base_chunk_size = total_size / world_size;
    size_t remainder = total_size % world_size;
    
    std::vector<int> sendcounts(world_size);
    std::vector<int> displs(world_size);
    
    size_t offset = 0;
    for (int i = 0; i < world_size; ++i) {
        sendcounts[i] = static_cast<int>(base_chunk_size + (i < remainder ? 1 : 0));
        displs[i] = static_cast<int>(offset);
        offset += sendcounts[i];
    }
    
    // Broadcast sendcounts to all processes
    MPI_Bcast(sendcounts.data(), world_size, MPI_INT, 0, MPI_COMM_WORLD);
    
    size_t local_size = sendcounts[world_rank];
    std::vector<int> local_arr(local_size);
    
    
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    
    int result = MPI_Scatterv(world_rank == 0 ? arr.data() : nullptr, sendcounts.data(), displs.data(), MPI_INT,
                              local_arr.data(), local_size, MPI_INT, 0, MPI_COMM_WORLD);
    if (result != MPI_SUCCESS) {
        char error_string[MPI_MAX_ERROR_STRING];
        int length_of_error_string;
        MPI_Error_string(result, error_string, &length_of_error_string);
        MPI_Abort(MPI_COMM_WORLD, result);
    }
    
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    
    mergeSort(local_arr, 0, local_size - 1);
    
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    std::vector<int> all_sizes(world_size);
    MPI_Allgather(&local_size, 1, MPI_INT, all_sizes.data(), 1, MPI_INT, MPI_COMM_WORLD);

    for (int step = 1; step < world_size; step *= 2) {
        if (world_rank % (2 * step) == 0) {
            if (world_rank + step < world_size) {
                CALI_MARK_BEGIN("comm");
                CALI_MARK_BEGIN("comm_large");
                
                int partner_size = all_sizes[world_rank + step];
                std::vector<int> received_arr(partner_size);
                
                MPI_Status status;
                MPI_Probe(world_rank + step, 0, MPI_COMM_WORLD, &status);
                
                int actual_size;
                MPI_Get_count(&status, MPI_INT, &actual_size);

                received_arr.resize(actual_size);
                
                result = MPI_Recv(received_arr.data(), actual_size, MPI_INT, world_rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                if (result != MPI_SUCCESS) {
                    char error_string[MPI_MAX_ERROR_STRING];
                    int length_of_error_string;
                    MPI_Error_string(result, error_string, &length_of_error_string);
                    log_message("MPI_Recv error: " + std::string(error_string), world_rank, __func__);
                    MPI_Abort(MPI_COMM_WORLD, result);
                }

                CALI_MARK_END("comm_large");
                CALI_MARK_END("comm");

                CALI_MARK_BEGIN("comp");
                CALI_MARK_BEGIN("comp_large");
                
                std::vector<int> merged_arr(local_arr.size() + received_arr.size());
                std::merge(local_arr.begin(), local_arr.end(), received_arr.begin(), received_arr.end(), merged_arr.begin());
                local_arr = std::move(merged_arr);
                
                CALI_MARK_END("comp_large");
                CALI_MARK_END("comp");
            }
        } else {
            int target = world_rank - step;
            if (target >= 0) {
                CALI_MARK_BEGIN("comm");
                CALI_MARK_BEGIN("comm_large");
                
                result = MPI_Send(local_arr.data(), local_arr.size(), MPI_INT, target, 0, MPI_COMM_WORLD);
                if (result != MPI_SUCCESS) {
                    char error_string[MPI_MAX_ERROR_STRING];
                    int length_of_error_string;
                    MPI_Error_string(result, error_string, &length_of_error_string);
                    log_message("MPI_Send error: " + std::string(error_string), world_rank, __func__);
                    MPI_Abort(MPI_COMM_WORLD, result);
                }
                
                CALI_MARK_END("comm_large");
                CALI_MARK_END("comm");
                break;
            }
        }
    }

    if (world_rank == 0) {
        arr = std::move(local_arr);
    }
}

bool correctness_check(const std::vector<int>& arr) {
    CALI_MARK_BEGIN("correctness_check");
    bool is_sorted = std::is_sorted(arr.begin(), arr.end());
    CALI_MARK_END("correctness_check");
    return is_sorted;
}

int main(int argc, char** argv) {
    CALI_MARK_BEGIN("main");

    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);


    if (argc != 3) {
        if (world_rank == 0) {
            std::cerr << "Usage: " << argv[0] << " <array_size> <input_type>" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    size_t input_size = std::atoi(argv[1]);
    std::string input_type = argv[2];
    
    adiak::init(NULL);
    adiak::launchdate();
    adiak::libraries();
    adiak::cmdline();
    adiak::clustername();
    adiak::value("algorithm", "merge");
    adiak::value("programming_model", "mpi");
    adiak::value("data_type", "int");
    adiak::value("size_of_data_type", sizeof(int));
    
    adiak::value("input_size", input_size);
    adiak::value("input_type", input_type);
    adiak::value("num_procs", world_size);
    adiak::value("scalability", "strong");
    adiak::value("group_num", 17);
    adiak::value("implementation_source", "handwritten");

    std::vector<int> arr;
    if (world_rank == 0) {
        data_init_runtime(arr, input_size, input_type);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    parallelMergeSort(arr, world_rank, world_size);

    if (world_rank == 0) {
        bool is_sorted = correctness_check(arr);
    }

    MPI_Finalize();
    CALI_MARK_END("main");

    return 0;
}