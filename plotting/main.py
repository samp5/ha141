import matplotlib.pyplot as plt

data = []
with open('../logs/1711396161/1711396161.log', 'r') as file:
    for line in file:
        parts = line.split()
        group_id = int(parts[0])
        neuron_id = int(parts[1])
        neuron_type = parts[2]
        timestamp = float(parts[3])
        value = float(parts[4])
        data.append((group_id, neuron_id, neuron_type, timestamp, value))

unique_ids = {(entry[0], entry[1]) for entry in data}

for group_id, neuron_id in unique_ids:

    filtered_data = [(entry[3], entry[4], entry[2]) for entry in data if entry[0] == group_id and entry[1] == neuron_id]

    neuron_type = filtered_data[1][2]
    sorted_data = sorted(filtered_data, key=lambda x: x[0])

    min_time = sorted_data[0][0]
    
    x_values = [entry[0] - min_time for entry in sorted_data]
    y_values = [entry[1] for entry in sorted_data]
    
    
    plt.figure()
    plt.scatter(x_values, y_values, marker='.' , label=f'Group ID: {group_id}, Item ID: {neuron_id}')
    plt.xlabel('Time')
    plt.ylabel('Membrane Potential')
    plt.title(f'Group ID: {group_id}, Neuron ID: {neuron_id}, Type: {neuron_type}')
    plt.legend()
    plt.grid(True)
    plt.savefig(f"group_{group_id}neuron_{neuron_id}.png")
    plt.close()

