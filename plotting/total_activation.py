import matplotlib.pyplot as plt
from pathlib import Path
import numpy as np
import argparse
import os

class DataPoint:
    def __init__(self, timestamp: float, value: float, stimulus_number:int ):
        self.time= timestamp
        self.value = value
        self.stimulus_number = stimulus_number

class Options:
    def __init__(self, args: argparse.Namespace):
        if args.recent:
            self.log_title = get_most_recent_log()
            self.log_file = Path(f"../logs/{self.log_title}/{self.log_title}.log")
            if not self.log_file.exists():
                print("Error: most recent log file does not exist")
                quit()
        else:
            if args.filename:
                self.log_title = args.filename
                if self.log_title[-4:] == ".log":
                    self.log_title = self.log_title[:-4]
                self.log_file = Path(f"../logs/{self.log_title}/{self.log_title}.log")
            else:
                print("Error: No file supplied")
                quit()
            if not self.log_file.exists():
                print(f"Error: {self.log_file} does not exist")
                quit()
        self.timestep = args.timestep
        self.graph = args.graph
        self.graphs = args.graphfor
        self.csv = args.csv
        self.output = args.output_file

def parser():
    parser = argparse.ArgumentParser(
            prog="Data parser for SNN",
            description="Create graphs, numpy matrices from log files",)
    parser.add_argument("-f","--filename", default="", dest="filename", help = "log filename to use", )
    parser.add_argument("-r", "--recent", action='store_true', help ="Use the most recent log file")
    parser.add_argument("-c", "--csv", action='store_true', help ="output CSV")
    parser.add_argument("-g", "--graph", action='store_true', help="Generate png results")
    parser.add_argument("-G", "--graphfor", nargs='*', dest="graphfor", default=[], help="Specify the stimulus number(s) for which to generate graphs")
    parser.add_argument("-s", "--step", type=float, dest="timestep", default=-1, help="Specify the timestep to be used in data processing")
    parser.add_argument("-o","--output", default="", dest="output_file", help = "Output file name")

    return parser, parser.parse_args()

def get_most_recent_log():
    log_dir = ""
    for root, dirs, _ in os.walk("../logs/"):
        if root == "../logs/":
            log_dir = dirs;
            break;
    max = log_dir[0];
    for log in log_dir:
        if log > max:
            max = log
    
    return max

def generate_graph(stim_num: int,x_values: list[float], y_values: list[float], opts: Options):
        plt.figure(dpi=300)
        plt.ylim(0, 30)
        plt.plot(x_values, y_values, linewidth = 0.5 )
        plt.xlabel("Time")
        plt.ylabel("Activation")
        plt.title(f"Run {opts.log_title} Stimulus #{stim_num}")
        plt.savefig(f"{opts.log_title}-{stim_num}.png")
        plt.close()



def main():
    psr, args = parser()
    opts = Options(args)

    if not any([opts.graph, opts.graphs != [], opts.csv, opts.log_title != ""]):
        print("-> No output format indicated, not processing file")
        print()
        psr.print_help()
        quit()

    min_stim = 100000000000
    max_stim = -1

    data = {}
    print("-> Processing file...")
    with open(opts.log_file, 'r') as file:
        for line in file:
            parts = line.split()
            timestamp = float(parts[3])
            value = float(parts[4])
            message_type = parts[5]
            stimulus_number = int(parts[6])

            max_stim = max(stimulus_number, max_stim)
            min_stim = min(stimulus_number, min_stim)

            if message_type == "R":
                if stimulus_number in data:
                    data[stimulus_number].append(DataPoint(timestamp, value, stimulus_number))
                else:
                    data[stimulus_number] = [DataPoint(timestamp, value, stimulus_number)]

# sort the data
    print("-> Sorting data...")
    for stim_num in data:
        data[stim_num] = sorted(data[stim_num], key = lambda x: x.time)

# all stimuli
    update_index = (max_stim - min_stim) / 10
    np_matrix = []
    for stim_num in range(min_stim, max_stim + 1):
        if stim_num % update_index == 0:
            print(f"-> Processing stimulus {stim_num}...")
        stim_data = data[stim_num]
        # timestep per bin
        opts.timestep = (stim_data[-1].time - stim_data[0].time) / 300 
        bins = 300
        x_values = []
        y_values = []
        lower = stim_data[0].time;
        absolute_lowest = lower
        upper = lower + opts.timestep;

        for _ in range(0, bins):
            x_values.append(lower - absolute_lowest)
            y_values.append(sum( ((x.time >= lower) and (x.time < upper)) for x in stim_data))
            lower = upper
            upper = upper + opts.timestep;

        np_matrix.append(y_values)

        if opts.graph:
            generate_graph(stim_num, x_values, y_values, opts)
        elif stim_num in opts.graphs:
            print(f"-> Generating graph for stimulus {stim_num}...")
            generate_graph(stim_num, x_values, y_values, opts)

    if opts.csv:
        opts.output = opts.output if opts.output != "" else opts.log_title + ".csv"
        print(f"-> Generating CSV...")
        a = np.matrix(np_matrix)
        np.savetxt(f"{opts.output}", a, delimiter = ",", fmt='%d')

if __name__ == "__main__":
    main()
