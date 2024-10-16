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
    if (argc == 2)
    {
        sizeOfArray = atoi(argv[1]);
    }
    else
    {
        printf("\n Please provide the size of the array");
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
    adiak::value("input_type", "Random");
    adiak::value("num_procs", numtasks);
    adiak::value("scalability", "strong");
    adiak::value("implementation_source", "handwritten");

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
        for (int i = 0; i < sizeOfArray; i++) {
            globalArray[i] = rand() % sizeOfArray;
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

```
- Radix Sort:
```

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
