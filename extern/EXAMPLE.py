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
images = images.to_numpy()[0:5]
vectorized_div = np.vectorize(div)
images = vectorized_div(images)
end = time.time()
print(f"-> Done, took {(end - start):.5f} seconds")

print("-> NetworkX generating graph...")
start = time.time()
G = nx.navigable_small_world_graph(84, seed=1)
end = time.time()
print(f"-> Done, took {(end - start):.5f} seconds")

print("-> Generating random edge weights...")
for n in G:
    for nbr in G[n]:
        G[n][nbr]["weight"] = random.random() * 10
        G[n][nbr]["delay"] = random.randint(2, 10)

print("-> Starting network...")
net = snn.pySNN()
net.initialize(nx.to_dict_of_dicts(G), images)

# net.setRefractoryMembranePotential(-7.0)
# print(f"refractory membrane potential after setting to -7.0: {net.getRefractoryMembranePotential()}");
start = time.time()
net.runBatch(images)
end = time.time()

print("-> Fetching data from network")
out = net.getActivation()
print("-> Done")
# net.writeData()
filestr = datetime.now().strftime("%m%d_%H%M")
print(f"-> Writing to file {dataset}v{v}_{filestr}.csv")
np.savetxt(f"{dataset}v{v}_{filestr}.csv", out,delimiter=",", fmt = '%d')
print("-> Done")

# net.setRefractoryMembranePotential(-99.99)
# print(f"refractory membrane potential after setting to -99.99: {net.getRefractoryMembranePotential()}");
# v = v +1
#
# net.batchReset();
# start = time.time()
# net.runBatch(images)
# end = time.time()
#
for n in G:
    for nbr in G[n]:
        old_weight = G[n][nbr]["weight"]
        G[n][nbr]["weight"] = old_weight + 4;
#
net.updateWeights(nx.to_dict_of_dicts(G))


# print(f"-> Done, took {(end - start):.5f} seconds")
#
# print("-> Fetching data from network")
# out = net.getActivation()
# print("-> Done")
# # net.writeData()
# filestr = datetime.now().strftime("%m%d_%H%M")
# print(f"-> Writing to file {dataset}v{v}_{filestr}.csv")
# np.savetxt(f"{dataset}v{v}_{filestr}.csv", out,delimiter=",", fmt = '%d')
# print("-> Done")
