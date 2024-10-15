#include <mpi.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <random>
#include <cstdlib>
#include <caliper/cali.h>
#include <adiak.hpp>

void merge(const std::vector<int>& left, const std::vector<int>& right, std::vector<int>& result) {
    CALI_CXX_MARK_FUNCTION;
    result.clear();
    result.reserve(left.size() + right.size());

    auto it_left = left.begin();
    auto it_right = right.begin();

    while (it_left != left.end() && it_right != right.end()) {
        if (*it_left <= *it_right) {
            result.push_back(*it_left++);
        } else {
            result.push_back(*it_right++);
        }
    }

    result.insert(result.end(), it_left, left.end());
    result.insert(result.end(), it_right, right.end());
}

void mergeSort(std::vector<int>& arr, int left, int right) {
    CALI_CXX_MARK_FUNCTION;
    if (left < right) {
        int mid = left + (right - left) / 2;
        std::vector<int> left_half(arr.begin() + left, arr.begin() + mid + 1);
        std::vector<int> right_half(arr.begin() + mid + 1, arr.begin() + right + 1);
        
        mergeSort(left_half, 0, left_half.size() - 1);
        mergeSort(right_half, 0, right_half.size() - 1);
        
        std::vector<int> result(right - left + 1);
        merge(left_half, right_half, result);
        
        std::copy(result.begin(), result.end(), arr.begin() + left);
    }
}

void parallelMergeSort(std::vector<int>& arr, int input_size, int world_rank, int world_size) {
    CALI_CXX_MARK_FUNCTION;
    int chunk_size = input_size / world_size;
    std::vector<int> local_arr(chunk_size);

    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    int result = MPI_Scatter(world_rank == 0 ? arr.data() : nullptr, chunk_size, MPI_INT, local_arr.data(), chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    if (result != MPI_SUCCESS) {
        char error_string[MPI_MAX_ERROR_STRING];
        int length_of_error_string;
        MPI_Error_string(result, error_string, &length_of_error_string);
        std::cerr << "MPI error in Scatter: " << error_string << std::endl;
        MPI_Abort(MPI_COMM_WORLD, result);
    }
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    mergeSort(local_arr, 0, chunk_size - 1);
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    for (int step = 1; step < world_size; step *= 2) {
        if (world_rank % (2 * step) == 0) {
            if (world_rank + step < world_size) {
                std::vector<int> received_arr(chunk_size * step);
                
                CALI_MARK_BEGIN("comm");
                CALI_MARK_BEGIN("comm_large");
                MPI_Recv(received_arr.data(), chunk_size * step, MPI_INT, world_rank + step, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                CALI_MARK_END("comm_large");
                CALI_MARK_END("comm");

                std::vector<int> merged_arr(chunk_size * 2 * step);
                
                CALI_MARK_BEGIN("comp");
                CALI_MARK_BEGIN("comp_large");
                merge(local_arr, received_arr, merged_arr);
                local_arr = merged_arr;
                CALI_MARK_END("comp_large");
                CALI_MARK_END("comp");
            }
        } else {
            int target = world_rank - step;
            if (target >= 0) {
                CALI_MARK_BEGIN("comm");
                CALI_MARK_BEGIN("comm_large");
                MPI_Send(local_arr.data(), chunk_size * step, MPI_INT, target, 0, MPI_COMM_WORLD);
                CALI_MARK_END("comm_large");
                CALI_MARK_END("comm");
                break;
            }
        }
    }

    if (world_rank == 0) {
        arr.resize(input_size);
    }
    MPI_Gather(local_arr.data(), chunk_size, MPI_INT, arr.data(), chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
}

void data_init_runtime(std::vector<int>& arr, int input_size, const std::string& input_type) {
    CALI_CXX_MARK_FUNCTION;
    arr.resize(input_size);
    int num_unsorted = 0;
    if (input_type == "Sorted") {
    
        for (int i = 0; i < input_size; i++) {
            arr[i] = i;
        }
        
    } else if (input_type == "ReverseSorted") {
    
        std::iota(arr.rbegin(), arr.rend(), 0);
        
    } else if (input_type == "Random") {
        srand(42);
        
        for (int i = 0; i < input_size; i++) {
            arr[i] = rand() % input_size;
        }
    } else if (input_type == "1_perc_perturbed") {
    
        for (int i = 0; i < input_size; i++) {
            arr[i] = i;
        }
        num_unsorted = input_size / 100;
        for (int i = 0; i < num_unsorted; i++) {
            int idx1 = rand() % input_size;
            int idx2 = rand() % input_size;
            std::swap(arr[idx1], arr[idx2]);
        }
    }
}

bool correctness_check(const std::vector<int>& arr) {
    CALI_CXX_MARK_FUNCTION;
    printf("Success");
    return std::is_sorted(arr.begin(), arr.end());
}

int main(int argc, char** argv) {
    CALI_CXX_MARK_FUNCTION;

    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int input_size = 0;
    if (world_rank == 0) {
        //Check command line arguments
        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <array_size>" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        input_size = std::atoi(argv[1]);
    }

    MPI_Bcast(&input_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    //To collect metadata
    adiak::init(NULL);
    adiak::launchdate();
    adiak::libraries();
    adiak::cmdline();
    adiak::clustername();
    adiak::value("algorithm", "merge");
    adiak::value("programming_model", "mpi");
    adiak::value("data_type", "int");
    adiak::value("size_of_data_type", sizeof(int));

    //Set input type used for generating data
    std::string input_type = "Random";
    const int max_type_length = 20;
    char input_type_c[max_type_length];
    if (world_rank == 0) {
        strncpy(input_type_c, input_type.c_str(), max_type_length - 1);
        input_type_c[max_type_length - 1] = '\0';
    }
    
    MPI_Bcast(input_type_c, max_type_length, MPI_CHAR, 0, MPI_COMM_WORLD);
    
    if (world_rank != 0) {
        input_type = std::string(input_type_c);
    }

    adiak::value("input_size", input_size);
    adiak::value("input_type", input_type);
    adiak::value("num_procs", world_size);
    adiak::value("scalability", "strong");
    adiak::value("group_num", 17);
    adiak::value("implementation_source", "handwritten");
    
    //Create array based on input size and geenrate data
    std::vector<int> arr(input_size);
    if (world_rank == 0) {
        CALI_MARK_BEGIN("data_init_runtime");
        data_init_runtime(arr, input_size, input_type);
        CALI_MARK_END("data_init_runtime");
    }

    MPI_Barrier(MPI_COMM_WORLD);

    //Call parallel sort merge
    parallelMergeSort(arr, input_size, world_rank, world_size);

    //Check correctness of sorted array
    if (world_rank == 0) {
        CALI_MARK_BEGIN("correctness_check");
        bool is_sorted = correctness_check(arr);
        CALI_MARK_END("correctness_check");

        
    }
   
    MPI_Finalize();
    return 0;
}
