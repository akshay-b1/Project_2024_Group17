# CSCE 435 Group project

## 0. Group number: 17

## 1. Group members:
1. Jeffrey Slobodkin
2. Aayush Garg
3. Akshay Belhe
4. Jesung Ha

We will communicate via iMessage and Discord

## 2. Project topic (e.g., parallel sorting algorithms)

### 2a. Brief project description (what algorithms will you be comparing and on what architectures)

- Bitonic Sort: (Jesung Ha) : Parallel sorting algorithm that splits sequence into a bitonic sequence and perform bitonic merge to sort the sequence. Bitonic sequence is a sequence that monotonically increase and then decrease. 
- Sample Sort (Aayush Garg) : a sorting algorithm that is a divide and conquer algorithm by paritioning the array into sub-intervals or buckets. In parallel, the buckets are then sorted individually across multiple processors and then concatenated together.
- Merge Sort: (Jeffrey Slobodkin): a comparison based sorting algorithm that uses divide and conquer by splitting the array many times until each has only one item. In parallel, they are sorted by merging the sublits across multiple processors into one list.
- Radix Sort (Akshay Belhe) : works by sorting numbers digit by digit, starting from the least significant digit. In parallel Radix Sort, the work of sorting each digit is split across multiple processors.

### 2b. Pseudocode for each parallel algorithm
- For MPI programs, include MPI calls you will use to coordinate between processes

- Bitonic Sort:
```
function comp_swap(a, i ,j, direction):
    //swap based on direction
    if (direction == 1 and a[i]>a[j]) or (direction == 0 and a[j] > a[i]):
      swap(a[i], a[j])


function bitonic_merge(a, low, count, dir):
    if count > 1:
      int k = count / 2
      //compare and swap elements
      for (i = low, i < low +k, i++){
        comp_swap(a, i, i+ k , dir)
      }

      //recursively merge
      bitonic_merge(arr, low, k, dir)
      bitonic_merge(arr, low+k, k, dir)


function bitonic_sort(a, low, count, dir):
    if (count > 1):
        int k = count / 2

        // Sort first half in ascending order (dir = 1)
        bitonic_sort(a, low, k, 1)

        // Sort second half in descending order (dir = 0)
        bitonic_sort(a, low + k, k, 0)

        // Merge based on direction
        bitonic_merge(a, low, count, dir)


src = https://www.geeksforgeeks.org/bitonic-sort/#

```
- Sample Sort:
```
function sampleSort(A[1..n], k, p)
    // if average bucket size is below a threshold switch to e.g. quicksort
    if n / k < threshold then smallSort(A) 
    /* Step 1 */
    select S = [S1, ..., S(p−1)k] randomly from // select samples
    sort S // sort sample
    [s0, s1, ..., sp−1, sp] <- [-∞, Sk, S2k, ..., S(p−1)k, ∞] // select splitters
    /* Step 2 */
    for each a in A
        find j such that sj−1 < a <= sj
        place a in bucket bj
    /* Step 3 and concatenation */
    return concatenate(sampleSort(b1), ..., sampleSort(bk))

src: https://en.wikipedia.org/wiki/Samplesort
```
- Merge Sort:
```
function parallelMergesort(A, lo, hi, B, off):
    len := hi - lo + 1               
    if len == 1 then
        B[off] := A[lo]                
    else:
        T := new array of length len   
        mid := floor((lo + hi) / 2)    
        mid' := mid - lo + 1           
        fork parallelMergesort(A, lo, mid, T, 1)
        parallelMergesort(A, mid + 1, hi, T, mid' + 1) 
        join                             
        parallelMerge(T, 1, mid', mid' + 1, len, B, off)

src: https://en.wikipedia.org/wiki/Merge_sort
```
- Radix Sort:
```
function parallel_radix_sort(input_array, num_processors)
    max_value = find_max(input_array)
    
    for digit in each digit of(max_value):
        local_array = mpi_scatter(input_array, num_processors)
        local_count = count_digits(local_array, digit)
        global_count = mpi_gather(local_count, root=0)

        if rank == 0:
            prefix_sum = compute_prefix_sum(global_count)

        prefix_sum = mpi_broadcast(prefix_sum, root=0)
        sorted_local_array = redistribute(local_array, prefix_sum)
        sorted_array = gather(sorted_local_array, root=0)

        if rank == 0:
            input_array = sorted_array

    return sorted_array
```

- MPI Calls:
  - MPI_Scatter()
  - MPI_Gather()
  - MPI_Bcast()

