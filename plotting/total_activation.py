import matplotlib.pyplot as plt
from pathlib import Path
import numpy as np
import argparse
import os
import math

class DataPoint:
    def __init__(self, timestamp: float, value: float, message_type: str, stimulus_number:int ):
        self.time= timestamp
        self.value = value
        self.message_type = message_type
        self.stimulus_number = stimulus_number

class Options:
    def __init__(self, args: argparse.Namespace):
        if args.recent:
            self.log_title = get_most_recent_log()
            self.log_file = Path(f"../logs/{self.log_title}/{self.log_title}.log")
            if not self.log_file.exists():
                print("Error: most recent log file does not exist")
                quit()
        self.timestep = args.timestep
        self.graph = args.graph

def parser():
    parser = argparse.ArgumentParser(
            prog="Data parser for SNN",
            description="Create graphs, numpy matrices from log files",)
    parser.add_argument("-r", "--recent", action='store_true', help ="Use the most recent log file")
    parser.add_argument("-g", "--graph", action='store_true', help="Generate png results")
    parser.add_argument("-s", "--step", type=float, dest="timestep", default=-1, help="Specify the timestep to be used in data processing")

    return parser.parse_args()

def get_most_recent_log():
    log_dir = ""
    for root, dirs, files in os.walk("../logs/"):
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
    args = parser()
    opts = Options(args)

    data = []
    with open(opts.log_file, 'r') as file:
        for line in file:
            parts = line.split()
            timestamp = float(parts[3])
            value = float(parts[4])
            message_type = parts[5]
            stimulus_number = int(parts[6])

            if message_type == "R":
                data.append(DataPoint(timestamp, value, message_type, stimulus_number))

# sort the data
    data = sorted(data, key = lambda x: x.time)

#max stimulus_number 
    max_stim = data[-1].stimulus_number
    min_stim = data[0].stimulus_number

# all stimuli
    stimlus_set = []
    for i in range(min_stim, max_stim + 1):
        filter = [data_point for data_point in data if data_point.stimulus_number == i]
        stimlus_set.append(filter)

    np_matrix = []
    for data in stimlus_set:

        # get the stimulus_number
        stim_num = data[0].stimulus_number

        # timestep per bin
        opts.timestep = (data[-1].time - data[0].time) / 300 if opts.timestep == -1 else opts.timestep
        bins = 300
        x_values = []
        y_values = []
        lower = data[0].time;
        absolute_lowest = lower
        upper = lower + opts.timestep;

        for i in range(0, bins):
            x_values.append(lower - absolute_lowest)
            y_values.append(sum( ((x.time >= lower) and (x.time < upper)) for x in data))
            lower = upper
            upper = upper + opts.timestep;
        np_matrix.append(y_values)

        if opts.graph:
            generate_graph(stim_num, x_values, y_values, opts)

    a = np.matrix(np_matrix)
    print(a)


if __name__ == "__main__":
    main()
