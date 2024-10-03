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

- Bitonic Sort: Jesung Ha
- Sample Sort (Aayush Garg) : a sorting algorithm that is a divide and conquer algorithm by paritioning the array into sub-intervals or buckets. In parallel, the buckets are then sorted individually across multiple processors and then concatenated together.
- Merge Sort: Jeffrey Slobodkin
- Radix Sort (Akshay Belhe) : works by sorting numbers digit by digit, starting from the least significant digit. In parallel Radix Sort, the work of sorting each digit is split across multiple processors.

### 2b. Pseudocode for each parallel algorithm
- For MPI programs, include MPI calls you will use to coordinate between processes

- Bitonic Sort:
```
(put here)
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
(put here)
```
- Radix Sort:
```
function radixSort(input_array, num_processors)
    max_value = find_max(input_array)

    for each digit in max_value from ones place to most significant:
        count_array = initialize_array()

        // divide the input_array among processors
        local_array = **MPI_Scatter**(input_array, num_processors)

        // perform counting based on current digit
        local_count = count_digit_occurrences(local_array, digit)

        // gather the local counts from each processor
        global_count = **MPI_Gather**(local_count, root=0)

        if processor_rank == 0:
            // Compute prefix sum of counts
            prefix_sum = compute_prefix_sum(global_count)
        
        // send prefix_sum to all processors
        prefix_sum = **MPI_Bcast**(prefix_sum, root=0)

        // sort digit based on prefix sum
        sorted_local_array = redistribute_elements(local_array, prefix_sum)

        // gather the sorted local arrays from all processors
        sorted_array = gather(sorted_local_array, root=0)

        if processor_rank == 0:
            // Update the input array for the next digit
            input_array = sorted_array

    return sorted_array
```

- MPI Calls:
  * (put here)

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
