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
