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
0.001 MPI_Comm_dup
0.000 MPI_Finalize
0.000 MPI_Finalized
0.000 MPI_Initialized
2.003 main
├─ 0.002 MPI_Barrier
├─ 0.021 MPI_Reduce
├─ 0.025 comm
│  └─ 0.025 comm_large
│     ├─ 0.006 MPI_Gather
│     └─ 0.024 MPI_Scatter
├─ 1.942 comp
│  └─ 1.942 comp_large
│     └─ 0.018 MPI_Sendrecv
├─ 0.015 correctness_check
└─ 0.018 data_init_runtime
```
- Sample Sort:
```
2.217 main
├─ 0.082 MPI_Comm_dup
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Finalized
├─ 0.000 MPI_Init
├─ 0.000 MPI_Initialized
└─ 0.526 main
   ├─ 0.398 comm
   │  ├─ 0.359 comm_large
   │  │  ├─ 0.284 MPI_Alltoall
   │  │  └─ 0.084 MPI_Scatterv
   │  └─ 0.040 comm_small
   │     ├─ 0.040 MPI_Bcast
   │     └─ 0.015 MPI_Gather
   ├─ 0.062 comp
   │  ├─ 0.060 comp_large
   │  └─ 0.011 comp_small
   ├─ 0.106 correctness_check
   │  ├─ 0.017 MPI_Gather
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

# Merge Sort - Performance Evaluation

## Data Set Size: 268,435,456

<div style="display: flex;">
  <img src="/merge_sort/mergesort_performance_eval_graphs/comp_large_268435456_performance.png" alt="Performance Metrics 268435456 - comp_large" style="width: 45%; margin-right: 10px;">
  <img src="/merge_sort/mergesort_performance_eval_graphs/comm_268435456_performance.png" alt="Performance Metrics 268435456 - comm" style="width: 45%;">
</div>

## Data Set Size: 4,194,304

<div style="display: flex;">
  <img src="/merge_sort/mergesort_performance_eval_graphs/comp_large_4194304_performance.png" alt="Performance Metrics 4194304 - comp_large" style="width: 45%; margin-right: 10px;">
  <img src="/merge_sort/mergesort_performance_eval_graphs/comm_4194304_performance.png" alt="Performance Metrics 4194304 - comm" style="width: 45%;">
</div>

## Data Set Size: 65,536

<div style="display: flex;">
  <img src="/merge_sort/mergesort_performance_eval_graphs/comp_large_65536_performance.png" alt="Performance Metrics 65536 - comp_large" style="width: 45%; margin-right: 10px;">
  <img src="/merge_sort/mergesort_performance_eval_graphs/comm_65536_performance.png" alt="Performance Metrics 65536 - comm" style="width: 45%;">
</div>

```The plots that are displayed above show different computational and communication performance metrics for input sizes of 2^16, 2^22, and 2^28. To begin by explaining the graphs, there are 5 plots covering the Min time/rank, Max time/rank, Avg time/rank, Total time, and Variance time/rank. In addition to this, for each of these metrics, it was tested on 4 different types of input. These include random, sorted, reverse sorted and sorted with 1% perturbed. Now to dig into the trends and analysis of these different graphs. For the first graph which is of size 2^28, for communication, it can be seen that as the number of processes increases there is a steady increase in the total time as well. This repeats for each of the 5 metrics in communication. In contrast, for the computation side, it can be seen that as the number of processes increases, the time also increases. It is important to note that geenrally it appears that random input tends to take more time in general. For the comp region it can be seen that avg time/rank deceases rapidly at first but then it stabilizes.```

# Radix Sort - Performance Evaluation

## Data Set Size: 268,435,456

<div style="display: flex;">
  <img src="/radix_sort/radixsort_performance_eval_graphs/performance_metrics_268435456_comp_large.png" alt="Performance Metrics 268435456 - comp_large" style="width: 45%; margin-right: 10px;">
  <img src="/radix_sort/radixsort_performance_eval_graphs/performance_metrics_268435456_comm.png" alt="Performance Metrics 268435456 - comm" style="width: 45%;">
