#!/bin/bash

# This script will automate running radixsort experiments with different input sizes, types, and number of processors.
# Based on mpi.grace_job and SLURM scheduler settings.

# Define input sizes and types
declare -a input_sizes=(65536 262144 1048576 4194304 16777216 67108864 268435456)  # 2^16, 2^18, 2^20, 2^22, 2^24, 2^26, 2^28
declare -a input_types=(0 1 2 3)  # 0: Sorted, 1: 1% Not Sorted, 2: Reverse Sorted, 3: Random
declare -a num_procs=(512 1024)  # Number of processors (limited to avoid resource issues)

# Iterate through each combination of input size, input type, and number of processors
for input_size in "${input_sizes[@]}"; do
    for input_type in "${input_types[@]}"; do
        for procs in "${num_procs[@]}"; do
        
            if [ $procs -le 32 ]; then
                nodes=1
            else
                nodes=$((procs / 32))
            fi
            
            # Create a unique job name for each combination
            job_name="radixsort_${input_size}_${input_type}_${procs}"

            # Submit the job using sbatch with customized parameters
            sbatch --job-name="$job_name" \
                   --time=00:15:00 \
                   --nodes=$nodes \
                   --ntasks-per-node=$((procs > 32 ? 32 : procs)) \
                   --mem=256G \
                   --output="output_${input_size}_${input_type}_${procs}.%j" \
                   mpi.grace_job $input_size $procs $input_type
        done
    done
done
