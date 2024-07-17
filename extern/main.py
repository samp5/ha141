import snn
import networkx as nx
import random
from datetime import datetime
import numpy as np
import pandas as pd
from sklearn.datasets import fetch_openml

def div(x: int) -> float:
    return float(x) / 85
# Load the MNIST dataset
# dataset = "mnist_784"
# v = 1
# mnist = fetch_openml(dataset, version=v)
# images, labels = mnist["data"], mnist["target"] # type: ignore
#
# images = images.map(div).to_numpy()[0:10]
# print(images.shape)

images = [[1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0], [1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0],[1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0],[1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0], [1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]]
images = np.array(images)

G = nx.navigable_small_world_graph(4, seed=1)
edgeWeights = {}

for n in G:
    for nbr in G[n]:
        G[n][nbr]["weight"] = random.random() + 5
        edgeDict = {}
        edgeDict["weight"] = G[n][nbr]["weight"]
        edgeWeights[n, nbr] = edgeDict

args = ["dummy", "base_config.toml"]
net = snn.pySNN(args)
dod = nx.to_dict_of_dicts(G)
print(dod);
net.initialize(dod, images);

for n in G:
    for nbr in G[n]:
        old_weight = G[n][nbr]["weight"]
        G[n][nbr]["weight"] = old_weight + 4;

net.updateWeights(nx.to_dict_of_dicts(G))

net.start()

# out = net.getActivation()
# net.writeData()
# filestr = datetime.now().strftime("%m%d_%H%M")
# np.savetxt(f"{dataset}v{v}_{filestr}.csv", out,delimiter=",", fmt = '%d')
