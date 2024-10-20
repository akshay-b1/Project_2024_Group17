#!/bin/bash

input_types=(0 1 2 3)
input_type_names=("Random" "Sorted" "ReverseSorted" "OnePercentPerturbed")
array_sizes=(65536 262144 1048576 4194304 16777216 67108864 268435456)
process_configs=(
    "1 2"
    "1 4"
    "1 8"
    "1 16"
    "1 32"
    "2 32"
    "4 32"
    "8 32"
    "16 32"
    "32 32"
)

declare -A time_limits
time_limits[65536]="00:30:00"
time_limits[262144]="00:30:00"
time_limits[1048576]="00:45:00"
time_limits[4194304]="01:00:00"
time_limits[16777216]="01:00:00"
time_limits[67108864]="01:00:00"
time_limits[268435456]="01:00:00"

for size in "${array_sizes[@]}"; do
    for config in "${process_configs[@]}"; do
        read nodes tasks_per_node <<< $config
        procs=$((nodes * tasks_per_node))
        
        time_limit="${time_limits[$size]}"
        
        if [[ $procs -ge 1024 ]]; then
            time_limit="00:07:00"
        elif [[ $procs -ge 512 ]]; then
            time_limit="00:10:00"
        elif [[ $procs -ge 256 ]]; then
            time_limit="00:13:00"
        elif [[ $procs -ge 128 ]]; then
            time_limit="00:16:00"
        elif [[ $procs -ge 16 ]]; then
            if [[ "$time_limit" == "01:00:00" ]]; then
                time_limit="00:45:00"
            elif [[ "$time_limit" == "00:45:00" ]]; then
                time_limit="00:30:00"
            elif [[ "$time_limit" == "00:30:00" ]]; then
                time_limit="00:20:00"
            fi
        fi
        
        for i in "${!input_types[@]}"; do
            type="${input_types[i]}"
            type_name="${input_type_names[i]}"
            
            mem_per_node=256
            
            job_name="p${procs}-a${size}-t${type}"
            
            # create the new script
            cat << EOF > job_${job_name}.sh
#!/bin/bash
#SBATCH --export=NONE
#SBATCH --get-user-env=L
#SBATCH --job-name=${job_name}
#SBATCH --time=${time_limit}
#SBATCH --nodes=${nodes}
#SBATCH --ntasks-per-node=${tasks_per_node}
#SBATCH --mem=${mem_per_node}G
#SBATCH --output=${job_name}.%j.out

module load intel/2020b
module load CMake/3.12.1
module load GCCcore/8.3.0
module load PAPI/6.0.0

CALI_CONFIG="spot(output=${job_name}.cali,time.variance,profile.mpi)" \\
mpirun -np $procs ./samplesort $size $type
EOF

            sbatch job_${job_name}.sh
            
            rm job_${job_name}.sh
            
            echo "Submitted job: ${job_name} with time limit ${time_limit}, ${nodes} nodes, ${tasks_per_node} tasks per node, and ${mem_per_node}G RAM per node"
        done
    done
done