### 2c. Evaluation plan - what and how will you measure and compare
- Input sizes, Input types
  * Input sizes
    - 10^3, 10^5, 10^7, 10^9
  * Input types
    - Sorted, Sorted with 1% not sorted, Reverse sorted, Random
- Strong scaling (same problem size, increase number of processors/nodes)
  - Problem Size: 10^7
  - Number of Processors: 1, 2, 4, 8, 16, 32
 
- Weak scaling (increase problem size, increase number of processors)
  * Problem Size: 10^7 in different intervals scaled to match rate at which processors are increasing
  * Increase number of processors and problem size
      * (1 * 10^7, 2), (2 * 10^7, 4), (4 * 10^7, 8), (8 * 10^7, 16) 

 - We will be measuring:
    - Execution Time: For each run, we will record the total execution time taken to sort the array.
    - Speedup: 
      $$\frac{\text{Time with 1 processor}}{\text{Time with N processors}}$$
    - Efficiency: 
      $$\frac{\text{Speedup}}{\text{Number of processors}}$$
      
- Versions to Compare: communication strategies (collectives vs. point-to-point) and parallelization strategies (master/worker vs. SPMD)

### 3a. Caliper instrumentation
Please use the caliper build `/scratch/group/csce435-f24/Caliper/caliper/share/cmake/caliper` 
(same as lab2 build.sh) to collect caliper files for each experiment you run.

Your Caliper annotations should result in the following calltree
(use `Thicket.tree()` to see the calltree):
```
main
|_ data_init_X      # X = runtime OR io
|_ comm
|    |_ comm_small
|    |_ comm_large
|_ comp
|    |_ comp_small
|    |_ comp_large
|_ correctness_check
```

