import snn
from datetime import datetime
import numpy as np
import pandas as pd
from sklearn.datasets import fetch_openml

def div(x: int) -> float:
    return float(x) / 85
# Load the MNIST dataset
dataset = "mnist_784"
v = 1
mnist = fetch_openml(dataset, version=v)
images, labels = mnist["data"], mnist["target"] # type: ignore

images = images.map(div).to_numpy()[0:10]
print(images.shape)

list = ["dummy", "base_config.toml"]
net = snn.pySNN(list)
net.generateSynapses()
net.start(images)
out = net.getActivation()
net.writeData()
filestr = datetime.now().strftime("%m%d_%H%M")
np.savetxt(f"{dataset}v{v}_{filestr}.csv", out,delimiter=",", fmt = '%d')
