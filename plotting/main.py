import matplotlib.pyplot as plt
import sys
import os

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

if len(sys.argv) == 1:
    file_name ='../logs/1712692411/1712692411.log' 
else:
    if sys.argv[1] == "-r":
        most_recent = get_most_recent_log()
        file_name = f"../logs/{most_recent}/{most_recent}.log"
    else:
        print("Invalid argument")
        quit()



data = []
with open(file_name, 'r') as file:
    for line in file:
        parts = line.split()
        group_id = int(parts[0])
        neuron_id = int(parts[1])
        neuron_type = parts[2]
        timestamp = float(parts[3])
        value = float(parts[4])
        message_type = parts[5]
        data.append((group_id, neuron_id, neuron_type, timestamp, value, message_type))

message_types = ["S", "R", "N", "D", "C"]
markers = {"S":("+", "gold"), "R":("_", "green"), "N":('2', "gold"), "D": ('|', "red"), "C": ('.', 'gray')}

unique_ids = {(entry[0], entry[1], entry[2]) for entry in data}

MARKER = 0
COLOR = 1
GROUPID = 0
NEURONID = 1
NEURONTYPE = 2
TIMESTAMP = 3
POTENTIAL = 4
MESSAGETYPE = 5

length = 0 

for group_id, neuron_id, neuron_type in unique_ids:

    # create figure
    plt.figure(dpi=300)

       # grab data for current neuron
    filtered_data = [(entry[3], entry[4], entry[5]) for entry in data if entry[0] == group_id and entry[1] == neuron_id]
    
    # sort data
    sorted_data = sorted(filtered_data, key=lambda x: x[0])

    # find min time
    min_time = sorted_data[0][0]

    # subtract out min time
    sorted_data = [(entry[0] - min_time, entry[1], entry[2]) for entry in sorted_data]

    # get x and y values
    x_values = [entry[0] for entry in sorted_data]
    y_values = [entry[1] for entry in sorted_data]

    # plot all data for current neuron
    plt.plot(x_values, y_values, linewidth = 0.1 , linestyle = '--')

    for message_type in message_types:

        # filter for message type
        filtered_data2 = [entry for entry in sorted_data if entry[2] == message_type]

        # if its empty continue
        if (not filtered_data2):
            continue


        # get x and y values
        x_values = [entry[0] for entry in filtered_data2]
        y_values = [entry[1] for entry in filtered_data2]

        #scatter plot
        plt.scatter(x_values, y_values, c=markers[message_type][COLOR], marker= markers[message_type][MARKER], label = message_type, linewidths=0.35);

    plt.xlabel('Time')
    plt.ylabel('Membrane Potential')
    plt.title(f'Group ID: {group_id}, Neuron ID: {neuron_id}, Type: {neuron_type}')
    plt.legend()
    plt.grid(True)
    plt.savefig(f"group_{group_id}neuron_{neuron_id}.png")
    plt.close()

