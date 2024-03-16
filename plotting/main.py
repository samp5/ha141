import matplotlib.pyplot as plt

data = []
with open('../logs/1710548245.log', 'r') as file:
    for line in file:
        parts = line.split()
        group_id = int(parts[0])
        neuron_id = int(parts[1])
        timestamp = float(parts[2])
        value = float(parts[3])
        data.append((group_id, neuron_id, timestamp, value))

unique_ids = {(entry[0], entry[1]) for entry in data}

for group_id, neuron_id in unique_ids:

    filtered_data = [(entry[2], entry[3]) for entry in data if entry[0] == group_id and entry[1] == neuron_id]

    sorted_data = sorted(filtered_data, key=lambda x: x[0])
    
    x_values = [entry[0] for entry in sorted_data]
    y_values = [entry[1] for entry in sorted_data]
    
    
    plt.figure()
    plt.plot(x_values, y_values, marker='o' , label=f'Group ID: {group_id}, Item ID: {neuron_id}')
    plt.xlabel('Time')
    plt.ylabel('Membrane Potential')
    plt.title(f'Group ID: {group_id}, Neuron ID: {neuron_id}')
    plt.legend()
    plt.grid(True)
    plt.savefig(f"group_{group_id}neuron_{neuron_id}.png")
