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
tk = th.Thicket.from_caliperreader(glob("samplesort/samplesortcalifiles/*.cali"))

tk.metadata_column_to_perfdata("num_procs")
tk.metadata_column_to_perfdata("input_size")
tk.metadata_column_to_perfdata("input_type")

tk.dataframe = tk.dataframe.reset_index().set_index(["node", "num_procs", "input_size", "input_type"]).sort_index()

processes = [2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]
input_sizes = [2**i for i in range(16, 30, 2)]
input_types = ["Random", "Sorted", "Reverse Sorted", "Sorted with 1% perturbed"]

def create_scaling_plots(df):
    input_sizes = df.index.get_level_values('input_size').unique()
    num_procs = df.index.get_level_values('num_procs').unique()
    input_types = df.index.get_level_values('input_type').unique()
    nodes = ['comp_large', 'comm', 'main']

    for node in nodes:
        for size in input_sizes:
            fig, ax = plt.subplots(figsize=(10, 7))
            ax.set_title(f'Strong Scaling - {node} - Input Size {size}', fontsize=16)
            size_data = df[df.index.get_level_values('input_size') == size]
            for input_type in input_types:
                input_data = size_data[size_data.index.get_level_values('input_type') == input_type]
                tempList = [n.frame.get("name") for n in input_data.index.get_level_values("node").values]
                nodeLoc = input_data.index.get_level_values("node").values[tempList.index(node)]
                data = input_data.loc[nodeLoc]
                
                # f = data['Total time'].iloc[0] / data['Total time']
                # speedup = 1 / (f + (1 - f) / num_procs)
                wall_clock_times = data['Total time'] / data.index.get_level_values('num_procs')

                base_time = wall_clock_times.iloc[0]
                
                speedup = base_time / wall_clock_times
                ax.plot(num_procs, speedup, label=input_type)
            
            # ideal line
            # ax.plot(num_procs, num_procs, linestyle='--', color='gray', label='Ideal')
            ax.set_xscale('log', base=2)
            ax.set_yscale('log', base=2)
            ax.set_xlabel('Number of Processes')
            ax.set_ylabel('Speedup')
            ax.legend(title='Input Type')
            plt.savefig(f'samplesort/samplesort_presentation_graphs/strong_scaling_{node}_input_size_{size}.png')
            plt.close()

        for input_type in input_types:
            fig, ax = plt.subplots(figsize=(10, 7))
            ax.set_title(f'Strong Scaling Speedup - {node} - {input_type}', fontsize=16)
            input_data = df[df.index.get_level_values('input_type') == input_type]
            for size in input_sizes:
                size_data = input_data[input_data.index.get_level_values('input_size') == size]
                tempList = [n.frame.get("name") for n in size_data.index.get_level_values("node").values]
                nodeLoc = size_data.index.get_level_values("node").values[tempList.index(node)]
                data = size_data.loc[nodeLoc]

                # f = data['Total time'].iloc[0] / data['Total time']
                # speedup = 1 / (f + (1 - f) / num_procs)
                wall_clock_times = data['Total time'] / data.index.get_level_values('num_procs')

                base_time = wall_clock_times.iloc[0]
                
                speedup = base_time / wall_clock_times
                ax.plot(num_procs, speedup, label=f'Size {size}')
            
            # ideal line
            # ax.plot(num_procs, num_procs, linestyle='--', color='gray', label='Ideal')
            ax.set_xscale('log', base=2)
            ax.set_yscale('log', base=2)
            ax.set_xlabel('Number of Processes')
            ax.set_ylabel('Speedup')
            ax.legend(title='Input Size')
            plt.savefig(f'samplesort/samplesort_presentation_graphs/strong_scaling_speedup_{node}_{input_type}.png')
            plt.close()

        for input_type in input_types:
            fig, ax = plt.subplots(figsize=(10, 7))
            ax.set_title(f'Weak Scaling - {node} - {input_type}', fontsize=16)
            input_data = df[df.index.get_level_values('input_type') == input_type]
            for procs in num_procs:
                proc_data = input_data[input_data.index.get_level_values('num_procs') == procs]
                tempList = [n.frame.get("name") for n in proc_data.index.get_level_values("node").values]
                nodeLoc = proc_data.index.get_level_values("node").values[tempList.index(node)]
                data = proc_data.loc[nodeLoc]

                ax.plot(input_sizes, data['Total time'], label=f'{procs} Processes')
            ax.set_xscale('log', base=2)
            ax.set_xlabel('Input Size')
            ax.set_ylabel('Total Time (s)')
            ax.legend(title='Number of Processes')
            plt.savefig(f'samplesort/samplesort_presentation_graphs/weak_scaling_{node}_{input_type}.png')
            plt.close()

create_scaling_plots(tk.dataframe)