import sys
# sys.path.append("/scratch/group/csce435-f24/python-3.10.4/lib/python3.10/site-packages")
# sys.path.append("/scratch/group/csce435-f24/thicket")
from glob import glob

import matplotlib.pyplot as plt
import pandas as pd

import thicket as th

pd.set_option("display.max_rows", None)
pd.set_option("display.max_columns", None)

#1_trial is a name of a folder containing the cali files, you may create a folder with a different name and replace the folder name here
tk = th.Thicket.from_caliperreader(glob("samplesortcalifiles/*.cali"))

tk.metadata_column_to_perfdata("num_procs")
tk.metadata_column_to_perfdata("input_size")
tk.metadata_column_to_perfdata("input_type")

tk.dataframe = tk.dataframe.reset_index().set_index(["node", "num_procs", "input_size", "input_type"]).sort_index()

processes = [2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]
input_sizes = [2**i for i in range(16, 30, 2)]
input_types = ["Random", "Sorted", "Reverse Sorted", "Sorted with 1% perturbed"]

def create_graphs(df):
    input_sizes = df.index.get_level_values('input_size').unique()
    num_procs = df.index.get_level_values('num_procs').unique()
    nodeVals = ['comm', 'comp']
    print(input_sizes)
    for node in nodeVals:
        for size in input_sizes:
            fig, axs = plt.subplots(2, 3, figsize=(20, 15))
            fig.suptitle(f'Performance Metrics {size} - {node}')
            size_data = df[df.index.get_level_values('input_size') == size]
            axis = [(0,0), (0, 1), (0,2), (1, 0), (1, 1)]
            metrics = ['Min time/rank', 'Max time/rank', "Avg time/rank", "Total time", "Variance time/rank"]
            handles, labels = [], []
            for i, metric in enumerate(metrics):
                ax = axs[axis[i]]
                ax.set_title(metric)
                for input_type in input_types:
                    input_data = size_data[size_data.index.get_level_values('input_type') == input_type]
                    tempList = [node.frame.get("name") for node in input_data.index.get_level_values("node").values]
                    nodeLoc = input_data.index.get_level_values("node").values[tempList.index(node)]
                    data = input_data.loc[nodeLoc]
                    
                    if metric == "Total time":
                        data[metric] = data[metric] / data.index.get_level_values('num_procs')
                    ax.plot(num_procs, data[metric], label=input_type)
                ax.set_xlabel('Number of Processes')
                ax.set_ylabel('Time (s)')
                ax.set_xscale('log', base=2)
                handles, labels = ax.get_legend_handles_labels()
            
            axs[1, 2].axis('off')
            ax = axs[1, 2]
            
            ax.legend(handles, labels)
            plt.savefig(f'samplesort_performance_eval_graphs/performance_metrics_{size}_{node}.png')
            plt.close()

create_graphs(tk.dataframe)