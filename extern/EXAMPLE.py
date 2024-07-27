import snn
import networkx as nx
import time
import random
from datetime import datetime
import numpy as np
import pandas as pd
from sklearn.datasets import fetch_openml


images = [[1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0],
          [2.0,2.0,2.0,2.0,2.0,2.0,2.0,2.0,2.0],
          [3.0,3.0,3.0,3.0,3.0,3.0,3.0,3.0,3.0],
          [4.0,4.0,4.0,4.0,4.0,4.0,4.0,4.0,4.0], 
          [5.0,5.0,5.0,5.0,5.0,5.0,5.0,5.0,5.0]]
dataset = "dummy"
v = 0
images = np.array(images)

print("-> NetworkX generating graph...")
start = time.time()
G = nx.navigable_small_world_graph(10, seed=1)
end = time.time()
print(f"-> Done, took {(end - start):.5f} seconds")

print("-> Generating random edge weights...")
for n in G:
    for nbr in G[n]:
        G[n][nbr]["weight"] = random.random() * 10

print("-> Starting network...")
net = snn.pySNN()
net.initialize(nx.to_dict_of_dicts(G), images)

net.setRefractoryMembranePotential(-7.0)
print(f"refractory membrane potential after setting to -7.0: {net.getRefractoryMembranePotential()}");
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

net.setRefractoryMembranePotential(-99.99)
print(f"refractory membrane potential after setting to -99.99: {net.getRefractoryMembranePotential()}");
v = v +1

net.batchReset();
start = time.time()
net.runBatch(images)
end = time.time()

for n in G:
    for nbr in G[n]:
        old_weight = G[n][nbr]["weight"]
        G[n][nbr]["weight"] = old_weight + 4;

net.updateWeights(nx.to_dict_of_dicts(G))


print(f"-> Done, took {(end - start):.5f} seconds")

print("-> Fetching data from network")
out = net.getActivation()
print("-> Done")
# net.writeData()
filestr = datetime.now().strftime("%m%d_%H%M")
print(f"-> Writing to file {dataset}v{v}_{filestr}.csv")
np.savetxt(f"{dataset}v{v}_{filestr}.csv", out,delimiter=",", fmt = '%d')
print("-> Done")
