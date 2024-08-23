import snn
import networkx as nx
import time
import random
from datetime import datetime
import numpy as np
import pandas as pd
from sklearn.datasets import fetch_openml

def div(x: int) -> float:
    return float(x) / 85

# Load the MNIST dataset
print("-> Fetching dataset...")
dataset = "mnist_784"
v = 1

start = time.time()
mnist = fetch_openml(dataset, version=v)
images, labels = mnist["data"], mnist["target"] # type: ignore
end = time.time()
print(f"-> Done, took {(end - start):.5f} seconds")

print("-> Mapping scale function to dataset...")
start = time.time()
images = images.to_numpy()[0:100]
vectorized_div = np.vectorize(div)
images = vectorized_div(images)
end = time.time()
print(f"-> Done, took {(end - start):.5f} seconds")

# images = [[1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0], [1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0],[1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0],[1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0], [1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]]
# dataset = "dummy"
# v = 0
# images = np.array(images)


print("-> NetworkX generating graph...")
start = time.time()
G = nx.navigable_small_world_graph(84, seed=1)
end = time.time()
print(f"-> Done, took {(end - start):.5f} seconds")

print("-> Generating random edge weights...")
for n in G:
    for nbr in G[n]:
        G[n][nbr]["weight"] = random.random() * 10

print("-> Starting network...")

args = ["dummy", "base_config.toml"]
net = snn.pySNN(args)
net.initialize(nx.to_dict_of_dicts(G), 784)
x = []
y = []

for i in range(1,5):
    start = time.time()
    net.runBatch(images[0:i])
    end = time.time()
    x.append(i)
    y.append(end - start)
    print(f"-> {i} stimulus, took {(end - start):.5f} seconds")
    print(f"-> Running batch reset")

    print("-> Fetching data from network")
    out = net.getActivation()
    print("-> Done")
    filestr = datetime.now().strftime("%m%d_%H%M")
    print(f"-> Writing to file {dataset}v{v}_{filestr}.csv")
    np.savetxt(f"{dataset}v{v}_{filestr}.csv", out,delimiter=",", fmt = '%d')
    print("-> Done")
    v = v + 1

    net.batchReset()

with open ('batchSizeVsTime.txt', 'w') as f:
    for i in range(0, len(x)):
        f.write(f"{x[i]} {y[i]}\n")
# for n in G:
#     for nbr in G[n]:
#         old_weight = G[n][nbr]["weight"]
#         G[n][nbr]["weight"] = old_weight + 4;
#
# net.updateWeights(nx.to_dict_of_dicts(G))



# print("-> Fetching data from network")
# out = net.getActivation()
# print("-> Done")

# net.writeData()
# filestr = datetime.now().strftime("%m%d_%H%M")
# print(f"-> Writing to file {dataset}v{v}_{filestr}.csv")
# np.savetxt(f"{dataset}v{v}_{filestr}.csv", out,delimiter=",", fmt = '%d')
# print("-> Done")
