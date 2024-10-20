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