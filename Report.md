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
(put here)
```
- Merge Sort:
```
(put here)
```
- Radix Sort:
```
(put here)
```

- MPI Calls:
  * (put here)

### 2c. Evaluation plan - what and how will you measure and compare
- Input sizes, Input types
  * Input sizes
    - (put here)
  * Input types
    - (put here)
- Strong scaling (same problem size, increase number of processors/nodes)
  * (put here)
- Weak scaling (increase problem size, increase number of processors)
  * (put here)
- Versions to Compare: communication strategies (collectives vs. point-to-point) and parallelization strategies (master/worker vs. SPMD)
