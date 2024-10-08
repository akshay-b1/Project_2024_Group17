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
