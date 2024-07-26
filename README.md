# Spiking Neural Network in C++

## Python API

First clone the repository if you haven't 

```bash
git clone https://github.com/samp5/ha141.git snn
```

Change to the repo and initialize the Pybind11 submodule with 

```bash
cd snn
git submodule update --init
```
Create the library with

```bash 
make pybind
```

This will create a python package is `snn/extern`.

Create a virtual enviroment (if you want) and install the necessary dependencies

```bash
cd extern
mkdir venv
python3 -m venv ./venv
source venv/bin/activate
pip install networkx numpy pandas scikit-learn
```

Crate the neccessay local directories

```bash
mkdir run_config
```

`./run_config` holds  `toml` files that specify runtime parameters for the network. If no file is specified in the initial arguement list passed to `snn.pySNN(args: [str])`, `base_config.toml` is used or created and used if it does not exist.


### Python interface and usage

- The `snn` module has a small interface but deep functionality.

#### `pySNN(configFile = "base_config.toml": string)`

- Return a spiking neural network object.

```python
import snn

net = snn.pySNN("my_custom_config.toml")

# or to use base_config.toml

net = snn.pySNN()
```

#### `pySNN.initialize( adjacencyDict : dict[tuple[int, int] : dict[tuple[int, int] : dict[string : float]]],  numberInput : int | stimulus : numpyArray )`

##### A note on the adjacencyDict:
- The adjacencyDict should be a dictionary with the following structure
```
{
   (0,0) : {
            (0,1) : {
                    "weight" : 1.0
                    }
            (0,2) : {
                    "weight" : 2.0
                    }

            }
   (0,1) : {
            (1,1) : {
                    "weight" : 1.0
                    }
            (0,2) : {
                    "weight" : 2.0
                    }

            }
    ...
}
```


The function is overloaded to either accept a  python numpy array and automatically detemine the number of input neurons from the number of stimulus inputs in a single dimension of the array (the number of columns) or a integer representing the desired number of input neurons.

> [!IMPORTANT]
> For size $n$ stimulus with $m$ input neurons
> Only the first $m$ stimulus inputs will be read.


The `adjacencyDict` can be automatically generated from `networkx` as long as the Graph is a **directed grid**

##### Example usage

```python
import networkx as nx
import numpy as np
G = nx.navigable_small_world_graph(10, seed=1)

# since a directed grid is NOT weighted, you have to add weights
for n in G:
    for nbr in G[n]:
        G[n][nbr]["weight"] = random.random()

stimulus = np.array([[1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0],
                     [1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0],
                     [1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0],
                     [1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0],
                     [1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]])

net = snn.pySNN(); # Create the network
net.initialize(nx.to_dict_of_dicts(G), stimulus) # initialize the network

net.runBatch(stimulus[0:2]) # Run the batch
activation = net.getActivation() # Get activation
net.batchReset()
```



#### `pySNN.updateWeights( adjacencyDict : dict[tuple[int, int] : dict[tuple[int, int] : dict[string : float]]] )`

Updates synapse weight for connection between `(x,y)` and `(a,b)` based on `adjacencyDict[(x,y)][(a,b)]["weight"]`

#### `pySNN.runBatch(buffer : numpy array)`

Starts a child process of the network in order to run the given stimulus set.

#### `pySNN.getActivation() -> numpyArray`

Outputs a numpy array with "time per stimulus" columns and "number of stimulus" rows

##### Example usage

```python
import snn
import networkx as nx
import time
import random
from datetime import datetime
import numpy as np

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
start = time.time()
net.runBatch(images)
end = time.time()

print(f"-> Done, took {(end - start):.5f} seconds")

print("-> Fetching data from network")
out = net.getActivation()
print("-> Done")

filestr = datetime.now().strftime("%m%d_%H%M")
print(f"-> Writing to file {dataset}v{v}_{filestr}.csv")
np.savetxt(f"{dataset}v{v}_{filestr}.csv", out,delimiter=",", fmt = '%d')
print("-> Done")

```

<details>
<summary>Output stored in `dummyv0_0726_1142.csv`</summary>
<br>

