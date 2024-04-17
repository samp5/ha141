import matplotlib.pyplot as plt
import sys
import os
import math

def help():
    print("Invalid argument")
    print("Options:")
    print("-r Most recent log")
    print("-s <value> timestep value")
    quit()


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

if len(sys.argv) < 2:
    file_name ='../logs/1712692411/1712692411.log' 
    timestep = -1
else:
    if sys.argv[1] == "-r":
        most_recent = get_most_recent_log()
        file_name = f"../logs/{most_recent}/{most_recent}.log"
    else:
        help()
    if len(sys.argv) > 2:
        if sys.argv[2] == "-s":
            try:
                timestep = float(sys.argv[3])
            except:
                help()
        else:
            help()

    else:
        timestep = -1




data = []

with open(file_name, 'r') as file:
    for line in file:
        parts = line.split()
        timestamp = float(parts[3])
        value = float(parts[4])
        message_type = parts[5]

        if message_type == "R":
            data.append((timestamp, value))

# sort the data
print(data)
data = sorted(data, key = lambda x: x[0])
plt.figure(dpi=300)

# timestep
timestep = data[-1][0] / 300 if timestep == -1 else timestep
bins = math.ceil(data[-1][0] / timestep)
x_values = []
y_values =  []
lower = data[0][0];
upper = lower + timestep;

for i in range(0, bins):
    x_values.append(lower)
    y_values.append(sum( ((lower <= x[0]) and (x[0] < upper)) for x in data))
    lower = upper
    upper = upper + timestep;

plt.plot(x_values, y_values, linewidth = 0.5 )
plt.xlabel("Time")
plt.ylabel("Activation")
plt.title(f"Run {most_recent}")
plt.savefig(f"{most_recent}.png")

