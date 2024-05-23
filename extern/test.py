from . import snn
import numpy as np
import pandas as pd
from sklearn.datasets import fetch_openml

def div(x: int) -> float:
    return float(x) / 255.0
# Load the MNIST dataset
mnist = fetch_openml('mnist_784', version=1)
images, labels = mnist["data"], mnist["target"] # type: ignore
images = images.map(div).to_numpy()

list = ["dummy", "base_config.toml"]
net = snn.pySNN(list)
net.generateSynapses()
net.start(images)
net.join()
out = net.getActivation()
np.savetxt("data.csv", out, fmt = '%d')