```csv
4,3,8,2,1,0,5,7,5,1,1,0,9,6,3,1,0,9,6,0,3,1,0,0,0,0,0,0,5,8,6,0,0,0,0,0,9,7,3,0,0,0,0,9,7,3,0,0,0,9,7,3,0,0,9,7,3,0,0,0,0,4,3,4,6,0,4,3,4,1,0,5,0,1,3,6,5,0,1,3,0,2,0,0,9,3,3,0,0,0,0,0,0,0,0,0,0,0,9,5,0,1,0,0,0,4,8,3,0,0,0,9,5,1,0,0,9,5,1,1,0,1,0,5,2,6,1,1,1,5,2,2,1,0,0,0,0,0,9,4,0,2,2,0,0,5,6,6,1,1,5,2,5,5,0,5,2,5,1,0,4,5,2,5,1,0,9,2,5,1,0,0,0,9,6,2,0,0,9,0,6,2,0,0,0,0,5,7,5,0,0,0,9,6,2,0,0,9,6,0
4,3,5,2,4,4,3,4,2,4,4,3,4,1,5,4,3,4,1,5,0,0,4,3,4,1,0,0,9,5,3,0,0,0,0,9,6,2,0,0,9,6,2,0,0,9,6,2,0,0,9,6,2,0,0,9,6,2,0,0,9,6,2,0,0,9,0,6,2,0,0,9,6,3,0,0,9,6,3,0,0,0,9,6,3,0,0,9,6,3,0,0,9,6,3,1,0,0,9,0,6,3,1,0,9,6,3,1,0,9,6,3,1,0,9,6,3,1,0,9,6,3,1,0,9,6,3,1,0,0,9,6,3,0,1,0,0,0,9,7,3,0,0,9,7,3,0,0,0,9,7,3,0,0,9,7,3,0,0,9,7,3,0,0,9,7,0,3,0,0,0,9,7,3,0,0,0,9,7,3,0,0,9,7,3,0,0,9,7,3,0,0,9,7,3,0,0,9,7,0
4,3,5,2,4,4,3,4,2,4,4,3,4,1,5,4,3,4,1,5,0,0,4,3,4,1,0,0,9,5,3,0,0,0,9,6,2,0,0,9,0,6,2,0,0,9,6,2,0,0,9,6,2,0,0,9,6,2,0,0,9,6,2,0,0,9,6,2,0,0,9,6,2,0,0,9,6,2,0,0,0,0,9,6,2,0,0,9,6,2,0,0,9,6,2,0,0,0,9,6,3,0,0,9,6,3,0,0,9,6,3,0,0,9,6,3,0,0,9,0,6,3,0,0,9,6,3,0,0,0,9,6,3,0,0,0,0,9,6,3,1,0,9,6,3,1,0,0,9,6,3,1,0,9,6,3,1,0,9,0,6,3,1,0,9,6,3,1,0,0,9,6,3,1,0,0,9,6,3,1,0,9,6,3,1,0,9,6,3,1,0,9,6,3,1,0,9,6,3,0
4,3,5,2,4,4,3,5,3,4,4,3,5,2,5,4,3,5,2,5,0,0,4,3,5,2,0,0,9,5,4,1,0,0,0,9,6,3,1,0,9,6,3,1,0,9,6,3,1,0,9,6,3,1,0,9,6,3,1,0,9,6,3,1,0,9,0,6,3,1,0,9,6,3,1,0,9,6,3,1,0,0,9,6,3,1,0,9,6,3,1,0,9,6,3,1,0,0,9,0,6,3,1,0,9,6,3,1,0,9,6,3,1,0,9,6,3,1,0,9,6,3,1,0,9,6,3,1,0,0,9,6,3,0,1,0,0,0,9,7,3,0,0,9,7,3,0,0,0,9,7,3,0,0,9,7,3,0,0,9,7,3,0,0,9,7,0,3,0,0,0,9,7,3,0,0,0,9,7,3,0,0,9,7,3,0,0,9,7,3,0,0,9,7,3,0,0,9,7,0
4,0,0,0,4,7,5,2,1,4,7,5,2,0,5,7,5,2,0,5,3,4,5,0,0,0,0,0,9,6,2,0,0,0,0,9,6,2,0,0,9,6,2,0,0,9,6,2,0,0,9,6,2,0,0,9,6,4,0,0,9,6,4,0,0,9,0,6,4,0,0,9,6,4,0,0,9,6,4,0,0,0,9,7,3,0,0,9,7,3,0,0,9,7,3,0,0,0,9,0,7,3,0,0,9,7,3,0,0,9,7,3,0,0,9,7,3,0,0,9,7,3,0,0,9,7,3,0,0,0,9,7,3,0,0,0,0,0,9,7,3,0,0,9,7,3,0,0,0,9,7,3,0,0,9,7,3,0,0,9,7,3,0,0,9,7,0,3,0,0,0,9,7,3,0,0,0,9,7,3,0,0,9,7,3,0,0,9,7,3,0,0,9,7,3,0,0,9,7,0
```

</details>


#### `pySNN.batchReset()` 

Reset the network to be ready to run another batch.


### Optimal batch sizes

Batch size has little effect on runtime per stimulus, except for small batches with "normal" activation levels.

The jump in runtime per stimulus is due to read write operations to a pipe beween parent and child processes. (For small batches the pipe capacity is smaller than the total amount of data passed).

Heres my optimization graph (because its so fast now)

![Optimization graph](./images/optimization.jpg)



## C++
To run the network:
- Clone the repository

```bash
git clone https://github.com/samp5/ha141.git snn

```

- Create the following directories in the root folder of the repo
```bash
cd snn
mkdir build logs run_config
```

- Build the executable
```bash
make build
```

- Run the network
```bash
make run

```
If it doesn't already exist this will create a `base_config.toml` file in `run_config/`. If you want to run the network on a different config use `build/snn <filename>.toml`
```bash
build/snn my_custom_config.toml
```
All other options are specified in config file