Required region annotations:
- `main` - top-level main function.
    - `data_init_X` - the function where input data is generated or read in from file. Use *data_init_runtime* if you are generating the data during the program, and *data_init_io* if you are reading the data from a file.
    - `correctness_check` - function for checking the correctness of the algorithm output (e.g., checking if the resulting data is sorted).
    - `comm` - All communication-related functions in your algorithm should be nested under the `comm` region.
      - Inside the `comm` region, you should create regions to indicate how much data you are communicating (i.e., `comm_small` if you are sending or broadcasting a few values, `comm_large` if you are sending all of your local values).
      - Notice that auxillary functions like MPI_init are not under here.
    - `comp` - All computation functions within your algorithm should be nested under the `comp` region.
      - Inside the `comp` region, you should create regions to indicate how much data you are computing on (i.e., `comp_small` if you are sorting a few values like the splitters, `comp_large` if you are sorting values in the array).
      - Notice that auxillary functions like data_init are not under here.
    - `MPI_X` - You will also see MPI regions in the calltree if using the appropriate MPI profiling configuration (see **Builds/**). Examples shown below.

All functions will be called from `main` and most will be grouped under either `comm` or `comp` regions, representing communication and computation, respectively. You should be timing as many significant functions in your code as possible. **Do not** time print statements or other insignificant operations that may skew the performance measurements.

### **Nesting Code Regions Example** - all computation code regions should be nested in the "comp" parent code region as following:
```
CALI_MARK_BEGIN("comp");
CALI_MARK_BEGIN("comp_small");
sort_pivots(pivot_arr);
CALI_MARK_END("comp_small");
CALI_MARK_END("comp");

# Other non-computation code
...

CALI_MARK_BEGIN("comp");
CALI_MARK_BEGIN("comp_large");
sort_values(arr);
CALI_MARK_END("comp_large");
CALI_MARK_END("comp");
```

### **Calltree Example**:
```
# MPI Mergesort
4.695 main
├─ 0.001 MPI_Comm_dup
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Finalized
├─ 0.000 MPI_Init
├─ 0.000 MPI_Initialized
├─ 2.599 comm
│  ├─ 2.572 MPI_Barrier
│  └─ 0.027 comm_large
│     ├─ 0.011 MPI_Gather
│     └─ 0.016 MPI_Scatter
├─ 0.910 comp
│  └─ 0.909 comp_large
├─ 0.201 data_init_runtime
└─ 0.440 correctness_check
```

### 3b. Collect Metadata

Have the following code in your programs to collect metadata:
```
adiak::init(NULL);
adiak::launchdate();    // launch date of the job
adiak::libraries();     // Libraries used
adiak::cmdline();       // Command line used to launch the job
adiak::clustername();   // Name of the cluster
adiak::value("algorithm", algorithm); // The name of the algorithm you are using (e.g., "merge", "bitonic")
adiak::value("programming_model", programming_model); // e.g. "mpi"
adiak::value("data_type", data_type); // The datatype of input elements (e.g., double, int, float)
adiak::value("size_of_data_type", size_of_data_type); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
adiak::value("input_size", input_size); // The number of elements in input dataset (1000)
adiak::value("input_type", input_type); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
adiak::value("num_procs", num_procs); // The number of processors (MPI ranks)
adiak::value("scalability", scalability); // The scalability of your algorithm. choices: ("strong", "weak")
adiak::value("group_num", group_number); // The number of your group (integer, e.g., 1, 10)
adiak::value("implementation_source", implementation_source); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").
```

They will show up in the `Thicket.metadata` if the caliper file is read into Thicket.

- Bitonic Sort:
```
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

    for (int step = 2; step <= world_size * local_size; step *= 2) {
    for (int substep = step / 2; substep > 0; substep /= 2) {
        for (int i = 0; i < local_size; i++) {
            int j = i ^ substep;
            int proc = j / local_size;

            if (proc == world_rank) {
                // Bitonic compare within the process
                bitonicCompare(local_arr, i % local_size, j % local_size, (i / step) % 2 == 0);
            } else {
                // Communication between processes
                int partner_rank = world_rank ^ (substep / local_size);
                int send_val = local_arr[i];
                int recv_val;

                MPI_Sendrecv(&send_val, 1, MPI_INT, partner_rank, 0,
                             &recv_val, 1, MPI_INT, partner_rank, 0,
                             MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                // Perform the comparison based on the step's direction
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
        for(int i = 0; i < arr.size(); i++){
            std::cout << std::to_string(arr[i]);
        }
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
```
- Sample Sort:
```
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

#define MASTER 0
#define MAX_PRINT_SIZE 100

void printArray(const char* name, int* arr, int size, int rank) {
    printf("[Rank %d] %s (size=%d): ", rank, name, size);
    int print_size = (size < MAX_PRINT_SIZE) ? size : MAX_PRINT_SIZE;
    for (int i = 0; i < print_size; i++) {
        printf("%d ", arr[i]);
    }
    if (size > MAX_PRINT_SIZE) printf("...");
    printf("\n");
}

int compare(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

int main (int argc, char *argv[])
{
    CALI_CXX_MARK_FUNCTION;
        
    int sizeOfArray;
    int inputType;    
    if (argc == 3)
    {
        sizeOfArray = atoi(argv[1]);
        inputType = atoi(argv[2]);        
    }
    else
    {
        printf("\n Please provide the size of the array and input type (0-3)");
        return 0;
    }

    int numtasks, taskid, *localArray, *globalArray = NULL;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);

    if (taskid == MASTER) {
        printf("\n=== Starting MPI Sample Sort with %d processes ===\n", numtasks);
        printf("Input size: %d\n", sizeOfArray);
    }

    cali::ConfigManager mgr;
    mgr.start();

    adiak::init(NULL);
    adiak::user();
    adiak::launchdate();
    adiak::libraries();
    adiak::cmdline();
    adiak::clustername();
    adiak::value("algorithm", "sample_sort");
    adiak::value("programming_model", "mpi");
    adiak::value("data_type", "int");
    adiak::value("size_of_data_type", sizeof(int));
    adiak::value("input_size", sizeOfArray);
    adiak::value("num_procs", numtasks);
    adiak::value("scalability", "strong");
    adiak::value("implementation_source", "handwritten");
    
    switch(inputType) {
        case 0:
            adiak::value("input_type", "Random");
            break;
        case 1:
            adiak::value("input_type", "Sorted");
            break;
        case 2:
            adiak::value("input_type", "Reverse Sorted");
            break;
        case 3:
            adiak::value("input_type", "Sorted with 1% perturbed");
            break;
        default:
            adiak::value("input_type", "Unknown");
    }        
        
    CALI_MARK_BEGIN("main");
    int localSize = sizeOfArray / numtasks;
    int remainder = sizeOfArray % numtasks;
    if (taskid < remainder) {
        localSize++;
    }

    if (taskid == MASTER) {
        printf("\nLocal size distribution:\n");
        for (int i = 0; i < numtasks; i++) {
            int size = sizeOfArray / numtasks + (i < remainder ? 1 : 0);
            printf("Rank %d: %d elements\n", i, size);
        }
    }

    localArray = (int*)malloc(localSize * sizeof(int));

    // data of array generation --> sorted, 1%purpuated, random, reverse sorted
    CALI_MARK_BEGIN("data_init_runtime");
    if (taskid == MASTER) {
        globalArray = (int*)malloc(sizeOfArray * sizeof(int));
        switch(inputType) {
            case 0: // Random
                for (int i = 0; i < sizeOfArray; i++) {
                    globalArray[i] = rand() % sizeOfArray;
                }
                break;
            case 1: // Sorted
                for (int i = 0; i < sizeOfArray; i++) {
                    globalArray[i] = i;
                }
                break;
            case 2: // Reverse Sorted
                for (int i = 0; i < sizeOfArray; i++) {
                    globalArray[i] = sizeOfArray - i - 1;
                }
                break;
            case 3: // Sorted with 1% perturbed
                for (int i = 0; i < sizeOfArray; i++) {
                    globalArray[i] = i;
                }
                int perturbCount = sizeOfArray / 100;
                for (int i = 0; i < perturbCount; i++) {
                    int idx = rand() % sizeOfArray;
                    globalArray[idx] = rand() % sizeOfArray;
                }
                break;
        }
        printArray("Initial global array", globalArray, sizeOfArray, MASTER);
    }
    CALI_MARK_END("data_init_runtime");

    // distribute data to ranks
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    int* sendcounts = (int*)malloc(numtasks * sizeof(int));
    int* displacements = (int*)malloc(numtasks * sizeof(int));
    
    int displacement = 0;
    for (int i = 0; i < numtasks; i++) {
        sendcounts[i] = sizeOfArray / numtasks;
        if (i < remainder) {
            sendcounts[i]++;
        }
        displacements[i] = displacement;
        displacement += sendcounts[i];
    }

    MPI_Scatterv(globalArray, sendcounts, displacements, MPI_INT, 
                 localArray, sendcounts[taskid], MPI_INT, 
                 MASTER, MPI_COMM_WORLD);
    printf("[Rank %d] Received local array\n", taskid);
    printArray("Local array before sorting", localArray, localSize, taskid);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");\

    // sort locally
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    qsort(localArray, localSize, sizeof(int), compare);
    printf("[Rank %d] Local array sorted\n", taskid);
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    // chose splitters
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");
    int* localSplitters = (int*)malloc((numtasks - 1) * sizeof(int));
    for (int i = 0; i < (numtasks - 1) ; i++) {
        int tempI = (localSize * (i + 1)) / (numtasks);
        localSplitters[i] = localArray[tempI];
    }
    printf("[Rank %d] Selected local splitters\n", taskid);
    printArray("Local splitters", localSplitters, numtasks - 1, taskid);
    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");

    // combine all local splitters
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");
    int* allSplitters = NULL;
    if (taskid == MASTER) {
        allSplitters = (int*)malloc(numtasks * (numtasks - 1) * sizeof(int));
    }
    MPI_Gather(localSplitters, (numtasks - 1), MPI_INT, 
               allSplitters, (numtasks - 1), MPI_INT, 
               MASTER, MPI_COMM_WORLD);
    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");

    // chose global splitters
    int* globalSplitters = NULL;
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");
    if (taskid == MASTER) {
        printf("\n[Master] Gathered all splitters\n");
        printArray("All gathered splitters", allSplitters, numtasks * (numtasks - 1), MASTER);
        
        qsort(allSplitters, numtasks * (numtasks - 1), sizeof(int), compare);
        globalSplitters = (int*)malloc((numtasks - 1) * sizeof(int));
        for (int i = 0; i < numtasks - 1; i++) {
            globalSplitters[i] = allSplitters[(i + 1) * (numtasks - 1)];
        }

        printf("[Master] Selected global splitters\n");
        printArray("Global splitters", globalSplitters, numtasks - 1, MASTER);
    } else {
        globalSplitters = (int*)malloc((numtasks - 1) * sizeof(int));
    }
    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");

     // send all global splitters
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_small");
    MPI_Bcast(globalSplitters, numtasks - 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    CALI_MARK_END("comm_small");
    CALI_MARK_END("comm");

    // parition the data
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    int bucketSize = localSize + 1;
    int* buckets = (int*)malloc(sizeof(int) * numtasks * bucketSize);
    memset(buckets, 0, sizeof(int) * numtasks * bucketSize);
    
    int j = 0;
    int k = 1;
    for (int i = 0; i < localSize; i++) {
        if (j < numtasks - 1) {
            if (localArray[i] < globalSplitters[j]) {
                buckets[bucketSize * j + k++] = localArray[i];
            } else {
                buckets[bucketSize * j] = k - 1;  // store count at start of bucket
                k = 1;
                j++;
                i--;  // reprocess current element
            }
        } else {
            buckets[bucketSize * j + k++] = localArray[i];
        }
    }
    buckets[bucketSize * j] = k - 1;  // store count for last bucket

    printf("[Rank %d] Created buckets\n", taskid);
    for (int i = 0; i < numtasks; i++) {
        printf("[Rank %d] Bucket %d size: %d\n", taskid, i, buckets[bucketSize * i]);
    }
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    // all to all communication
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    int* bucketBuffer = (int*)malloc(sizeof(int) * numtasks * bucketSize);
    MPI_Alltoall(buckets, bucketSize, MPI_INT, 
                 bucketBuffer, bucketSize, MPI_INT, 
                 MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");

     // redistribution
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    int* localBucket = (int*)malloc(sizeof(int) * 2 * localSize);
    int count = 1;
    
    for (j = 0; j < numtasks; j++) {
        k = 1;
        for (int i = 0; i < bucketBuffer[bucketSize * j]; i++) {
            localBucket[count++] = bucketBuffer[bucketSize * j + k++];
        }
    }
    localBucket[0] = count - 1;

    // local sort again
    printf("[Rank %d] Combined local bucket size: %d\n", taskid, localBucket[0]);
    printArray("Combined local bucket", &localBucket[1], localBucket[0], taskid);

    qsort(&localBucket[1], localBucket[0], sizeof(int), compare);
    printf("[Rank %d] Final sorted local portion\n", taskid);
    printArray("Sorted local portion", &localBucket[1], localBucket[0], taskid);
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");

    // gather results
    CALI_MARK_BEGIN("correctness_check");
    int* finalArray = NULL;
    int* allCounts = NULL;
    if (taskid == MASTER) {
        allCounts = (int*)malloc(numtasks * sizeof(int));
    }

    MPI_Gather(&localBucket[0], 1, MPI_INT, 
               allCounts, 1, MPI_INT, 
               MASTER, MPI_COMM_WORLD);

    if (taskid == MASTER) {
        int totalSize = 0;
        for (int i = 0; i < numtasks; i++) {
             printf("Rank %d: %d elements\n", i, allCounts[i]);
            totalSize += allCounts[i];
        }
        finalArray = (int*)malloc(totalSize * sizeof(int));
        
        int* gatherDisplacements = (int*)malloc(numtasks * sizeof(int));
        gatherDisplacements[0] = 0;
        for (int i = 1; i < numtasks; i++) {
            gatherDisplacements[i] = gatherDisplacements[i-1] + allCounts[i-1];
        }

        MPI_Gatherv(&localBucket[1], localBucket[0], MPI_INT,
                    finalArray, allCounts, gatherDisplacements, MPI_INT,
                    MASTER, MPI_COMM_WORLD);
        
        printf("\n[Master] Final gathered array\n");
        printArray("Final sorted array", finalArray, totalSize, MASTER);

        int isSorted = 1;
        for (int i = 1; i < totalSize; i++) {
            if (finalArray[i] < finalArray[i-1]) {
                isSorted = 0;
                break;
            }
        }
        printf("Array is %s\n", isSorted ? "correctly sorted" : "not sorted");
        
        free(gatherDisplacements);
    } else {
        MPI_Gatherv(&localBucket[1], localBucket[0], MPI_INT,
                    NULL, NULL, NULL, MPI_INT,
                    MASTER, MPI_COMM_WORLD);
    }
    CALI_MARK_END("correctness_check");
    CALI_MARK_END("main");

    free(localArray);
    free(localSplitters);
    free(globalSplitters);
    free(buckets);
    free(bucketBuffer);
    free(localBucket);
    free(sendcounts);
    free(displacements);
    if (taskid == MASTER) {
        free(globalArray);
        free(allSplitters);
        free(allCounts);
        free(finalArray);
    }

    mgr.stop();
    mgr.flush();
    MPI_Finalize();
    return 0;
}
```
- Merge Sort:
```
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
    MPI_Scatter(world_rank == 0 ? arr.data() : nullptr, chunk_size, MPI_INT, local_arr.data(), chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
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
```
- Radix Sort:
```
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
```

### **See the `Builds/` directory to find the correct Caliper configurations to get the performance metrics.** They will show up in the `Thicket.dataframe` when the Caliper file is read into Thicket.
## 4. Performance evaluation

Include detailed analysis of computation performance, communication performance. 
Include figures and explanation of your analysis.

### 4a. Vary the following parameters
For input_size's:
- 2^16, 2^18, 2^20, 2^22, 2^24, 2^26, 2^28

For input_type's:
- Sorted, Random, Reverse sorted, 1%perturbed

MPI: num_procs:
- 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024

This should result in 4x7x10=280 Caliper files for your MPI experiments.

### 4b. Hints for performance analysis

To automate running a set of experiments, parameterize your program.

- input_type: "Sorted" could generate a sorted input to pass into your algorithms
- algorithm: You can have a switch statement that calls the different algorithms and sets the Adiak variables accordingly
- num_procs: How many MPI ranks you are using

When your program works with these parameters, you can write a shell script 
that will run a for loop over the parameters above (e.g., on 64 processors, 
perform runs that invoke algorithm2 for Sorted, ReverseSorted, and Random data).  

### 4c. You should measure the following performance metrics
- `Time`
    - Min time/rank
    - Max time/rank
    - Avg time/rank
    - Total time
    - Variance time/rank

# Sample Sort - Performance Evaluation

## Data Set Size: 268,435,456

<div style="display: flex;">
  <img src="/samplesort/samplesort_performance_eval_graphs/performance_metrics_268435456_comp.png" alt="Performance Metrics 268435456 - comp" style="width: 45%; margin-right: 10px;">
  <img src="/samplesort/samplesort_performance_eval_graphs/performance_metrics_268435456_comm.png" alt="Performance Metrics 268435456 - comm" style="width: 45%;">
</div>

## Data Set Size: 4,194,304

<div style="display: flex;">
  <img src="/samplesort/samplesort_performance_eval_graphs/performance_metrics_4194304_comp.png" alt="Performance Metrics 4194304 - comp" style="width: 45%; margin-right: 10px;">
  <img src="/samplesort/samplesort_performance_eval_graphs/performance_metrics_4194304_comm.png" alt="Performance Metrics 4194304 - comm" style="width: 45%;">
</div>

## Data Set Size: 65,536

<div style="display: flex;">
  <img src="/samplesort/samplesort_performance_eval_graphs/performance_metrics_65536_comp.png" alt="Performance Metrics 65536 - comp" style="width: 45%; margin-right: 10px;">
  <img src="/samplesort/samplesort_performance_eval_graphs/performance_metrics_65536_comm.png" alt="Performance Metrics 65536 - comm" style="width: 45%;">
</div>

```Above we can see the computation and communication performance for input sizes of 2^16, 2^22, and 2^28 (you can view all the graphs generated in the samplesortgraphs folder). One trend we can analyze amongst all of these is that as we increase the input size, we see an increase in the total time over both computation and communication. We also see an overall decline in the average communication and computation time as we increase the number of processes. It is important to note that on the smallest input size, 2^16, it seems that there is a point when increasing the number of processors does drop the total time significantly, but then it keeps increasing. It can also be seen that as the input size increases, the randomly generated input (as seen on the communication side) takes more time on average comapred to either sorted, reverse sorted, or sorted 1% perturbed. Because we are observing the the min, max, average time per rank, we can see that as the number of processes increases, on average the time for computation per each one is definitely decreasing. The reason we see an increase in the total time is because we are looking at the total time as an overall unit compared to per each rank as I said before. So instead if we observe the average time per rank for both communication and computation we can see that as more processors are introduced, more overhead is introduced in terms of communication, which explains the increase in average time/rank for that, but in terms of computation, the average time per processor is definitely decreasing.```
## 5. Presentation
Plots for the presentation should be as follows:
- For each implementation:
    - For each of comp_large, comm, and main:
        - Strong scaling plots for each input_size with lines for input_type (7 plots - 4 lines each)
        - Strong scaling speedup plot for each input_type (4 plots)
        - Weak scaling plots for each input_type (4 plots)

Analyze these plots and choose a subset to present and explain in your presentation.

## 6. Final Report
Submit a zip named `TeamX.zip` where `X` is your team number. The zip should contain the following files:
- Algorithms: Directory of source code of your algorithms.
- Data: All `.cali` files used to generate the plots seperated by algorithm/implementation.
- Jupyter notebook: The Jupyter notebook(s) used to generate the plots for the report.
- Report.md