</div>

## Data Set Size: 4,194,304

<div style="display: flex;">
  <img src="/radix_sort/radixsort_performance_eval_graphs/performance_metrics_4194304_comp_large.png" alt="Performance Metrics 4194304 - comp_large" style="width: 45%; margin-right: 10px;">
  <img src="/radix_sort/radixsort_performance_eval_graphs/performance_metrics_4194304_comm.png" alt="Performance Metrics 4194304 - comm" style="width: 45%;">
</div>

## Data Set Size: 65,536

<div style="display: flex;">
  <img src="/radix_sort/radixsort_performance_eval_graphs/performance_metrics_65536_comp_large.png" alt="Performance Metrics 65536 - comp_large" style="width: 45%; margin-right: 10px;">
  <img src="/radix_sort/radixsort_performance_eval_graphs/performance_metrics_65536_comm.png" alt="Performance Metrics 65536 - comm" style="width: 45%;">
</div>

```The generated plots provide insights into the performance metrics of different computational tasks across input sizes of 2^16, 2^22, and 2^28 and various input types (you can view all the graphs generated in the radixsort_performance_eval_graphs folder). For each input size, we observe how different input types (Random, Sorted, Reverse Sorted, and Sorted with 1% perturbed) affect key metrics such as Min time/rank, Max time/rank, Avg time/rank, Total time, and Variance time/rank. In general, as the number of processes increases, the average time per rank decreases, this shows a strong scalability for larger workloads. The total execution time also shows a reduction with increasing numbers of processes, this shows that parallelization is effective. But there are variations between input types. For example, the Sorted input tends to perform better in terms of consistency, as indicated by lower variance, while the Random and Reverse Sorted inputs often show higher fluctuations in time metrics. The plots also highlight the diminishing returns in performance gains beyond a certain number of processes, especially for smaller input sizes.```

# Bitonic Sort - Performance Evaluation

## Data Set Size: 268,435,456

<div style="display: flex;">
  <img src="/BitonicSort/performance/performance_metrics_268435456_comp.png" alt="Performance Metrics 268435456 - comp_large" style="width: 45%; margin-right: 10px;">
  <img src="/BitonicSort/performance/performance_metrics_268435456_comm.png" alt="Performance Metrics 268435456 - comm" style="width: 45%;">
</div>

## Data Set Size: 4,194,304

<div style="display: flex;">
  <img src="/BitonicSort/performance/performance_metrics_4194304_comp.png" alt="Performance Metrics 4194304 - comp_large" style="width: 45%; margin-right: 10px;">
  <img src="/BitonicSort/performance/performance_metrics_4194304_comm.png" alt="Performance Metrics 4194304 - comm" style="width: 45%;">
</div>

## Data Set Size: 65,536

<div style="display: flex;">
  <img src="/BitonicSort/performance/performance_metrics_65536_comp.png" alt="Performance Metrics 65536 - comp_large" style="width: 45%; margin-right: 10px;">
  <img src="/BitonicSort/performance/performance_metrics_65536_comm.png" alt="Performance Metrics 65536 - comm" style="width: 45%;">
</div>

```I couldn't produce all the .cali files because the Grace cluster was not accepting jobs and the tasks remained stuck in the queue. When I initially ran the jobs before Grace became busy, the 1% permuted version overwrote other input types such as random, reverse, and sorted. I plan to finish generating the .cali files and complete the graphs once the situation with Grace improves. The plots displayed above show various computational and communication performance metrics for input sizes of 2^16, 2^22, and 2^28. There are 5 plots representing the Min time/rank, Max time/rank, Avg time/rank, Total time, and Variance time/rank. Each of these metrics was tested across four different types of input: random, sorted, reverse sorted, and sorted with 1% perturbation (it was tested but no califile was captured). Now, looking at the trends in these graphs, there is a consistent pattern: as the number of processes increases, the total time also steadily increases. On the computation side, the time taken also rises as the number of processes increases.```
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
