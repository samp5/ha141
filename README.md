# Spiking Neural Network in C++

## Python API

First clone the repository

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

Create the neccessay local directories

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
<summary>Output stored in <code>dummyv0_0726_1142.csv</code></summary>
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

#### Accessors and Mutators for Configuration variables


<details>
<summary><code>pySNN.setProbabilityOfSuccess(pSuccess: float)</code></summary>
<br>

- Sets the probability of success for input neurons 
    - For a time per stimulus of 100ms and probability of success of 0.5, on average, input neurons will recieve 50 stimulus events.

- Accessor: `getProbabilityOfSucess()`

</details>

<details>
<summary><code>pySNN.setMaxLatency(mLatency: int, update = true : bool)</code></summary>
<br>


- The latency of an input neuron is calculated via their distance the the "center" of an image. 
- For an image with dimensions 5 x 5, and a maximum latency of 10ms input neurons at each "corner" of the image will have a latency of 10ms whereas those in the center will have a latency of (integer rounding) 0. 
- The latency for a given input neuron is calculated as follows:
    1. The index of the input neuron is transformed into a coordinate as if the 1-D stimulus was a 2-D square (or a rectangle of the smallest perimeter)
    2. The distance of the input neuron to the center of the image is calculated
    3. The latency of the input neuron is $d_{\textrm{input}}/d_{\textrm{max}} \times$ `max_latency` interpretted as an integer
- Becuase the image is stored in memory, the default argument `update` rebuilds the image and updates the latency data member of all `InputNeurons`. If for some reason you do not want the latency to be immediately updated, `False` can be specified

- Accessor: `pySNN.getProbabilityOfSucess()`

</details>

<details>
<summary><code>pySNN.setTau(Tau: float)</code></summary>
<br>

- Sets tau, which is used to calculate the decay rate of the membrane potential of a neuron according to

$$    \frac{dV}{dt} = \frac{V(t) - V_{\textrm{rest}}}{\tau} $$
Where $V_{\textrm{rest}}$ is the refractory membrane potential

- Accessor: `pySNN.getTau()`

</details>

<details>
<summary><code>pySNN.setRefractoryDuration(duration: int, update = true : bool)</code></summary>
<br>


- Sets the length of time for which a neuron will not acknowledge messages after firing

- Becuase some frequently accesses configuration variables are copied and stored as `Neuron` data members,  the default argument `update` calls the approapriate mutators and updates these values. If for some reason you do not want the values to be immediately updated, `False` can be specified

- Accessor: `pySNN.getRefractoryDuration()`

</details>

<details>
<summary><code>pySNN.setRefractoryMembranePotential(refractoryMembranePotential: float, update = true: bool);</code></summary>
<br>


-  Set the membrane potential each neuron is set to after firing

- Becuase some frequently accesses configuration variables are copied and stored as `Neuron` data members,  the default argument `update` calls the approapriate mutators and updates these values. If for some reason you do not want the values to be immediately updated, `False` can be specified

- Accessor: `pySNN.getRefractoryMembranePotential()`

</details>

<details>
<summary><code>pySNN.setTimePerStimulus(timePerStimulus: int)</code></summary>
<br>

- Sets the simuluated length of time each stimulus recieves

> [!NOTE] 
> This will linearly increase runtime

- Accessor: `pySNN.getTimePerStimulus()`

</details>

<details>
<summary><code>pySNN.setSeed(seed: int)</code></summary>
<br>


- Seeds random number generator
    - Uses Mersenne Twister random number generator ([`std::mt19937`](https://cplusplus.com/reference/random/mt19937/))
  
</details>

<details>
<summary><code>pySNN.setInitialMembranePotential(initialMembranePotential: float)</code></summary>
<br>

- Sets the membrane potential that initializes each `Neuron`

- Accessor: `pySNN.getInitialMembranePotential()`

</details>

<details>
<summary><code>pySNN.setActivationThreshold(activationThreshold: float, update = True: bool)</code></summary>
<br>


- Sets the membrance potential at which a neuron will fire

- Becuase some frequently accesses configuration variables are copied and stored as `Neuron` data members,  the default argument `update` calls the approapriate mutators and updates these values. If for some reason you do not want the values to be immediately updated, `False` can be specified

- Accessor: `pySNN.getActivationThreshold()`


</details>

### Optimal batch sizes

Batch size has little effect on runtime per stimulus, except for small batches with "normal" activation levels.

The following graph shows runtimes for the MNIST data set for a network of ~8000 neurons where total activation for the network for each stimulus ~20,000 activations.

The jump in runtime per stimulus around 15 stimuli is due to read write operations to a pipe beween parent and child processes. (For small batches the pipe capacity is smaller than the total amount of data passed).



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


