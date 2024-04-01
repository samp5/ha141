import matplotlib.pyplot as plt

data = []
with open('../logs/1711995342/1711995342.log', 'r') as file:
    for line in file:
        parts = line.split()
        group_id = int(parts[0])
        neuron_id = int(parts[1])
        neuron_type = parts[2]
        timestamp = float(parts[3])
        value = float(parts[4])
        message_type = parts[5]
        data.append((group_id, neuron_id, neuron_type, timestamp, value, message_type))

message_types = ["S", "R", "N", "D"]
markers = {"S":("+", "gold"), "R":("_", "green"), "N":('.', "gold"), "D": ('|', "red")}

unique_ids = {(entry[0], entry[1]) for entry in data}

MARKER = 0
COLOR = 1
GROUPID = 0
NEURONID = 1
NEURONTYPE = 2
TIMESTAMP = 3
POTENTIAL = 4
MESSAGETYPE = 5

length = 0 

for group_id, neuron_id in unique_ids:

    plt.figure()

    # this is unique neuron id data
    filtered_data = [(entry[TIMESTAMP], entry[POTENTIAL], entry[NEURONTYPE], entry[MESSAGETYPE]) for entry in data if entry[GROUPID] == group_id and entry[NEURONID] == neuron_id]
    
    sorted_data = sorted(filtered_data, key=lambda x: x[0])

    min_time = sorted_data[0][0]

    x_values = [(entry[0] - min_time) for entry in sorted_data]
    y_values = [entry[1] for entry in sorted_data]
    # plt.plot(x_values, y_values, linewidth = 1, linestyle = '--')

    neuron_type = filtered_data[0][2]

    for message_type in message_types:

        filtered_data2 = [data for data in filtered_data if data[3] == message_type]

        sorted_data = sorted(filtered_data2, key=lambda x: x[0])

        if (not sorted_data):
            continue

        min_time = sorted_data[0][0]

        x_values = [(entry[0] - min_time) for entry in sorted_data]
        y_values = [entry[1] for entry in sorted_data]

        plt.scatter(x_values, y_values, c=markers[message_type][COLOR], marker= markers[message_type][MARKER], label = message_type);


    
    plt.xlabel('Time')
    plt.ylabel('Membrane Potential')
    plt.title(f'Group ID: {group_id}, Neuron ID: {neuron_id}, Type: {neuron_type}')
    plt.legend()
    plt.grid(True)
    plt.savefig(f"group_{group_id}neuron_{neuron_id}.png")
    plt.close()

