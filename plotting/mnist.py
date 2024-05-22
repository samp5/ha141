from sklearn.datasets import fetch_openml
import numpy as np
import pandas as pd

def div(x: int) -> float:
    return float(x) / 255.0

# Load the MNIST dataset
mnist = fetch_openml('mnist_784', version=1)
images, labels = mnist["data"], mnist["target"] # type: ignore
images = images.map(div)
np.savetxt("mnist.txt", images.values, fmt = '%f')
