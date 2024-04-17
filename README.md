# Honors Supplement for CS 141
Project for CS 141 Honors Supplement: Toy spiking neural network using a multithreaded approach.

### 📁 Folder Structure

```
├── src
│   ├── functions.hpp
│   ├── functions.cpp
│   ├── input_neuron.hpp
│   ├── input_neuron.cpp
│   ├── log.hpp
│   ├── log.cpp
│   ├── main.cpp
│   ├── main_neuron_groups.cpp
│   ├── message.hpp
│   ├── neuron.hpp
│   ├── neuron.cpp
│   ├── neuron_group.hpp
│   ├── neuron_group.cpp
│   ├── synapse.hpp
│   └── synapse.cpp
├── images // hold images for README
├── pthred_ex //practice pthread examples
│   └── ...
├── plotting 
│   ├── venv //virtual environment for python packages
│   ├── total_activation.py
│   └── main.py

├── run_config 
│   ├── base_config.toml // base config
│   └── toml.hpp // header for toml++
├── logs 
│   └── // all .log files ignored 
├── input_files 
│   └── ... // input files for neuron activation
├── .gitignore
├── README.md
└── makefile
```

### In-Progress 🚀
- [x] ~~Create and integrate Log Class~~
- [x] ~~Neuron Group Class~~
- [x] ~~Activate Neuron from file inputs~~
- [x] ~~Decay functionality~~
- [x] ~~TOML configuration for run-time options~~
- [x] ~~Neuron Types for differentiated functionality (input, output)~~
- [x] Message list sorting
- [ ] Data structure for tracking presynaptic propagation 
- [ ] Copy functionality for replicating graph layout

| Date  | Key Points 🔑   |  Issues 🐛   |
|--------------- | --------------- |--------------- |
| [4-16](#-update-4-16)   | Total activation graph |  None |
| [4-14](#-update-4-14)   | Multiple stimuli same run |  None |
| [4-11](#-update-4-11)   | New synapse class |  None |
| [4-10](#-update-4-10)   | Graphing looks much much better. New decay functionality, fixed a sneaky SEGV |  None |
| [4-1](#-update-4-1)   | Graph updates (markers for different events) some logic changes for neuron activation/firing/etc. Input Neurons! | Not entirely sure the neuron implementation is correct, but the graphs are looking more promising |
| [3-19](#-update-3-19)   | Changed decay funciton, firing logic, and x-axis for graphing| None |
| [3-18](#-update-3-18)   | Neuron input type | None |
| [3-15](#-update-3-15)   | .toml configuration for run-time options! | None |
| [3-14](#-update-3-14)   | Messaging working between and within groups! Reading from file. Decay functionality. Basic plotting with Python | None |
| [3-12](#-update-3-12)   | Start of messaging functionality between neuron groups. | None |
| [3-11](#-update-3-11)   | Start of Neruon Group Class| None |
| [3-5](#-update-3-5)   | New fully integrated Log class. Write neuron ids and potential values to a `<current_time>.log` file. | None |
| [3-3 pt.2](#-update-3-3-part-2)   | Fixed [Issue 2](#-issue-2)| None |
| [3-3](#-update-3-3)   | Added time stamps to logging messages. Added function descriptions.| None |
| [2-29](#-update-2-29)   | Updated Neuron Class with with membrane potentials, refractory phases, Update to edge weights, fixed issue 1, guard clauses on header files.   | "Quit" functionality does not work for the menu [~~Issue 2~~](#-issue-2)|
| [2-28](#-update-2-28)   | Basic Node class that sends and recieves messages   | `random_neighbors` may repeat edges. [~~Issue 1~~](#-issue-1)|

### 📌 Update 4-16
**New addtions:**
- Total Activation graphs!
    - Local network on dummy data set

     ![Local network on dummy data set](https://github.com/samp5/ha141/blob/main/images/3315642.png)
- First mnist data set run!
    - Ran with 4900 neurons, 784 input neurons and 500,000 connections over 3 seconds
    - Mnist data set line 1

     ![Mnist data set line 1](https://github.com/samp5/ha141/blob/main/images/3313330.png)
    - Mnist data set line 4

     ![Mnist data set line 4](https://github.com/samp5/ha141/blob/main/images/3315419.png)
- Added functions to read specific line of a file `set_line_x(const std::vector<InputNeuron *>& vec, int target)`
- New python script to generate total activation graph. 
    - `-r` for most recent
    - `-s <value>` to set the time step (size of segment where activations are counted)
        - Default value is to have 350 "blocks" or `runtime / 350`

### 📌 Update 4-14
**New addtions:**
- Changed timing system to `std::chrono::high_resolution_clock`
- Changed the way that input is read and fed to `InputNeuron`s
- Uses `std::getline` with an `ifstream` to keep track of where we are in the file and read successive lines after a given amount of time
    - Specifics:
        - Successive stimuli **must** be on separate lines, but mnist data is already in this format so it works
        - For $n$ input neurons, only $n$ values are read from a given line. The rest are thrown away
- Graphs showing multiple stimulus switches during a run
    - ![Input Neuron Example](https://github.com/samp5/ha141/blob/main/images/41401.png)
    - ![Regular Neuron Example](https://github.com/samp5/ha141/blob/main/images/41402.png)

#### New control path for `InputNeuron`s
- The stimulus values for an `InputNeuron` are updated via a `pthread_cond_signal` 
    - Anytime the `InputNeuron` is run, a global bool is checked, `switching_stimulus`

```cpp
if (::switching_stimulus) {
// wait for all input neurons to switch to the new stimulus
pthread_mutex_lock(&::stimulus_switch_mutex);
while (::switching_stimulus) {
  pthread_cond_wait(&::stimulus_switch_cond, &::stimulus_switch_mutex);
}
pthread_mutex_unlock(&::stimulus_switch_mutex);
}

```

- The input neuron then waits, effectivley haulting its parent group's thread, until all values for all `InputNeuron`s have been updated. 
- To account for the time taken for this process, an offset is introduced to the `Log` member function `get_time_stamp`

```cpp

// main_neuron_groups.cpp ... 

auto start = std::chrono::high_resolution_clock::now();
switching_stimulus = true;

// update the stimulus values for each input_neuron
set_next_line(input_neurons);

auto end = std::chrono::high_resolution_clock::now();

auto duration =
    std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

// adjust the offset
lg.set_offset(duration.count());

pthread_mutex_lock(&stimulus_switch_mutex);
switching_stimulus = false;
pthread_cond_broadcast(&stimulus_switch_cond);
pthread_mutex_unlock(&stimulus_switch_mutex);
```

<details>
<summary>Specifics of helper functions</summary>
<br>

```cpp
// to set values of each input neuron 
void set_next_line(const vector<InputNeuron *> &input_neurons) {
  std::string line;
  get_next_line(line);

  std::stringstream s(line);
  double value;

  for (InputNeuron *input_neuron : input_neurons) {
    s >> value;
    input_neuron->set_input_value(value);
  }
}

//updated get_time_stamp

double Log::get_time_stamp() {

  hr_clock::time_point now = hr_clock::now();

  duration time_span = std::chrono::duration_cast<duration>(now - this->start);
  return time_span.count() - this->off_set;
}
```


</details>


### 📌 Update 4-11
**New addtions:**
- `Synapse` Class to replace maps in `Neuron` Class
- Hopefully will make backpropagation easier
    - Keep track of `last_weight` in `Synapse`
- Command-line arguments for `main.py`
    - use `-r` to automatically use the most recent log file in the `logs` folder

```cpp
class Synapse {
public:
  Synapse(Neuron *from, Neuron *to, double w)
      : _origin(from), _destination(to), _weight(w){};
  Neuron *getPostSynaptic() { return this->_destination; }
  Neuron *getPreSynaptic() { return this->_origin; }
  void propagate();
  void alterWeight(double weight);
  double getWeight() { return this->_weight; }

private:
  Neuron *_origin = nullptr;
  Neuron *_destination = nullptr;
  double _weight = 0.0;
  double _last_weight = -1.0;
};
```

- Greatly simplifies sending messages
```cpp

void Neuron::send_messages_in_group() {

  for (const auto synapse : getPostSynaptic()) {
    synapse->propagate();
  }

  lg.log_group_neuron_state(
      INFO,
      "(%d) Neuron %d reached activation threshold, entering refractory phase",
      this->group->get_id(), this->id);

  this->refractory();
}

```

- Rewrote helper functions like `add_random_neighbors` and `has_neighbor` to be `random_synapses` and `has_synaptic_connection` to accommodate the new data structure

```cpp
// signatures
bool has_synaptic_connection(Neuron *from_neuron, Neuron *to_neuron);
void random_synapses(vector<NeuronGroup *> groups, int number_neighbors);
```

<details>
<summary>Definitions here </summary>
<br>

```cpp
void random_synapses(vector<NeuronGroup *> groups, int number_neighbors) {
  int i = 0;
  while (i < number_neighbors) {

    // Get random neurons
    Neuron *from = get_random_neuron(groups);
    Neuron *to = get_random_neuron(groups, false);

    // check for self connections
    if (from == to) {
      continue;
    }
    if (has_synaptic_connection(from, to)) {
      continue;
    }

    from->add_neighbor(to, weight_function());
    i++;
  }
}

bool has_synaptic_connection(Neuron *from_neuron, Neuron *to_neuron) {
  auto pPostsynaptic = from_neuron->getPostSynaptic();
  auto pPresynaptic = from_neuron->getPresynaptic();

  if (find_if(pPostsynaptic.begin(), pPostsynaptic.end(),
              [to_neuron](Synapse *syn) {
                return syn->getPostSynaptic() == to_neuron;
              }) == pPostsynaptic.end()) {

    return false;
  } else if (find_if(pPresynaptic.begin(), pPresynaptic.end(),
                     [to_neuron](Synapse *syn) {
                       return syn->getPreSynaptic() == to_neuron;
                     }) == pPresynaptic.end()) {
    return false;
  } else {
    return true;
  }
}

```

</details>

### 📌 Update 4-10
**New addtions:**

- Graphing seems like its working exactly as it should!
    - [Group 1 Neuron 2](./images/4101.png)
    - [Group 1 Neuron 6](./images/4102.png)
    - [Group 3 Neuron 4](./images/4103.png) 
    - [Group 4 Neuron 4](./images/4104.png) 

**New Decay Function**

- Retroactive decay approach rather than decay helper thread

```cpp
void Neuron::retroactive_decay(double from, double to, double tau,
                               double v_rest) {

  // Catches the first decay for this neuron (initialized to -1)
  if (from < 0) {
    this->last_decay = to;
    return;
  }

  double decay_time_step = 2e-3;
  Message_t message_decay_type = Decay;
  double first_decay = from;
  double i;

  for (i = first_decay; i < to; i += decay_time_step) {
    double decay_value = (this->membrane_potential - v_rest) / tau;
    // catch decays that are negative or really small and ignore them
    if (decay_value < 0 || decay_value < 0.0001) {
      continue;
    }
    this->update_potential(-decay_value);
    lg.add_data(this->get_group()->get_id(), this->get_id(),
                this->membrane_potential, i, this->get_type(),
                message_decay_type, this);
  }
  // reset the last decay
  this->last_decay = i;
}
```



### 📌 Update 4-1
**New addtions:**

- Graphs not have legends for (S)timulus (N)eighborActivation (D)ecay (R)efractory

    - [Input Neuron Group 1 Neuron 4 Run 1711992931](./images/group_1neuron_4.png)
    - [Input Neuron Group 1 Neuron 3 Run 1711992931](./images/group_1neuron_3.png)
    - [Input Neuron Group 3 Neuron 5 Run 1711994996](./images/group_3neuron_5.png) ( has not incoming connections)
    - [Input Neuron Group 3 Neuron 3 Run 1711994996](./images/group_3neuron_3.png) ( seems like the decay function doens't run as often as it should? )

- Input neuron subclass with (`run_in_group` and `send_messages_in_group` are `virtual` in base class)

```cpp
extern double INPUT_PROB_SUCCESS;

class InputNeuron : public Neuron {

};
```

- Input neurons are mixed in with all other neurons
- `dynamic_cast` is used to access derived class methods during group run

```cpp

//... in NeuronGroup::group_run()
if (neuron->is_activated()) {

lg.log_group_neuron_state(DEBUG2, "Running (%d) Neuron (%d)",
                          this->get_id(), neuron->get_id());

if (neuron->get_type() == Input) {
  InputNeuron *neuron = dynamic_cast<InputNeuron *>(neuron);
}
// Run neuron
neuron->run_in_group();
}
```

- Poisson Procces for input neuron
    - Input Neuron run process is
        1. Check refractory status (ignore any input if still in refractory period)
        2. Recieve input if poisson success
            - Determined by `INPUT_PROB_SUCCESS` set by `["neuron"]["poisson_prop_of_success"]` in toml configuration file
        3. Check activaiton threshold and fire if above.

- Maximum edges function to ensure runtime configuration is possible

```cpp
int maximum_edges() {
  // for an undirected graph there are n(n-1) / 2 edges
  //
  // Since our connections are only allowed to go one way and self loops are forbidden 
  // the network is essentially an undirected graph.
  //
  // input neurons can only have outgoing edges 
  // the parameter NUMBER_NEURONS represents the total number of neurons
  // then n_t = NUMBER_NEURONS
  // n_i = NUMBER_INPUT_NEURONS
  //
  // the max edges are the total number of maximum edges minus the number of
  // edges that would be possible in a undirected graph of only the input
  // neurons maximum_edges = n_t(n_t) / 2 - n_i(n_i-1) / 2

  int n_i = NUMBER_INPUT_NEURONS;
  int n_t = NUMBER_NEURONS;

  // always even so division is fine
  int edges_lost_to_input = n_i * (n_i - 1) / 2;
  int max_edges = (n_t * (n_t - 1) / 2) - edges_lost_to_input;

  return max_edges;

```


### 📌 Update 3-19
**New addtions:**
- Decay logic now includes $\tau$ and $V_{rest}$
- Refractory period does not use `usleep` but instead is stored in data member `refractory_start` and any messages with the timestamp `refractory_start + REFRACTORY_DURATION` are ignored. 
- Neurons enter refractory period even if they have no neighbors
- Added `data_mutex` to ensure all data points are added with the correct information to the `log_data` vector

- Graphs now show activation, refractory periods, and decay


### 📌 Update 3-18
**New addtions:**
- Neuron types 
    - `Input` type fully functional, Output not
    - Restrictions on `add_random_neighbors` function for `Input` neurons to add outgoing connections only
- Run-time configuration setting for number of input neurons
- Shows Neuron type on plot 
- Log neuron type

- Example output with `DEBUG4`

    - [Example Output 11](./logs/example_output_11)

    - [Corresponding Log](./logs/1710775791.log)

<details>
<summary>Plot example</summary>
<br>

[Example Plot](./plotting/group_1neuron_2.png)

</details>


### 📌 Update 3-15
**New addtions:**
- .toml file for configuring multiple run-time options
- Specify other .toml files as command line arguement

``` 
build/ex2 <filename> 
```

<details>
<summary>.toml_file</summary>
<br>

- Base_config is regenerated if it isn't found in the directory
- Able to change run parameters without re-compiling.
- Easy to implement more run-time options
- Uses the [toml++](https://github.com/marzer/tomlplusplus) library for file parsing


```toml
[neuron]

# number of neurons
neuron_count = 6

# number of groups
group_count = 2

# number of connections
edge_count = 4

# in milliseconds
wait_time = 100

# integer
wait_loops = 5

# value each neuron is initialized with
initial_membrane_potential = -55.0

# minimum potential at which a neuron will fire
activation_threshold = -55.0

# value that each neuron is set to after firing
refractory_membrane_potential = -70.0

[debug]
# Options are
# INFO
# DEBUG
# DEBUG2
# DEBUG3
# DEBUG4
level = "INFO"

[decay]
# Value that each neuron decays every wait_time * wait_loops
value = 1.0

[random]
# options are 'time' for using the current time, or an integer (as a string e.g. "1")
seed = "time"

[runtime_vars]
# in seconds
runtime = 20
```


</details>

### 📌 Update 3-14
**New addtions:**
- New messaging structure for `NeuronGroups`
- Additional mutex variables for logging and messaging 
- Log file for Neuron Groups
- Neurons decay over time
- Plotting with matplotlib

<details>
<summary>Plotting</summary>
<br>

- Basic plotting with `matplotlib` 
- `.pngs` are in `group_id_neuron_id.png` format
- Images are generated in `plotting/`

</details>
<details>
<summary>Neuron potential decay</summary>
<br>

- Works on a separate thread, using a mutex to protect the neuron `membrane_potential` across threads

- Neurons decay based on `DECAY_VALUE` constant

```cpp

void decay_neurons(vector<NeuronGroup *> *groups) {
  vector<Neuron *> neuron_vec;

  // make a vector of all available neurons
  for (const auto &group : *groups) {
    for (const auto &neuron : group->get_neruon_vector()) {
      neuron_vec.push_back(neuron);
    }
  }

  while (::active) {

    for (int i = 1; i <= WAIT_INCREMENT; i++) {
      lg.log_value(DEBUG3, "decay_neurons waiting: %d", i);
      usleep(WAIT_TIME);
    }

    for (auto neuron : neuron_vec) {
      neuron->decay();
    }
  }
}

```


- Neuron Decay member function:

```cpp

double Neuron::decay() {
  pthread_mutex_lock(&mutex);
  this->membrane_potential -= DECAY_VALUE;

  lg.add_data(this->get_group()->get_id(), this->get_id(),
              this->membrane_potential);

  pthread_mutex_unlock(&mutex);

  lg.log_group_neuron_value(
      DEBUG2, "(%d) Neuron %d decaying. Membrane potential now %f",
      this->get_group()->get_id(), this->get_id(), this->get_potential());
  return this->membrane_potential;
}
```

</details>

<details>
<summary>Example Output 10 (this is long) </summary>
<br>

- This output is for a runtime of 60 seconds, with 5 second wait time on both the messager thread and neuron group threads
- `LogLevel` is `DEBUG3`
- There are 2 Neuron groups and 4 total neurons

```
Running build/ex2
./build/ex2

Adding Neurons
----------------

[1710434170:856328] ❶  Adding Group 1
[1710434170:856332] ⓘ  Group 1
[1710434170:856336] ⓘ  (1) Neuron 1 added: excitatory
[1710434170:856339] ⓘ  (1) Neuron 2 added: excitatory
[1710434170:856341] ❶  Adding Group 2
[1710434170:856342] ⓘ  Group 2
[1710434170:856343] ⓘ  (2) Neuron 1 added: excitatory
[1710434170:856344] ⓘ  (2) Neuron 2 added: excitatory

Adding Random Edges
======================

[1710434170:856348] ❸           (2) Neuron 2 has no outgoing connections
[1710434170:856349] ❸           (2) Neuron 2 has no incoming connections
[1710434170:856349] ❸           (1) Neuron 1 has no outgoing connections
[1710434170:856350] ❸           (1) Neuron 1 has no incoming connections
[1710434170:856352] ⓘ  Edge from Neuron 2 to Neuron 1 added.
[1710434170:856355] ❷  Neuron 2 added to the _presynaptic map of Neuron 
[1710434170:856357] ❷        (2) Neuron 2 is connected to:
[1710434170:856358] ❷           (1) Neuron 1
[1710434170:856358] ❸           (2) Neuron 2 has no incoming connections
[1710434170:856359] ❸           (1) Neuron 1 has no outgoing connections
[1710434170:856360] ❷        (1) Neuron 1 has connections from:
[1710434170:856361] ❷          (2) Neuron 2
[1710434170:856362] ❶  has_neighbor: Neuron 2 is already connected to Neuron 1
[1710434170:856363] ❸           (1) Neuron 1 has no outgoing connections
[1710434170:856363] ❷        (1) Neuron 1 has connections from:
[1710434170:856364] ❷          (2) Neuron 2
[1710434170:856365] ❸           (1) Neuron 2 has no outgoing connections
[1710434170:856365] ❸           (1) Neuron 2 has no incoming connections
[1710434170:856366] ⓘ  Edge from Neuron 1 to Neuron 2 added.
[1710434170:856368] ❷  Neuron 1 added to the _presynaptic map of Neuron 
[1710434170:856368] ❷        (2) Neuron 2 is connected to:
[1710434170:856369] ❷           (1) Neuron 1
[1710434170:856370] ❸           (2) Neuron 2 has no incoming connections
[1710434170:856371] ❷        (1) Neuron 1 is connected to:
[1710434170:856371] ❷           (1) Neuron 2
[1710434170:856372] ❷        (1) Neuron 1 has connections from:
[1710434170:856373] ❷          (2) Neuron 2
[1710434170:856373] ❶  has_neighbor: Neuron 2 is already connected to Neuron 1
[1710434170:856374] ❸           (1) Neuron 2 has no outgoing connections
[1710434170:856375] ❷        (1) Neuron 2 has connections from:
[1710434170:856375] ❷          (1) Neuron 1
[1710434170:856376] ❷        (2) Neuron 2 is connected to:
[1710434170:856377] ❷           (1) Neuron 1
[1710434170:856377] ❸           (2) Neuron 2 has no incoming connections
[1710434170:856378] ⓘ  Edge from Neuron 2 to Neuron 2 added.
[1710434170:856379] ❷  Neuron 2 added to the _presynaptic map of Neuron 

[1710434170:856381] ❶  Neuron Group 1 (2 neurons)
========================================================
[1710434170:856382] ❶     (1) Neuron 1
[1710434170:856383] ❷        (1) Neuron 1 is connected to:
[1710434170:856383] ❷           (1) Neuron 2
[1710434170:856384] ❷        (1) Neuron 1 has connections from:
[1710434170:856385] ❷          (2) Neuron 2
[1710434170:856385] ❶     (1) Neuron 2
[1710434170:856386] ❷        (1) Neuron 2 is connected to:
[1710434170:856387] ❷           (2) Neuron 2
[1710434170:856387] ❷        (1) Neuron 2 has connections from:
[1710434170:856388] ❷          (1) Neuron 1


[1710434170:856389] ❶  Neuron Group 2 (2 neurons)
========================================================
[1710434170:856390] ❶     (2) Neuron 1
[1710434170:856390] ❸           (2) Neuron 1 has no outgoing connections
[1710434170:856391] ❸           (2) Neuron 1 has no incoming connections
[1710434170:856392] ❶     (2) Neuron 2
[1710434170:856392] ❷        (2) Neuron 2 is connected to:
[1710434170:856393] ❷           (1) Neuron 1
[1710434170:856394] ❷        (2) Neuron 2 has connections from:
[1710434170:856394] ❷          (1) Neuron 2

[1710434170:856437] ❸  Message: 0.000000 1 1 1.000000
[1710434170:856439] ❸  Message: 0.000000 1 2 2.000000
[1710434170:856461] ❸  Message: 0.000000 2 1 1.000000
[1710434170:856463] ❸  Message: 0.000000 2 2 2.000000
[1710434170:856576] ❸  send_messages waiting: 1
[1710434170:856724] ⓘ  Group 2 running
[1710434170:856731] ❷  Checking activation:(2) Neuron 1 is inactive
[1710434170:856731] ⓘ  Group 1 running
[1710434170:856733] ❷  Checking activation:(2) Neuron 2 is inactive
[1710434170:856737] ❶  Group 2 pausing
[1710434170:856737] ❷  Checking activation:(1) Neuron 1 is inactive
[1710434170:856738] ❸  Group 2 waiting: 1
[1710434170:856740] ❷  Checking activation:(1) Neuron 2 is inactive
[1710434170:856741] ❶  Group 1 pausing
[1710434170:856743] ❸  Group 1 waiting: 1
[1710434171:856779] ❸  send_messages waiting: 2
[1710434171:856877] ❸  Group 2 waiting: 2
[1710434171:856877] ❸  Group 1 waiting: 2
[1710434172:856992] ❸  send_messages waiting: 3
[1710434172:857038] ❸  Group 1 waiting: 3
[1710434172:857038] ❸  Group 2 waiting: 3
[1710434173:857168] ❸  send_messages waiting: 4
[1710434173:857220] ❸  Group 1 waiting: 4
[1710434173:857220] ❸  Group 2 waiting: 4
[1710434174:857337] ❸  send_messages waiting: 5
[1710434174:857393] ❸  Group 1 waiting: 5
[1710434174:857399] ❸  Group 2 waiting: 5
[1710434175:857532] ❷  Adding Message: 0.000000 1 1 1.000000
[1710434175:857553] ❷  Adding Message: 0.000000 1 2 2.000000
[1710434175:857558] ❷  Adding Message: 0.000000 2 1 1.000000
[1710434175:857563] ❷  Adding Message: 0.000000 2 2 2.000000
[1710434175:857566] ❸  send_messages waiting: 1
[1710434175:857602] ❶  Group 2 resuming
[1710434175:857632] ❷  Checking activation:(2) Neuron 1 is active
[1710434175:857635] ❷  Running (2) Neuron (1)
[1710434175:857602] ❶  Group 1 resuming
[1710434175:857648] ❷  Checking activation:(1) Neuron 1 is active
[1710434175:857652] ❷  Running (1) Neuron (1)
[1710434175:857652] ⓘ  (2) Neuron 1 is activated, accumulated equal to -54.000000
[1710434175:857664] ❶  No additional messages for (2) Neuron 1
[1710434175:857667] ⓘ  Group 2: Neuron 1 does not have any neighbors
[1710434175:857671] ❷  Checking activation:(2) Neuron 2 is active
[1710434175:857673] ❷  Running (2) Neuron (2)
[1710434175:857683] ⓘ  (1) Neuron 1 is activated, accumulated equal to -54.000000
[1710434175:857695] ❶  No additional messages for (1) Neuron 1
[1710434175:857702] ⓘ  Group 1: Neuron 1 is sending a mesage to Group 1: Neuron 2
[1710434175:857710] ❶  Accumulated for Group 1: Neuron 1 is -54.000000
[1710434175:857714] ⓘ  (2) Neuron 2 is activated, accumulated equal to -53.000000
[1710434175:857716] ❶  Weight for Group 1: Neuron 1 to Group 1: Neuron 2 is 0.242887
[1710434175:857721] ❶  No additional messages for (2) Neuron 2
[1710434175:857722] ❶  Group 1: Neuron 1 modifier is -
[1710434175:857728] ⓘ  Group 2: Neuron 2 is sending a mesage to Group 1: Neuron 1
[1710434175:857732] ⓘ  Message from Group 1: Neuron 1 to Group 1: Neuron 2 is 13.115886
[1710434175:857735] ❶  Accumulated for Group 2: Neuron 2 is -53.000000
[1710434175:857737] ⓘ  Neuron 1 fired, entering refractory phase
[1710434175:857743] ❶  Weight for Group 2: Neuron 2 to Group 1: Neuron 1 is 0.277775
[1710434175:857748] ❶  Group 2: Neuron 2 modifier is -
[1710434175:857754] ⓘ  Message from Group 2: Neuron 2 to Group 1: Neuron 1 is 14.722060
[1710434175:857759] ⓘ  Neuron 2 fired, entering refractory phase
[1710434175:857771] ⓘ  Neuron 1 portential set to -70.0000
[1710434175:857774] ⓘ  Neuron 2 portential set to -70.0000
[1710434175:859910] ⓘ  Neuron 1 completed refractory phase, running
[1710434175:859937] ❷  Checking activation:(1) Neuron 2 is active
[1710434175:859910] ⓘ  Neuron 2 completed refractory phase, running
[1710434175:859955] ❶  Group 2 pausing
[1710434175:859958] ❸  Group 2 waiting: 1
[1710434175:859940] ❷  Running (1) Neuron (2)
[1710434175:859982] ⓘ  (1) Neuron 2 is activated, accumulated equal to -53.000000
[1710434175:859989] ⓘ  (1) Neuron 2 is activated, accumulated equal to -39.884114
[1710434175:859997] ❶  No additional messages for (1) Neuron 2
[1710434175:860119] ⓘ  Group 1: Neuron 2 is sending a mesage to Group 2: Neuron 2
[1710434175:860127] ❶  Accumulated for Group 1: Neuron 2 is -39.884114
[1710434175:860133] ❶  Weight for Group 1: Neuron 2 to Group 2: Neuron 2 is 0.512932
[1710434175:860137] ❶  Group 1: Neuron 2 modifier is -
[1710434175:860142] ⓘ  Message from Group 1: Neuron 2 to Group 2: Neuron 2 is 20.457854
[1710434175:860146] ⓘ  Neuron 1 fired, entering refractory phase
[1710434175:860154] ⓘ  Neuron 2 portential set to -70.0000
[1710434175:862348] ⓘ  Neuron 1 completed refractory phase, running
[1710434175:862362] ❶  Group 1 pausing
[1710434175:862365] ❸  Group 1 waiting: 1
[1710434176:857708] ❸  send_messages waiting: 2
[1710434176:860234] ❸  Group 2 waiting: 2
[1710434176:862616] ❸  Group 1 waiting: 2
[1710434177:857965] ❸  send_messages waiting: 3
[1710434177:860476] ❸  Group 2 waiting: 3
[1710434177:862902] ❸  Group 1 waiting: 3
[1710434178:858145] ❸  send_messages waiting: 4
[1710434178:860751] ❸  Group 2 waiting: 4
[1710434178:863141] ❸  Group 1 waiting: 4
[1710434179:858383] ❸  send_messages waiting: 5
[1710434179:860999] ❸  Group 2 waiting: 5
[1710434179:863328] ❸  Group 1 waiting: 5
[1710434180:858634] ❷  Adding Message: 0.000000 1 1 1.000000
[1710434180:858652] ❷  Adding Message: 0.000000 1 2 2.000000
[1710434180:858656] ❷  Adding Message: 0.000000 2 1 1.000000
[1710434180:858659] ❷  Adding Message: 0.000000 2 2 2.000000
[1710434180:858661] ❸  send_messages waiting: 1
[1710434180:861218] ❶  Group 2 resuming
[1710434180:861235] ❷  Checking activation:(2) Neuron 1 is active
[1710434180:861237] ❷  Running (2) Neuron (1)
[1710434180:861250] ⓘ  (2) Neuron 1 is activated, accumulated equal to -53.000000
[1710434180:861253] ❶  No additional messages for (2) Neuron 1
[1710434180:861255] ⓘ  Group 2: Neuron 1 does not have any neighbors
[1710434180:861258] ❷  Checking activation:(2) Neuron 2 is active
[1710434180:861260] ❷  Running (2) Neuron (2)
[1710434180:861264] ⓘ  (2) Neuron 2 is activated, accumulated equal to -49.542146
[1710434180:861267] ⓘ  (2) Neuron 2 is activated, accumulated equal to -47.542146
[1710434180:861269] ❶  No additional messages for (2) Neuron 2
[1710434180:861273] ⓘ  Group 2: Neuron 2 is sending a mesage to Group 1: Neuron 1
[1710434180:861278] ❶  Accumulated for Group 2: Neuron 2 is -47.542146
[1710434180:861282] ❶  Weight for Group 2: Neuron 2 to Group 1: Neuron 1 is 0.277775
[1710434180:861285] ❶  Group 2: Neuron 2 modifier is -
[1710434180:861289] ⓘ  Message from Group 2: Neuron 2 to Group 1: Neuron 1 is 13.206006
[1710434180:861293] ⓘ  Neuron 2 fired, entering refractory phase
[1710434180:861300] ⓘ  Neuron 2 portential set to -70.0000
[1710434180:863488] ⓘ  Neuron 2 completed refractory phase, running
[1710434180:863505] ❶  Group 2 pausing
[1710434180:863507] ❸  Group 2 waiting: 1
[1710434180:863488] ❶  Group 1 resuming
[1710434180:863516] ❷  Checking activation:(1) Neuron 1 is active
[1710434180:863519] ❷  Running (1) Neuron (1)
[1710434180:863531] ⓘ  (1) Neuron 1 is activated, accumulated equal to -55.277940
[1710434180:863538] ⓘ  (1) Neuron 1 is activated, accumulated equal to -54.277940
[1710434180:863542] ⓘ  (1) Neuron 1 is activated, accumulated equal to -41.071935
[1710434180:863543] ❶  No additional messages for (1) Neuron 1
[1710434180:863547] ⓘ  Group 1: Neuron 1 is sending a mesage to Group 1: Neuron 2
[1710434180:863552] ❶  Accumulated for Group 1: Neuron 1 is -41.071935
[1710434180:863555] ❶  Weight for Group 1: Neuron 1 to Group 1: Neuron 2 is 0.242887
[1710434180:863558] ❶  Group 1: Neuron 1 modifier is -
[1710434180:863561] ⓘ  Message from Group 1: Neuron 1 to Group 1: Neuron 2 is 9.975830
[1710434180:863563] ⓘ  Neuron 1 fired, entering refractory phase
[1710434180:863569] ⓘ  Neuron 1 portential set to -70.0000
[1710434180:865786] ⓘ  Neuron 1 completed refractory phase, running
[1710434180:865806] ❷  Checking activation:(1) Neuron 2 is active
[1710434180:865809] ❷  Running (1) Neuron (2)
[1710434180:865928] ⓘ  (1) Neuron 2 is activated, accumulated equal to -68.000000
[1710434180:865934] ⓘ  (1) Neuron 2 is activated, accumulated equal to -58.024170
[1710434180:865936] ❶  No additional messages for (1) Neuron 2
[1710434180:865941] ⓘ  Membrane potential for Group 1: Neuron 2 is below the threshold, not firing
[1710434180:865944] ❶  Group 1 pausing
[1710434180:865946] ❸  Group 1 waiting: 1
[1710434181:858876] ❸  send_messages waiting: 2
[1710434181:863689] ❸  Group 2 waiting: 2
[1710434181:866209] ❸  Group 1 waiting: 2
[1710434182:859051] ❸  send_messages waiting: 3
[1710434182:863907] ❸  Group 2 waiting: 3
[1710434182:866485] ❸  Group 1 waiting: 3
[1710434183:859325] ❸  send_messages waiting: 4
[1710434183:864209] ❸  Group 2 waiting: 4
[1710434183:866660] ❸  Group 1 waiting: 4
[1710434184:859489] ❸  send_messages waiting: 5
[1710434184:864433] ❸  Group 2 waiting: 5
[1710434184:866867] ❸  Group 1 waiting: 5
[1710434185:859666] ❷  Adding Message: 0.000000 1 1 1.000000
[1710434185:859687] ❷  Adding Message: 0.000000 1 2 2.000000
[1710434185:859692] ❷  Adding Message: 0.000000 2 1 1.000000
[1710434185:859696] ❷  Adding Message: 0.000000 2 2 2.000000
[1710434185:859699] ❸  send_messages waiting: 1
[1710434185:864718] ❶  Group 2 resuming
[1710434185:864739] ❷  Checking activation:(2) Neuron 1 is active
[1710434185:864742] ❷  Running (2) Neuron (1)
[1710434185:864758] ⓘ  (2) Neuron 1 is activated, accumulated equal to -52.000000
[1710434185:864763] ❶  No additional messages for (2) Neuron 1
[1710434185:864766] ⓘ  Group 2: Neuron 1 does not have any neighbors
[1710434185:864769] ❷  Checking activation:(2) Neuron 2 is active
[1710434185:864771] ❷  Running (2) Neuron (2)
[1710434185:864776] ⓘ  (2) Neuron 2 is activated, accumulated equal to -68.000000
[1710434185:864778] ❶  No additional messages for (2) Neuron 2
[1710434185:864784] ⓘ  Membrane potential for Group 2: Neuron 2 is below the threshold, not firing
[1710434185:864786] ❶  Group 2 pausing
[1710434185:864788] ❸  Group 2 waiting: 1
[1710434185:867064] ❶  Group 1 resuming
[1710434185:867085] ❷  Checking activation:(1) Neuron 1 is active
[1710434185:867088] ❷  Running (1) Neuron (1)
[1710434185:867103] ⓘ  (1) Neuron 1 is activated, accumulated equal to -69.000000
[1710434185:867107] ❶  No additional messages for (1) Neuron 1
[1710434185:867111] ⓘ  Membrane potential for Group 1: Neuron 1 is below the threshold, not firing
[1710434185:867115] ❷  Checking activation:(1) Neuron 2 is active
[1710434185:867117] ❷  Running (1) Neuron (2)
[1710434185:867122] ⓘ  (1) Neuron 2 is activated, accumulated equal to -56.024170
[1710434185:867168] ❶  No additional messages for (1) Neuron 2
[1710434185:867171] ⓘ  Membrane potential for Group 1: Neuron 2 is below the threshold, not firing
[1710434185:867173] ❶  Group 1 pausing
[1710434185:867175] ❸  Group 1 waiting: 1
[1710434186:859959] ❸  send_messages waiting: 2
[1710434186:865016] ❸  Group 2 waiting: 2
[1710434186:867440] ❸  Group 1 waiting: 2
[1710434187:860164] ❸  send_messages waiting: 3
[1710434187:865283] ❸  Group 2 waiting: 3
[1710434187:867638] ❸  Group 1 waiting: 3
[1710434188:860324] ❸  send_messages waiting: 4
[1710434188:865484] ❸  Group 2 waiting: 4
[1710434188:867894] ❸  Group 1 waiting: 4
[1710434189:860581] ❸  send_messages waiting: 5
[1710434189:865713] ❸  Group 2 waiting: 5
[1710434189:868112] ❸  Group 1 waiting: 5
[1710434190:860865] ❷  Adding Message: 0.000000 1 1 1.000000
[1710434190:860878] ❷  Adding Message: 0.000000 1 2 2.000000
[1710434190:860881] ❷  Adding Message: 0.000000 2 1 1.000000
[1710434190:860884] ❷  Adding Message: 0.000000 2 2 2.000000
[1710434190:860885] ❸  send_messages waiting: 1
[1710434190:865927] ❶  Group 2 resuming
[1710434190:865943] ❷  Checking activation:(2) Neuron 1 is active
[1710434190:865945] ❷  Running (2) Neuron (1)
[1710434190:865957] ⓘ  (2) Neuron 1 is activated, accumulated equal to -51.000000
[1710434190:866042] ❶  No additional messages for (2) Neuron 1
[1710434190:866045] ⓘ  Group 2: Neuron 1 does not have any neighbors
[1710434190:866047] ❷  Checking activation:(2) Neuron 2 is active
[1710434190:866049] ❷  Running (2) Neuron (2)
[1710434190:866053] ⓘ  (2) Neuron 2 is activated, accumulated equal to -66.000000
[1710434190:866055] ❶  No additional messages for (2) Neuron 2
[1710434190:866057] ⓘ  Membrane potential for Group 2: Neuron 2 is below the threshold, not firing
[1710434190:866059] ❶  Group 2 pausing
[1710434190:866061] ❸  Group 2 waiting: 1
[1710434190:868314] ❶  Group 1 resuming
[1710434190:868329] ❷  Checking activation:(1) Neuron 1 is active
[1710434190:868331] ❷  Running (1) Neuron (1)
[1710434190:868343] ⓘ  (1) Neuron 1 is activated, accumulated equal to -68.000000
[1710434190:868346] ❶  No additional messages for (1) Neuron 1
[1710434190:868349] ⓘ  Membrane potential for Group 1: Neuron 1 is below the threshold, not firing
[1710434190:868352] ❷  Checking activation:(1) Neuron 2 is active
[1710434190:868353] ❷  Running (1) Neuron (2)
[1710434190:868357] ⓘ  (1) Neuron 2 is activated, accumulated equal to -54.024170
[1710434190:868359] ❶  No additional messages for (1) Neuron 2
[1710434190:868363] ⓘ  Group 1: Neuron 2 is sending a mesage to Group 2: Neuron 2
[1710434190:868367] ❶  Accumulated for Group 1: Neuron 2 is -54.024170
[1710434190:868371] ❶  Weight for Group 1: Neuron 2 to Group 2: Neuron 2 is 0.512932
[1710434190:868373] ❶  Group 1: Neuron 2 modifier is -
[1710434190:868377] ⓘ  Message from Group 1: Neuron 2 to Group 2: Neuron 2 is 27.710747
[1710434190:868380] ⓘ  Neuron 1 fired, entering refractory phase
[1710434190:868387] ⓘ  Neuron 2 portential set to -70.0000
[1710434190:870595] ⓘ  Neuron 1 completed refractory phase, running
[1710434190:870610] ❶  Group 1 pausing
[1710434190:870612] ❸  Group 1 waiting: 1
[1710434191:861120] ❸  send_messages waiting: 2
[1710434191:866315] ❸  Group 2 waiting: 2
[1710434191:870795] ❸  Group 1 waiting: 2
[1710434192:861415] ❸  send_messages waiting: 3
[1710434192:866552] ❸  Group 2 waiting: 3
[1710434192:871086] ❸  Group 1 waiting: 3
[1710434193:861603] ❸  send_messages waiting: 4
[1710434193:866779] ❸  Group 2 waiting: 4
[1710434193:871389] ❸  Group 1 waiting: 4
[1710434194:861852] ❸  send_messages waiting: 5
[1710434194:867006] ❸  Group 2 waiting: 5
[1710434194:871662] ❸  Group 1 waiting: 5
[1710434195:862014] ❷  Adding Message: 0.000000 1 1 1.000000
[1710434195:862027] ❷  Adding Message: 0.000000 1 2 2.000000
[1710434195:862030] ❷  Adding Message: 0.000000 2 1 1.000000
[1710434195:862033] ❷  Adding Message: 0.000000 2 2 2.000000
[1710434195:862035] ❸  send_messages waiting: 1
[1710434195:867271] ❶  Group 2 resuming
[1710434195:867286] ❷  Checking activation:(2) Neuron 1 is active
[1710434195:867289] ❷  Running (2) Neuron (1)
[1710434195:867300] ⓘ  (2) Neuron 1 is activated, accumulated equal to -50.000000
[1710434195:867303] ❶  No additional messages for (2) Neuron 1
[1710434195:867306] ⓘ  Group 2: Neuron 1 does not have any neighbors
[1710434195:867308] ❷  Checking activation:(2) Neuron 2 is active
[1710434195:867309] ❷  Running (2) Neuron (2)
[1710434195:867313] ⓘ  (2) Neuron 2 is activated, accumulated equal to -38.289253
[1710434195:867317] ⓘ  (2) Neuron 2 is activated, accumulated equal to -36.289253
[1710434195:867318] ❶  No additional messages for (2) Neuron 2
[1710434195:867322] ⓘ  Group 2: Neuron 2 is sending a mesage to Group 1: Neuron 1
[1710434195:867326] ❶  Accumulated for Group 2: Neuron 2 is -36.289253
[1710434195:867330] ❶  Weight for Group 2: Neuron 2 to Group 1: Neuron 1 is 0.277775
[1710434195:867333] ❶  Group 2: Neuron 2 modifier is -
[1710434195:867337] ⓘ  Message from Group 2: Neuron 2 to Group 1: Neuron 1 is 10.080237
[1710434195:867340] ⓘ  Neuron 2 fired, entering refractory phase
[1710434195:867346] ⓘ  Neuron 2 portential set to -70.0000
[1710434195:869543] ⓘ  Neuron 2 completed refractory phase, running
[1710434195:869646] ❶  Group 2 pausing
[1710434195:869649] ❸  Group 2 waiting: 1
[1710434195:871955] ❶  Group 1 resuming
[1710434195:871975] ❷  Checking activation:(1) Neuron 1 is active
[1710434195:871978] ❷  Running (1) Neuron (1)
[1710434195:871994] ⓘ  (1) Neuron 1 is activated, accumulated equal to -67.000000
[1710434195:872001] ⓘ  (1) Neuron 1 is activated, accumulated equal to -56.919763
[1710434195:872003] ❶  No additional messages for (1) Neuron 1
[1710434195:872007] ⓘ  Membrane potential for Group 1: Neuron 1 is below the threshold, not firing
[1710434195:872010] ❷  Checking activation:(1) Neuron 2 is active
[1710434195:872012] ❷  Running (1) Neuron (2)
[1710434195:872017] ⓘ  (1) Neuron 2 is activated, accumulated equal to -68.000000
[1710434195:872019] ❶  No additional messages for (1) Neuron 2
[1710434195:872021] ⓘ  Membrane potential for Group 1: Neuron 2 is below the threshold, not firing
[1710434195:872023] ❶  Group 1 pausing
[1710434195:872026] ❸  Group 1 waiting: 1
[1710434196:862217] ❸  send_messages waiting: 2
[1710434196:869823] ❸  Group 2 waiting: 2
[1710434196:872208] ❸  Group 1 waiting: 2
[1710434197:862475] ❸  send_messages waiting: 3
[1710434197:870022] ❸  Group 2 waiting: 3
[1710434197:872406] ❸  Group 1 waiting: 3
[1710434198:862749] ❸  send_messages waiting: 4
[1710434198:870257] ❸  Group 2 waiting: 4
[1710434198:872694] ❸  Group 1 waiting: 4
[1710434199:863002] ❸  send_messages waiting: 5
[1710434199:870471] ❸  Group 2 waiting: 5
[1710434199:873000] ❸  Group 1 waiting: 5
[1710434200:863252] ❷  Adding Message: 0.000000 1 1 1.000000
[1710434200:863267] ❷  Adding Message: 0.000000 1 2 2.000000
[1710434200:863271] ❷  Adding Message: 0.000000 2 1 1.000000
[1710434200:863274] ❷  Adding Message: 0.000000 2 2 2.000000
[1710434200:863276] ❸  send_messages waiting: 1
[1710434200:870669] ❶  Group 2 resuming
[1710434200:870684] ❷  Checking activation:(2) Neuron 1 is active
[1710434200:870686] ❷  Running (2) Neuron (1)
[1710434200:870698] ⓘ  (2) Neuron 1 is activated, accumulated equal to -49.000000
[1710434200:870701] ❶  No additional messages for (2) Neuron 1
[1710434200:870703] ⓘ  Group 2: Neuron 1 does not have any neighbors
[1710434200:870705] ❷  Checking activation:(2) Neuron 2 is active
[1710434200:870707] ❷  Running (2) Neuron (2)
[1710434200:870710] ⓘ  (2) Neuron 2 is activated, accumulated equal to -68.000000
[1710434200:870712] ❶  No additional messages for (2) Neuron 2
[1710434200:870715] ⓘ  Membrane potential for Group 2: Neuron 2 is below the threshold, not firing
[1710434200:870716] ❶  Group 2 pausing
[1710434200:870718] ❸  Group 2 waiting: 1
[1710434200:873224] ❶  Group 1 resuming
[1710434200:873241] ❷  Checking activation:(1) Neuron 1 is active
[1710434200:873243] ❷  Running (1) Neuron (1)
[1710434200:873256] ⓘ  (1) Neuron 1 is activated, accumulated equal to -55.919763
[1710434200:873259] ❶  No additional messages for (1) Neuron 1
[1710434200:873262] ⓘ  Membrane potential for Group 1: Neuron 1 is below the threshold, not firing
[1710434200:873264] ❷  Checking activation:(1) Neuron 2 is active
[1710434200:873266] ❷  Running (1) Neuron (2)
[1710434200:873270] ⓘ  (1) Neuron 2 is activated, accumulated equal to -66.000000
[1710434200:873271] ❶  No additional messages for (1) Neuron 2
[1710434200:873273] ⓘ  Membrane potential for Group 1: Neuron 2 is below the threshold, not firing
[1710434200:873275] ❶  Group 1 pausing
[1710434200:873277] ❸  Group 1 waiting: 1
[1710434201:863423] ❸  send_messages waiting: 2
[1710434201:870937] ❸  Group 2 waiting: 2
[1710434201:873517] ❸  Group 1 waiting: 2
[1710434202:863586] ❸  send_messages waiting: 3
[1710434202:871135] ❸  Group 2 waiting: 3
[1710434202:873726] ❸  Group 1 waiting: 3
[1710434203:863738] ❸  send_messages waiting: 4
[1710434203:871302] ❸  Group 2 waiting: 4
[1710434203:873958] ❸  Group 1 waiting: 4
[1710434204:863976] ❸  send_messages waiting: 5
[1710434204:871565] ❸  Group 2 waiting: 5
[1710434204:874244] ❸  Group 1 waiting: 5
[1710434205:864218] ❷  Adding Message: 0.000000 1 1 1.000000
[1710434205:864233] ❷  Adding Message: 0.000000 1 2 2.000000
[1710434205:864237] ❷  Adding Message: 0.000000 2 1 1.000000
[1710434205:864239] ❷  Adding Message: 0.000000 2 2 2.000000
[1710434205:864241] ❸  send_messages waiting: 1
[1710434205:871853] ❶  Group 2 resuming
[1710434205:871870] ❷  Checking activation:(2) Neuron 1 is active
[1710434205:871872] ❷  Running (2) Neuron (1)
[1710434205:871885] ⓘ  (2) Neuron 1 is activated, accumulated equal to -48.000000
[1710434205:871888] ❶  No additional messages for (2) Neuron 1
[1710434205:871891] ⓘ  Group 2: Neuron 1 does not have any neighbors
[1710434205:871893] ❷  Checking activation:(2) Neuron 2 is active
[1710434205:871895] ❷  Running (2) Neuron (2)
[1710434205:871898] ⓘ  (2) Neuron 2 is activated, accumulated equal to -66.000000
[1710434205:871933] ❶  No additional messages for (2) Neuron 2
[1710434205:871936] ⓘ  Membrane potential for Group 2: Neuron 2 is below the threshold, not firing
[1710434205:871938] ❶  Group 2 pausing
[1710434205:871940] ❸  Group 2 waiting: 1
[1710434205:874589] ❶  Group 1 resuming
[1710434205:874606] ❷  Checking activation:(1) Neuron 1 is active
[1710434205:874608] ❷  Running (1) Neuron (1)
[1710434205:874621] ⓘ  (1) Neuron 1 is activated, accumulated equal to -54.919763
[1710434205:874624] ❶  No additional messages for (1) Neuron 1
[1710434205:874629] ⓘ  Group 1: Neuron 1 is sending a mesage to Group 1: Neuron 2
[1710434205:874634] ❶  Accumulated for Group 1: Neuron 1 is -54.919763
[1710434205:874639] ❶  Weight for Group 1: Neuron 1 to Group 1: Neuron 2 is 0.242887
[1710434205:874641] ❶  Group 1: Neuron 1 modifier is -
[1710434205:874645] ⓘ  Message from Group 1: Neuron 1 to Group 1: Neuron 2 is 13.339284
[1710434205:874649] ⓘ  Neuron 1 fired, entering refractory phase
[1710434205:874656] ⓘ  Neuron 1 portential set to -70.0000
[1710434205:876832] ⓘ  Neuron 1 completed refractory phase, running
[1710434205:876852] ❷  Checking activation:(1) Neuron 2 is active
[1710434205:876855] ❷  Running (1) Neuron (2)
[1710434205:876870] ⓘ  (1) Neuron 2 is activated, accumulated equal to -64.000000
[1710434205:876877] ⓘ  (1) Neuron 2 is activated, accumulated equal to -50.660716
[1710434205:876879] ❶  No additional messages for (1) Neuron 2
[1710434205:876885] ⓘ  Group 1: Neuron 2 is sending a mesage to Group 2: Neuron 2
[1710434205:876891] ❶  Accumulated for Group 1: Neuron 2 is -50.660716
[1710434205:876895] ❶  Weight for Group 1: Neuron 2 to Group 2: Neuron 2 is 0.512932
[1710434205:876899] ❶  Group 1: Neuron 2 modifier is -
[1710434205:876903] ⓘ  Message from Group 1: Neuron 2 to Group 2: Neuron 2 is 25.985522
[1710434205:876907] ⓘ  Neuron 1 fired, entering refractory phase
[1710434205:876915] ⓘ  Neuron 2 portential set to -70.0000
[1710434205:879194] ⓘ  Neuron 1 completed refractory phase, running
[1710434205:879215] ❶  Group 1 pausing
[1710434205:879219] ❸  Group 1 waiting: 1
[1710434206:864459] ❸  send_messages waiting: 2
[1710434206:872104] ❸  Group 2 waiting: 2
[1710434206:879443] ❸  Group 1 waiting: 2
[1710434207:864648] ❸  send_messages waiting: 3
[1710434207:872282] ❸  Group 2 waiting: 3
[1710434207:879720] ❸  Group 1 waiting: 3
[1710434208:864730] ❸  send_messages waiting: 4
[1710434208:872440] ❸  Group 2 waiting: 4
[1710434208:879907] ❸  Group 1 waiting: 4
[1710434209:864909] ❸  send_messages waiting: 5
[1710434209:872702] ❸  Group 2 waiting: 5
[1710434209:880110] ❸  Group 1 waiting: 5
[1710434210:865204] ❷  Adding Message: 0.000000 1 1 1.000000
[1710434210:865230] ❷  Adding Message: 0.000000 1 2 2.000000
[1710434210:865237] ❷  Adding Message: 0.000000 2 1 1.000000
[1710434210:865243] ❷  Adding Message: 0.000000 2 2 2.000000
[1710434210:865247] ❸  send_messages waiting: 1
[1710434210:872923] ❶  Group 2 resuming
[1710434210:872946] ❷  Checking activation:(2) Neuron 1 is active
[1710434210:872949] ❷  Running (2) Neuron (1)
[1710434210:873087] ⓘ  (2) Neuron 1 is activated, accumulated equal to -47.000000
[1710434210:873092] ❶  No additional messages for (2) Neuron 1
[1710434210:873096] ⓘ  Group 2: Neuron 1 does not have any neighbors
[1710434210:873100] ❷  Checking activation:(2) Neuron 2 is active
[1710434210:873102] ❷  Running (2) Neuron (2)
[1710434210:873109] ⓘ  (2) Neuron 2 is activated, accumulated equal to -40.014478
[1710434210:873114] ⓘ  (2) Neuron 2 is activated, accumulated equal to -38.014478
[1710434210:873116] ❶  No additional messages for (2) Neuron 2
[1710434210:873122] ⓘ  Group 2: Neuron 2 is sending a mesage to Group 1: Neuron 1
[1710434210:873129] ❶  Accumulated for Group 2: Neuron 2 is -38.014478
[1710434210:873135] ❶  Weight for Group 2: Neuron 2 to Group 1: Neuron 1 is 0.277775
[1710434210:873140] ❶  Group 2: Neuron 2 modifier is -
[1710434210:873146] ⓘ  Message from Group 2: Neuron 2 to Group 1: Neuron 1 is 10.559461
[1710434210:873151] ⓘ  Neuron 2 fired, entering refractory phase
[1710434210:873161] ⓘ  Neuron 2 portential set to -70.0000
[1710434210:875358] ⓘ  Neuron 2 completed refractory phase, running
[1710434210:875378] ❶  Group 2 pausing
[1710434210:875381] ❸  Group 2 waiting: 1
[1710434210:880429] ❶  Group 1 resuming
[1710434210:880450] ❷  Checking activation:(1) Neuron 1 is active
[1710434210:880454] ❷  Running (1) Neuron (1)
[1710434210:880472] ⓘ  (1) Neuron 1 is activated, accumulated equal to -69.000000
[1710434210:880481] ⓘ  (1) Neuron 1 is activated, accumulated equal to -58.440539
[1710434210:880485] ❶  No additional messages for (1) Neuron 1
[1710434210:880489] ⓘ  Membrane potential for Group 1: Neuron 1 is below the threshold, not firing
[1710434210:880492] ❷  Checking activation:(1) Neuron 2 is active
[1710434210:880495] ❷  Running (1) Neuron (2)
[1710434210:880501] ⓘ  (1) Neuron 2 is activated, accumulated equal to -68.000000
[1710434210:880503] ❶  No additional messages for (1) Neuron 2
[1710434210:880506] ⓘ  Membrane potential for Group 1: Neuron 2 is below the threshold, not firing
[1710434210:880509] ❶  Group 1 pausing
[1710434210:880512] ❸  Group 1 waiting: 1
[1710434211:865361] ❸  send_messages waiting: 2
[1710434211:875500] ❸  Group 2 waiting: 2
[1710434211:880673] ❸  Group 1 waiting: 2
[1710434212:865522] ❸  send_messages waiting: 3
[1710434212:875698] ❸  Group 2 waiting: 3
[1710434212:880865] ❸  Group 1 waiting: 3
[1710434213:865695] ❸  send_messages waiting: 4
[1710434213:875899] ❸  Group 2 waiting: 4
[1710434213:881012] ❸  Group 1 waiting: 4
[1710434214:865835] ❸  send_messages waiting: 5
[1710434214:876022] ❸  Group 2 waiting: 5
[1710434214:881242] ❸  Group 1 waiting: 5
[1710434215:866066] ❷  Adding Message: 0.000000 1 1 1.000000
[1710434215:866087] ❷  Adding Message: 0.000000 1 2 2.000000
[1710434215:866092] ❷  Adding Message: 0.000000 2 1 1.000000
[1710434215:866096] ❷  Adding Message: 0.000000 2 2 2.000000
[1710434215:866099] ❸  send_messages waiting: 1
[1710434215:876134] ❶  Group 2 resuming
[1710434215:876152] ❷  Checking activation:(2) Neuron 1 is active
[1710434215:876155] ❷  Running (2) Neuron (1)
[1710434215:876169] ⓘ  (2) Neuron 1 is activated, accumulated equal to -46.000000
[1710434215:876173] ❶  No additional messages for (2) Neuron 1
[1710434215:876175] ⓘ  Group 2: Neuron 1 does not have any neighbors
[1710434215:876178] ❷  Checking activation:(2) Neuron 2 is active
[1710434215:876180] ❷  Running (2) Neuron (2)
[1710434215:876184] ⓘ  (2) Neuron 2 is activated, accumulated equal to -68.000000
[1710434215:876186] ❶  No additional messages for (2) Neuron 2
[1710434215:876188] ⓘ  Membrane potential for Group 2: Neuron 2 is below the threshold, not firing
[1710434215:876190] ❶  Group 2 pausing
[1710434215:876192] ❸  Group 2 waiting: 1
[1710434215:881519] ❶  Group 1 resuming
[1710434215:881544] ❷  Checking activation:(1) Neuron 1 is active
[1710434215:881548] ❷  Running (1) Neuron (1)
[1710434215:881568] ⓘ  (1) Neuron 1 is activated, accumulated equal to -57.440539
[1710434215:881693] ❶  No additional messages for (1) Neuron 1
[1710434215:881699] ⓘ  Membrane potential for Group 1: Neuron 1 is below the threshold, not firing
[1710434215:881703] ❷  Checking activation:(1) Neuron 2 is active
[1710434215:881706] ❷  Running (1) Neuron (2)
[1710434215:881714] ⓘ  (1) Neuron 2 is activated, accumulated equal to -66.000000
[1710434215:881717] ❶  No additional messages for (1) Neuron 2
[1710434215:881720] ⓘ  Membrane potential for Group 1: Neuron 2 is below the threshold, not firing
[1710434215:881723] ❶  Group 1 pausing
[1710434215:881726] ❸  Group 1 waiting: 1
[1710434216:866213] ❸  send_messages waiting: 2
[1710434216:876458] ❸  Group 2 waiting: 2
[1710434216:881980] ❸  Group 1 waiting: 2
[1710434217:866475] ❸  send_messages waiting: 3
[1710434217:876648] ❸  Group 2 waiting: 3
[1710434217:882162] ❸  Group 1 waiting: 3
[1710434218:866616] ❸  send_messages waiting: 4
[1710434218:876855] ❸  Group 2 waiting: 4
[1710434218:882410] ❸  Group 1 waiting: 4
[1710434219:866819] ❸  send_messages waiting: 5
[1710434219:877076] ❸  Group 2 waiting: 5
[1710434219:882623] ❸  Group 1 waiting: 5
[1710434220:867011] ❷  Adding Message: 0.000000 1 1 1.000000
[1710434220:867030] ❷  Adding Message: 0.000000 1 2 2.000000
[1710434220:867035] ❷  Adding Message: 0.000000 2 1 1.000000
[1710434220:867038] ❷  Adding Message: 0.000000 2 2 2.000000
[1710434220:867041] ❸  send_messages waiting: 1
[1710434220:877354] ❶  Group 2 resuming
[1710434220:877372] ❷  Checking activation:(2) Neuron 1 is active
[1710434220:877375] ❷  Running (2) Neuron (1)
[1710434220:877389] ⓘ  (2) Neuron 1 is activated, accumulated equal to -45.000000
[1710434220:877393] ❶  No additional messages for (2) Neuron 1
[1710434220:877396] ⓘ  Group 2: Neuron 1 does not have any neighbors
[1710434220:877399] ❷  Checking activation:(2) Neuron 2 is active
[1710434220:877400] ❷  Running (2) Neuron (2)
[1710434220:877404] ⓘ  (2) Neuron 2 is activated, accumulated equal to -66.000000
[1710434220:877406] ❶  No additional messages for (2) Neuron 2
[1710434220:877410] ⓘ  Membrane potential for Group 2: Neuron 2 is below the threshold, not firing
[1710434220:877412] ❶  Group 2 pausing
[1710434220:877414] ❸  Group 2 waiting: 1
[1710434220:882900] ❶  Group 1 resuming
[1710434220:882916] ❷  Checking activation:(1) Neuron 1 is active
[1710434220:882918] ❷  Running (1) Neuron (1)
[1710434220:882932] ⓘ  (1) Neuron 1 is activated, accumulated equal to -56.440539
[1710434220:882935] ❶  No additional messages for (1) Neuron 1
[1710434220:882938] ⓘ  Membrane potential for Group 1: Neuron 1 is below the threshold, not firing
[1710434220:882941] ❷  Checking activation:(1) Neuron 2 is active
[1710434220:882942] ❷  Running (1) Neuron (2)
[1710434220:882946] ⓘ  (1) Neuron 2 is activated, accumulated equal to -64.000000
[1710434220:882948] ❶  No additional messages for (1) Neuron 2
[1710434220:882950] ⓘ  Membrane potential for Group 1: Neuron 2 is below the threshold, not firing
[1710434220:882952] ❶  Group 1 pausing
[1710434220:882953] ❸  Group 1 waiting: 1
[1710434221:867264] ❸  send_messages waiting: 2
[1710434221:877621] ❸  Group 2 waiting: 2
[1710434221:883180] ❸  Group 1 waiting: 2
[1710434222:867440] ❸  send_messages waiting: 3
[1710434222:877887] ❸  Group 2 waiting: 3
[1710434222:883455] ❸  Group 1 waiting: 3
[1710434223:867603] ❸  send_messages waiting: 4
[1710434223:878158] ❸  Group 2 waiting: 4
[1710434223:883756] ❸  Group 1 waiting: 4
[1710434224:867788] ❸  send_messages waiting: 5
[1710434224:878420] ❸  Group 2 waiting: 5
[1710434224:883916] ❸  Group 1 waiting: 5
[1710434225:867973] ❷  Adding Message: 0.000000 1 1 1.000000
[1710434225:867994] ❷  Adding Message: 0.000000 1 2 2.000000
[1710434225:867999] ❷  Adding Message: 0.000000 2 1 1.000000
[1710434225:868003] ❷  Adding Message: 0.000000 2 2 2.000000
[1710434225:868005] ❸  send_messages waiting: 1
[1710434225:878729] ❶  Group 2 resuming
[1710434225:878747] ❷  Checking activation:(2) Neuron 1 is active
[1710434225:878853] ❷  Running (2) Neuron (1)
[1710434225:878868] ⓘ  (2) Neuron 1 is activated, accumulated equal to -44.000000
[1710434225:878871] ❶  No additional messages for (2) Neuron 1
[1710434225:878874] ⓘ  Group 2: Neuron 1 does not have any neighbors
[1710434225:878877] ❷  Checking activation:(2) Neuron 2 is active
[1710434225:878879] ❷  Running (2) Neuron (2)
[1710434225:878883] ⓘ  (2) Neuron 2 is activated, accumulated equal to -64.000000
[1710434225:878885] ❶  No additional messages for (2) Neuron 2
[1710434225:878888] ⓘ  Membrane potential for Group 2: Neuron 2 is below the threshold, not firing
[1710434225:878890] ❶  Group 2 pausing
[1710434225:878892] ❸  Group 2 waiting: 1
[1710434225:884156] ❶  Group 1 resuming
[1710434225:884172] ❷  Checking activation:(1) Neuron 1 is active
[1710434225:884174] ❷  Running (1) Neuron (1)
[1710434225:884188] ⓘ  (1) Neuron 1 is activated, accumulated equal to -55.440539
[1710434225:884191] ❶  No additional messages for (1) Neuron 1
[1710434225:884194] ⓘ  Membrane potential for Group 1: Neuron 1 is below the threshold, not firing
[1710434225:884197] ❷  Checking activation:(1) Neuron 2 is active
[1710434225:884199] ❷  Running (1) Neuron (2)
[1710434225:884203] ⓘ  (1) Neuron 2 is activated, accumulated equal to -62.000000
[1710434225:884205] ❶  No additional messages for (1) Neuron 2
[1710434225:884207] ⓘ  Membrane potential for Group 1: Neuron 2 is below the threshold, not firing
[1710434225:884208] ❶  Group 1 pausing
[1710434225:884210] ❸  Group 1 waiting: 1
[1710434226:868182] ❸  send_messages waiting: 2
[1710434226:879136] ❸  Group 2 waiting: 2
[1710434226:884455] ❸  Group 1 waiting: 2
[1710434227:868400] ❸  send_messages waiting: 3
[1710434227:879352] ❸  Group 2 waiting: 3
[1710434227:884718] ❸  Group 1 waiting: 3
[1710434228:868580] ❸  send_messages waiting: 4
[1710434228:879655] ❸  Group 2 waiting: 4
[1710434228:884904] ❸  Group 1 waiting: 4
[1710434229:868797] ❸  send_messages waiting: 5
[1710434229:879855] ❸  Group 2 waiting: 5
[1710434229:885211] ❸  Group 1 waiting: 5
[1710434230:856652] ⓘ  Writing data to file...

[1710434230:857030] ❶  Neuron Group 1 (2 neurons)
========================================================
[1710434230:857033] ❶     (1) Neuron 1
[1710434230:857036] ❷        (1) Neuron 1 is connected to:
[1710434230:857038] ❷           (1) Neuron 2
[1710434230:857040] ❷        (1) Neuron 1 has connections from:
[1710434230:857042] ❷          (2) Neuron 2
[1710434230:857043] ❶     (1) Neuron 2
[1710434230:857045] ❷        (1) Neuron 2 is connected to:
[1710434230:857046] ❷           (2) Neuron 2
[1710434230:857047] ❷        (1) Neuron 2 has connections from:
[1710434230:857048] ❷          (1) Neuron 1


[1710434230:857050] ❶  Neuron Group 2 (2 neurons)
========================================================
[1710434230:857052] ❶     (2) Neuron 1
[1710434230:857053] ❸           (2) Neuron 1 has no outgoing connections
[1710434230:857054] ❸           (2) Neuron 1 has no incoming connections
[1710434230:857056] ❶     (2) Neuron 2
[1710434230:857057] ❷        (2) Neuron 2 is connected to:
[1710434230:857058] ❷           (1) Neuron 1
[1710434230:857059] ❷        (2) Neuron 2 has connections from:
[1710434230:857060] ❷          (1) Neuron 2

[1710434230:857063] ❶  Deleteing Group 1 Neuron 1
[1710434230:857067] ❶  Deleteing Group 1 Neuron 2
[1710434230:857069] ❶  Deleteing Group 2 Neuron 1
[1710434230:857070] ❶  Deleteing Group 2 Neuron 2
```




</details>

<details>
<summary>Log file</summary>
<br>

- This log is for a runtime of 60 seconds, with 5 second wait time on both the messager thread and neuron group threads
- Example:
    - Columns are `group_id neuron_id time potential`

```

2 1 1710432472.534000 -54.000000
1 1 1710432472.533860 -54.000000
2 2 1710432472.534050 -53.000000
1 2 1710432472.533950 -53.000000
1 2 1710432472.535760 -39.884114
2 1 1710432477.544780 -53.000000
2 2 1710432472.560560 -49.542146
2 2 1710432477.544840 -47.542146
1 1 1710432472.536580 -55.277940
1 1 1710432477.544610 -54.277940
1 1 1710432477.571260 -41.071935
1 2 1710432477.544720 -68.000000
1 2 1710432477.595780 -58.024170
2 1 1710432482.556490 -52.000000
2 2 1710432482.556560 -68.000000
1 1 1710432482.556310 -69.000000
1 2 1710432482.556430 -56.024170
2 1 1710432487.568480 -51.000000
2 2 1710432487.568560 -66.000000
1 1 1710432487.568280 -68.000000
1 2 1710432487.568410 -54.024170
2 1 1710432492.580400 -50.000000
2 2 1710432487.643090 -38.289253
2 2 1710432492.580610 -36.289253
1 1 1710432492.579830 -67.000000
1 1 1710432492.631490 -56.919763
1 2 1710432492.580100 -68.000000
2 1 1710432497.592390 -49.000000
2 2 1710432497.592500 -68.000000
1 1 1710432497.591940 -55.919763
1 2 1710432497.592240 -66.000000
2 1 1710432502.606400 -48.000000
2 2 1710432502.606490 -66.000000
1 1 1710432502.606110 -54.919763
1 2 1710432502.606280 -64.000000
1 2 1710432502.704360 -50.660716
2 1 1710432507.618390 -47.000000
2 2 1710432502.727340 -40.014478
2 2 1710432507.618450 -38.014478
1 1 1710432507.618230 -69.000000
1 1 1710432507.699600 -58.440539
1 2 1710432507.618330 -68.000000
2 1 1710432512.630750 -46.000000
2 2 1710432512.630790 -68.000000
1 1 1710432512.630600 -57.440539
1 2 1710432512.630690 -66.000000
2 1 1710432517.642160 -45.000000
2 2 1710432517.642210 -66.000000
1 1 1710432517.641950 -56.440539
1 2 1710432517.642080 -64.000000
2 1 1710432522.653550 -44.000000
2 2 1710432522.653590 -64.000000
1 1 1710432522.653410 -55.440539
1 2 1710432522.653500 -62.000000
```

</details>
<details>
<summary>Messaging Structure</summary>
<br>

- The messaging structure works as follows:
    1. Messager thread starts and reads from file
    2. Messager thread sends messages to neurons and then waits `WAIT_TIME` `WAIT_INCREMENT` times
    3. Each `NeuronGroup` thread loops through its neurons and checks their activation status
        - if activated the neuron runs, exhausts its message queue, and sends messages to any neighbor neurons (by adding to each respective queue)
    4. After running through all its neurons, the `NeuronGroup` waits `WAIT_TIME` `WAIT_INCREMENT` times.
- The message queue is implemented as `std::list` because random access is not needed and popping from the front is neccessary.


- Main message function
```cpp

void send_messages(const vector<Message *> *messages) {

  while (::active) {
    for (int i = 1; i <= WAIT_INCREMENT; i++) {
      lg.log_value(DEBUG3, "send_messages waiting: %d", i);
      usleep(WAIT_TIME);
    }

    for (auto message : *messages) {

      lg.log_message(DEBUG2, "Adding Message: %f %d %d %f", message->timestamp,
                     message->target_neuron_group->get_id(),
                     message->target_neuron->get_id(), message->message);

      Message *message_copy =
          construct_message(message->message, message->target_neuron);
      message_copy->timestamp = lg.get_time_stamp();

      message->target_neuron->add_message(message_copy);
      message->target_neuron->activate();
    }
  }
  pthread_exit(NULL);
}
```

- Both the message function and the `run_group` function are controlled by the global variable `active`
    - This is deactivated based on the constant `RUN_TIME`;
```cpp
  // main_neuron_groups.cpp
  usleep(RUN_TIME);
  active = false;
```

- Constructing messages from a file input
    - Constructs a vector of dynamically allocated Messages (deallocated in `deallocate_message_vector`)
    - The message value corresponds directly the the value in the file but this could be changed easily.
    - In the future, the construct message from file could take a vector of `INPUT` neurons only.
```cpp

vector<Message *>
construct_message_vector_from_file(vector<NeuronGroup *> groups,
                                   const char *file_name) {
  vector<Neuron *> neuron_vec;
  vector<Message *> message_vector;

  // make a vector of all available neurons
  for (const auto &group : groups) {
    for (const auto &neuron : group->get_neruon_vector()) {
      neuron_vec.push_back(neuron);
    }
  }

  std::ifstream file(file_name);

  if (!file.is_open()) {
    lg.log(ERROR, "construct_message_vector_from_file: Unable to open file");
    return message_vector;
  }

  int number_neurons = neuron_vec.size();
  int data_read = 0;
  double value;

  while (!file.eof() && data_read < number_neurons) {
    file >> value;
    message_vector.push_back(construct_message(value, neuron_vec[data_read]));
    data_read++;
  }

  return message_vector;
}
```

</details>

### 📌 Update 3-12
**New addtions:**
- Basic `NeuronGroup` messaging.
- `NeuronGroup`s run in separate threads
- Messaging struct
- Changed Logging struct
- Changed logging output for readability
    - ⓘ  for `INFO`
    - ❶ for `DEBUG`
    - ❷ for `DEBUG2`
    - and so on for `DEBUG3` and `DEBUG4`

<details>
<summary>Logging Struct</summary>
<br>

```cpp 
typedef struct {
  int neuron_id;
  int group_id;
  double timestamp;
  double membrane_potentail;
} LogData;
```

</details>
<details>
<summary>Message Struct</summary>
<br>

```cpp
typedef struct {
  double message;
  Neuron *target_neuron;
  NeuronGroup *target_neuron_group;
  double timestamp;
} Message;
```

</details>
<details>
<summary>Example Output 9 (summarized)</summary>
<br>

```
// adding neurons 
// random edges

// new output

[1710275464:285432] ❶  Neuron Group 1 (4 neurons)
========================================================
[1710275464:285434] ❶     (1) Neuron 1
[1710275464:285435] ❶     (1) Neuron 2
[1710275464:285437] ❶     (1) Neuron 3
[1710275464:285438] ❶     (1) Neuron 4
[1710275464:285439] ❷        (1) Neuron 4 has connections from:
[1710275464:285441] ❷          (2) Neuron 2


[1710275464:285442] ❶  Neuron Group 2 (4 neurons)
========================================================
[1710275464:285443] ❶     (2) Neuron 1
[1710275464:285444] ❶     (2) Neuron 2
[1710275464:285446] ❷        (2) Neuron 2 is connected to:
[1710275464:285447] ❷           (1) Neuron 4
[1710275464:285448] ❶     (2) Neuron 3
[1710275464:285449] ❶     (2) Neuron 4

[1710275464:285453] ❷  Thread started for group 1
[1710275464:285503] ❷  Thread started for group 2
[1710275464:285653] ⓘ  Group 2 running
[1710275464:285661] ❷  Checking activation:(2) Neuron 1 is inactive
[1710275464:285663] ❷  Checking activation:(2) Neuron 2 is inactive
[1710275464:285664] ❷  Checking activation:(2) Neuron 3 is inactive
[1710275464:285665] ❷  Checking activation:(2) Neuron 4 is inactive
[1710275464:285667] ❷  Checking activation:(2) Neuron 1 is inactive
[1710275464:285668] ❷  Checking activation:(2) Neuron 2 is inactive
[1710275464:285669] ❷  Checking activation:(2) Neuron 3 is inactive
[1710275464:285670] ❷  Checking activation:(2) Neuron 4 is inactive
[1710275464:285672] ❷  Checking activation:(2) Neuron 1 is inactive
[1710275464:285673] ❷  Checking activation:(2) Neuron 2 is inactive
[1710275464:285674] ❷  Checking activation:(2) Neuron 3 is inactive
[1710275464:285675] ❷  Checking activation:(2) Neuron 4 is inactive
[1710275464:285722] ⓘ  Group 1 running
[1710275464:285729] ❷  Checking activation:(1) Neuron 1 is active
[1710275464:285731] ❷  Running (1) Neuron (1)
[1710275464:285742] ⓘ  (1) Neuron 1 is activated, accumulated equal to -45.000000
[1710275464:285745] ⓘ  Group 1: Neuron 1 does not have any neighbors
[1710275464:285747] ❷  Checking activation:(1) Neuron 2 is inactive
[1710275464:285748] ❷  Checking activation:(1) Neuron 3 is inactive
[1710275464:285750] ❷  Checking activation:(1) Neuron 4 is active
[1710275464:285751] ❷  Running (1) Neuron (4)
[1710275464:285753] ⓘ  (1) Neuron 4 is activated, accumulated equal to -45.000000
[1710275464:285755] ⓘ  Group 1: Neuron 4 does not have any neighbors
[1710275464:285757] ❷  Checking activation:(1) Neuron 1 is inactive
[1710275464:285758] ❷  Checking activation:(1) Neuron 2 is inactive
[1710275464:285759] ❷  Checking activation:(1) Neuron 3 is inactive
[1710275464:285760] ❷  Checking activation:(1) Neuron 4 is inactive
[1710275464:285762] ❷  Checking activation:(1) Neuron 1 is inactive
[1710275464:285763] ❷  Checking activation:(1) Neuron 2 is inactive
[1710275464:285764] ❷  Checking activation:(1) Neuron 3 is inactive
[1710275464:285765] ❷  Checking activation:(1) Neuron 4 is inactive

[1710275464:285986] ❶  Deleteing Group 1 Neuron 1
[1710275464:285990] ❶  Deleteing Group 1 Neuron 2
[1710275464:285992] ❶  Deleteing Group 1 Neuron 3
[1710275464:285994] ❶  Deleteing Group 1 Neuron 4
[1710275464:285998] ❶  Deleteing Group 2 Neuron 1
[1710275464:285999] ❶  Deleteing Group 2 Neuron 2
[1710275464:286001] ❶  Deleteing Group 2 Neuron 3
[1710275464:286003] ❶  Deleteing Group 2 Neuron 4
```
</details>

<details>
<summary>Example Output 9 (all ouput)</summary>
<br>

```
Adding Neurons
----------------

[1710275464:285392] ❶  Adding Group 1
[1710275464:285398] ⓘ  Group 1
[1710275464:285403] ⓘ  (1) Neuron 1 added: excitatory
[1710275464:285406] ⓘ  (1) Neuron 2 added: excitatory
[1710275464:285408] ⓘ  (1) Neuron 3 added: excitatory
[1710275464:285410] ⓘ  (1) Neuron 4 added: excitatory
[1710275464:285412] ❶  Adding Group 2
[1710275464:285412] ⓘ  Group 2
[1710275464:285414] ⓘ  (2) Neuron 1 added: excitatory
[1710275464:285415] ⓘ  (2) Neuron 2 added: inhibitory
[1710275464:285417] ⓘ  (2) Neuron 3 added: excitatory
[1710275464:285418] ⓘ  (2) Neuron 4 added: excitatory

Adding Random Edges
======================

[1710275464:285425] ⓘ  Edge from Neuron 2 to Neuron 4 added.
[1710275464:285430] ❷  Neuron 2 added to the _presynaptic map of Neuron 

[1710275464:285432] ❶  Neuron Group 1 (4 neurons)
========================================================
[1710275464:285434] ❶     (1) Neuron 1
[1710275464:285435] ❶     (1) Neuron 2
[1710275464:285437] ❶     (1) Neuron 3
[1710275464:285438] ❶     (1) Neuron 4
[1710275464:285439] ❷        (1) Neuron 4 has connections from:
[1710275464:285441] ❷          (2) Neuron 2


[1710275464:285442] ❶  Neuron Group 2 (4 neurons)
========================================================
[1710275464:285443] ❶     (2) Neuron 1
[1710275464:285444] ❶     (2) Neuron 2
[1710275464:285446] ❷        (2) Neuron 2 is connected to:
[1710275464:285447] ❷           (1) Neuron 4
[1710275464:285448] ❶     (2) Neuron 3
[1710275464:285449] ❶     (2) Neuron 4

[1710275464:285453] ❷  Thread started for group 1
[1710275464:285503] ❷  Thread started for group 2
[1710275464:285653] ⓘ  Group 2 running
[1710275464:285661] ❷  Checking activation:(2) Neuron 1 is inactive
[1710275464:285663] ❷  Checking activation:(2) Neuron 2 is inactive
[1710275464:285664] ❷  Checking activation:(2) Neuron 3 is inactive
[1710275464:285665] ❷  Checking activation:(2) Neuron 4 is inactive
[1710275464:285667] ❷  Checking activation:(2) Neuron 1 is inactive
[1710275464:285668] ❷  Checking activation:(2) Neuron 2 is inactive
[1710275464:285669] ❷  Checking activation:(2) Neuron 3 is inactive
[1710275464:285670] ❷  Checking activation:(2) Neuron 4 is inactive
[1710275464:285672] ❷  Checking activation:(2) Neuron 1 is inactive
[1710275464:285673] ❷  Checking activation:(2) Neuron 2 is inactive
[1710275464:285674] ❷  Checking activation:(2) Neuron 3 is inactive
[1710275464:285675] ❷  Checking activation:(2) Neuron 4 is inactive
[1710275464:285722] ⓘ  Group 1 running
[1710275464:285729] ❷  Checking activation:(1) Neuron 1 is active
[1710275464:285731] ❷  Running (1) Neuron (1)
[1710275464:285742] ⓘ  (1) Neuron 1 is activated, accumulated equal to -45.000000
[1710275464:285745] ⓘ  Group 1: Neuron 1 does not have any neighbors
[1710275464:285747] ❷  Checking activation:(1) Neuron 2 is inactive
[1710275464:285748] ❷  Checking activation:(1) Neuron 3 is inactive
[1710275464:285750] ❷  Checking activation:(1) Neuron 4 is active
[1710275464:285751] ❷  Running (1) Neuron (4)
[1710275464:285753] ⓘ  (1) Neuron 4 is activated, accumulated equal to -45.000000
[1710275464:285755] ⓘ  Group 1: Neuron 4 does not have any neighbors
[1710275464:285757] ❷  Checking activation:(1) Neuron 1 is inactive
[1710275464:285758] ❷  Checking activation:(1) Neuron 2 is inactive
[1710275464:285759] ❷  Checking activation:(1) Neuron 3 is inactive
[1710275464:285760] ❷  Checking activation:(1) Neuron 4 is inactive
[1710275464:285762] ❷  Checking activation:(1) Neuron 1 is inactive
[1710275464:285763] ❷  Checking activation:(1) Neuron 2 is inactive
[1710275464:285764] ❷  Checking activation:(1) Neuron 3 is inactive
[1710275464:285765] ❷  Checking activation:(1) Neuron 4 is inactive

[1710275464:285986] ❶  Deleteing Group 1 Neuron 1
[1710275464:285990] ❶  Deleteing Group 1 Neuron 2
[1710275464:285992] ❶  Deleteing Group 1 Neuron 3
[1710275464:285994] ❶  Deleteing Group 1 Neuron 4
[1710275464:285998] ❶  Deleteing Group 2 Neuron 1
[1710275464:285999] ❶  Deleteing Group 2 Neuron 2
[1710275464:286001] ❶  Deleteing Group 2 Neuron 3
[1710275464:286003] ❶  Deleteing Group 2 Neuron 4
```
</details>

### 📌 Update 3-11
**New addtions:**
- Neuron Group Class
    - Add intergroup and intragroup edges
    - New functions for adding neigbors between groups and within groups
    - New logging functions for reporting group values, state, and grouped neuron interactions
    - Not complete!
        - Unable to send messages
        - Some functions that are aimed to do that, but they are not tested at all
- Some new logging functionality
    - `NeuronGroup` logging; 
    - `DEBUG2` debug level
    - `NeuronGroup` versions of other logging
- Updated makefile
    - To build without groups use the `build1` target (and `run1` target to run)
    - To build with groups use the `build2` target (and `run2` target to run)

<details>
<summary>Neuron Group Class</summary>
<br>

``` cpp
class NeuronGroup {
  // NOT COMPLETE
private:
  vector<Neuron *> neurons;
  int id;
  pthread_t thread;

  // this variable should be read only from outside the class
  // Analagous to "value" in previous version
  int message;

public:
  NeuronGroup(int _id, int number_neurons);
  // might move neuron memory responsibilities to this class
  ~NeuronGroup();

  void *group_run();
  void start_thread() { pthread_create(&thread, NULL, thread_helper, this); }
  double get_message() const { return message; }
  double get_id() const { return id; }

  void set_message(double message);
  void print_group();
  int neuron_count();
  const vector<Neuron *> &get_neruon_vector();

  /*--------------------------------------------------------------*\
   *                  Thread helper:
   *    POSIX needs a void* (*)(void*) function signature
   *    This function allows us to use the run() member funciton
  \--------------------------------------------------------------*/
  static void *thread_helper(void *instance) {
    return ((NeuronGroup *)instance)->group_run();
  }
};
```

</details>

<details>
<summary>Example Output 8</summary>
<br>

- Debug level : `DEBUG2`

``` 
Running build/ex2
./build/ex2

Adding Neurons
----------------

[1710180962:736259] [Debug] Adding Group 1
[1710180962:736263] [Info] Group 1
[1710180962:736268] [Info] (1) Neuron 1 added: excitatory
[1710180962:736271] [Info] (1) Neuron 2 added: inhibitory
[1710180962:736273] [Info] (1) Neuron 3 added: excitatory
[1710180962:736275] [Debug] Adding Group 2
[1710180962:736275] [Info] Group 2
[1710180962:736277] [Info] (2) Neuron 1 added: excitatory
[1710180962:736278] [Info] (2) Neuron 2 added: excitatory
[1710180962:736279] [Info] (2) Neuron 3 added: excitatory

Adding Random Edges
======================

[1710180962:736284] [Debug2]          (2) Neuron 3 has no outgoing connections
[1710180962:736285] [Debug2]          (2) Neuron 3 has no incoming connections
[1710180962:736286] [Debug2]          (1) Neuron 2 has no outgoing connections
[1710180962:736287] [Debug2]          (1) Neuron 2 has no incoming connections
[1710180962:736289] [Info] Edge from Neuron 3 to Neuron 2 added.
[1710180962:736293] [Debug2] Neuron 3 added to the _presynaptic map of Neuron 
[1710180962:736294] [Debug2]          (1) Neuron 3 has no outgoing connections
[1710180962:736295] [Debug2]          (1) Neuron 3 has no incoming connections
[1710180962:736296] [Debug2]          (1) Neuron 2 has no outgoing connections
[1710180962:736297] [Debug2]       (1) Neuron 2 has connections from:
[1710180962:736298] [Debug2]         (2) Neuron 3
[1710180962:736299] [Info] Edge from Neuron 3 to Neuron 2 added.
[1710180962:736301] [Debug2] Neuron 3 added to the _presynaptic map of Neuron 
[1710180962:736302] [Debug2]          (2) Neuron 2 has no outgoing connections
[1710180962:736303] [Debug2]          (2) Neuron 2 has no incoming connections
[1710180962:736304] [Debug2]          (2) Neuron 1 has no outgoing connections
[1710180962:736304] [Debug2]          (2) Neuron 1 has no incoming connections
[1710180962:736305] [Info] Edge from Neuron 2 to Neuron 1 added.
[1710180962:736307] [Debug2] Neuron 2 added to the _presynaptic map of Neuron 
[1710180962:736308] [Debug2]          (1) Neuron 2 has no outgoing connections
[1710180962:736309] [Debug2]       (1) Neuron 2 has connections from:
[1710180962:736309] [Debug2]         (1) Neuron 3
[1710180962:736310] [Debug2]         (2) Neuron 3
[1710180962:736311] [Debug2]          (1) Neuron 1 has no outgoing connections
[1710180962:736312] [Debug2]          (1) Neuron 1 has no incoming connections
[1710180962:736313] [Info] Edge from Neuron 2 to Neuron 1 added.
[1710180962:736314] [Debug2] Neuron 2 added to the _presynaptic map of Neuron 
[1710180962:736315] [Debug2]          (1) Neuron 1 has no outgoing connections
[1710180962:736316] [Debug2]       (1) Neuron 1 has connections from:
[1710180962:736317] [Debug2]         (1) Neuron 2
[1710180962:736318] [Debug2]       (2) Neuron 3 is connected to:
[1710180962:736319] [Debug2]          (1) Neuron 2
[1710180962:736320] [Debug2]          (2) Neuron 3 has no incoming connections
[1710180962:736321] [Info] Edge from Neuron 1 to Neuron 3 added.
[1710180962:736322] [Debug2] Neuron 1 added to the _presynaptic map of Neuron 

[1710180962:736324] [Debug] Neuron Group 1 (3 neurons)
========================================================
[1710180962:736326] [Debug]    (1) Neuron 1
[1710180962:736327] [Debug2]       (1) Neuron 1 is connected to:
[1710180962:736327] [Debug2]          (2) Neuron 3
[1710180962:736328] [Debug2]       (1) Neuron 1 has connections from:
[1710180962:736329] [Debug2]         (1) Neuron 2
[1710180962:736330] [Debug]    (1) Neuron 2
[1710180962:736330] [Debug2]       (1) Neuron 2 is connected to:
[1710180962:736331] [Debug2]          (1) Neuron 1
[1710180962:736332] [Debug2]       (1) Neuron 2 has connections from:
[1710180962:736333] [Debug2]         (1) Neuron 3
[1710180962:736334] [Debug2]         (2) Neuron 3
[1710180962:736334] [Debug]    (1) Neuron 3
[1710180962:736335] [Debug2]       (1) Neuron 3 is connected to:
[1710180962:736336] [Debug2]          (1) Neuron 2
[1710180962:736337] [Debug2]          (1) Neuron 3 has no incoming connections


[1710180962:736338] [Debug] Neuron Group 2 (3 neurons)
========================================================
[1710180962:736361] [Debug]    (2) Neuron 1
[1710180962:736362] [Debug2]          (2) Neuron 1 has no outgoing connections
[1710180962:736363] [Debug2]       (2) Neuron 1 has connections from:
[1710180962:736364] [Debug2]         (2) Neuron 2
[1710180962:736364] [Debug]    (2) Neuron 2
[1710180962:736365] [Debug2]       (2) Neuron 2 is connected to:
[1710180962:736366] [Debug2]          (2) Neuron 1
[1710180962:736367] [Debug2]          (2) Neuron 2 has no incoming connections
[1710180962:736368] [Debug]    (2) Neuron 3
[1710180962:736368] [Debug2]       (2) Neuron 3 is connected to:
[1710180962:736369] [Debug2]          (1) Neuron 2
[1710180962:736370] [Debug2]       (2) Neuron 3 has connections from:
[1710180962:736371] [Debug2]         (1) Neuron 1

[1710180962:736372] [Debug] Deleteing Group 1 Neuron 1
[1710180962:736374] [Debug] Deleteing Group 1 Neuron 2
[1710180962:736375] [Debug] Deleteing Group 1 Neuron 3
[1710180962:736377] [Debug] Deleteing Group 2 Neuron 1
[1710180962:736378] [Debug] Deleteing Group 2 Neuron 2
[1710180962:736379] [Debug] Deleteing Group 2 Neuron 3
```
</details>

<details>
<summary>Example Output 7</summary>
<br>

``` 
Running build/ex2
./build/ex2

Adding Neurons
----------------

[1710180872:468797] [Debug] Adding Group 1
[1710180872:468800] [Info] Group 1
[1710180872:468803] [Info] (1) Neuron 1 added: excitatory
[1710180872:468805] [Info] (1) Neuron 2 added: excitatory
[1710180872:468807] [Info] (1) Neuron 3 added: excitatory
[1710180872:468808] [Debug] Adding Group 2
[1710180872:468808] [Info] Group 2
[1710180872:468809] [Info] (2) Neuron 1 added: excitatory
[1710180872:468810] [Info] (2) Neuron 2 added: inhibitory
[1710180872:468811] [Info] (2) Neuron 3 added: excitatory

Adding Random Edges
======================

[1710180872:468815] [Info] Edge from Neuron 3 to Neuron 1 added.
[1710180872:468821] [Info] Edge from Neuron 1 to Neuron 2 added.
[1710180872:468823] [Info] Edge from Neuron 2 to Neuron 2 added.
[1710180872:468827] [Info] Edge from Neuron 1 to Neuron 2 added.
[1710180872:468830] [Info] Edge from Neuron 2 to Neuron 3 added.

[1710180872:468832] [Debug] Neuron Group 1 (3 neurons)
========================================================
[1710180872:468832] [Debug]    (1) Neuron 1
[1710180872:468833] [Debug]    (1) Neuron 2
[1710180872:468835] [Debug]    (1) Neuron 3


[1710180872:468836] [Debug] Neuron Group 2 (3 neurons)
========================================================
[1710180872:468837] [Debug]    (2) Neuron 1
[1710180872:468839] [Debug]    (2) Neuron 2
[1710180872:468840] [Debug]    (2) Neuron 3

[1710180872:468841] [Debug] Deleteing Group 1 Neuron 1
[1710180872:468842] [Debug] Deleteing Group 1 Neuron 2
[1710180872:468843] [Debug] Deleteing Group 1 Neuron 3
[1710180872:468844] [Debug] Deleteing Group 2 Neuron 1
[1710180872:468845] [Debug] Deleteing Group 2 Neuron 2
[1710180872:468845] [Debug] Deleteing Group 2 Neuron 3
```

</details>

### 📌 Update 3-5
**New addtions:**
- Fully integrated `Log` class with Debug Level functionality. 
- Collect Neuron ID and Membrane Potential durring execution and write to a `<current_time>.log` file for later use in graphing potential vs time. 
    - Logs are stored in `./logs` 
- Swapped out bare data member refrences with `this->data_member` for readability
- Debug Levels:
    - level is set in `main.cpp:27`
```cpp
enum LogLevel { 
  DATA,
  ERROR,
  WARNING,
  INFO,
  DEBUG,
};
```



<details>
<summary>Log File Example</summary>
<br>

- Format is "time neuron_id memabrane_potential"
```
1709662312.633490 1 -55.000000
1709662317.948280 2 -55.000000
1709662314.449250 3 -55.000000
1709662314.452480 2 -106.695394
1709662314.455150 5 -90.316228
1709662314.457940 6 -92.203995
1709662314.158270 4 -55.000000
1709662321.825460 5 -90.316228
1709662320.091510 6 -92.203995
1709662318.345790 1 -55.000000
```
</details>

<details>
<summary> Example Output 6 </summary>
<br>

```
Adding Neurons
----------------

[1709787252:721647] [Info] Neuron 1 added: inhibitory
[1709787252:721652] [Info] Neuron 2 added: inhibitory
[1709787252:721654] [Info] Neuron 3 added: excitatory
[1709787252:721655] [Info] Neuron 4 added: excitatory
[1709787252:721657] [Info] Neuron 5 added: inhibitory
[1709787252:721659] [Info] Neuron 6 added: excitatory

Adding Random Edges
--------------------------

[1709787252:721663] [Info] Edge from Neuron 3 to Neuron 4 added.
[1709787252:721670] [Info] Edge from Neuron 4 to Neuron 5 added.
[1709787252:721673] [Info] Edge from Neuron 3 to Neuron 2 added.
[1709787252:721676] [Info] Edge from Neuron 1 to Neuron 5 added.
[1709787252:721680] [Info] Edge from Neuron 4 to Neuron 6 added.

[1709787252:721815] [Info] Neuron 1 is waiting
[1709787252:721907] [Info] Neuron 3 is waiting
[1709787252:722201] [Info] Neuron 2 is waiting
[1709787252:722283] [Info] Neuron 6 is waiting
[1709787252:722379] [Info] Neuron 5 is waiting
[1709787252:722471] [Info] Neuron 4 is waiting

Activate neuron ( or [-1] to quit )
 Neuron 1
 Neuron 2
 Neuron 3
 Neuron 4
 Neuron 5
 Neuron 6
Input: 1

[1709787256:21207] [Info] Neuron 1 is activated, accumulated equal to -55.0000
[1709787256:21345] [Info] Neuron 1 is sending a mesage to Neuron 5
[1709787256:21569] [Info] Message from Neuron 1 to Neuron 5 is -43.278355
[1709787256:21765] [Info] Neuron 5 is activated, accumulated equal to -98.2783
[1709787256:21869] [Info] Neuron 5 does not have any neighbors
[1709787256:22018] [Info] Neuron 5 is waiting
[1709787256:21929] [Info] Neuron 1 fired, entering refractory phase
[1709787256:22238] [Info] Neuron 1 portential set to -70.0000
[1709787256:24797] [Info] Neuron 1 completed refractory phase, running
[1709787256:24959] [Info] Neuron 1 is waiting
```

</details>

<details>

<summary> Log Class </summary>
<br>

```cpp
using std::vector;

enum LogLevel {
  DATA,
  ERROR,
  WARNING,
  INFO,
  DEBUG,
};

extern LogLevel level;

class Log {
public:
  // constructor

  void add_data(int id, double data);

  void write_data(const char *filesname = "./logs/%ld.log");

  void log(LogLevel level, const char *message, std::ostream &os = std::cout);

  void log_neuron_state(LogLevel level, const char *message, int id);

  void log_neuron_interaction(LogLevel level, const char *message, int id1,
                              int id2);
  void log_neuron_interaction(LogLevel level, const char *message, int id1,
                              int id2, double value);
  void log_neuron_value(LogLevel level, const char *message, int id,
                        double accumulated);
  void log_neuron_type(LogLevel level, const char *message, int id,
                       const char *type);

private:
  vector<double> time;
  vector<double> data;
  vector<int> id;
};
```

</details>

### 📌 Update 3-3 Part 2

**New addtions:**
- Working quit functionality on menu! 
- Memory deallocation working.
- Additional logging

<details>
<summary> Example Output 5 </summary>
<br>


``` text
Time format is |HH:MM:SS:mircroseconds|

Adding Neurons
----------------

|14:30:0:926336| Neuron 1 added (excitatory type)
|14:30:0:926380| Neuron 2 added (excitatory type)
|14:30:0:926383| Neuron 3 added (inhibitory type)

Adding Random Edges
--------------------------

|14:30:0:926387| Edge from Neuron 2 to Neuron 3 added
|14:30:0:926391| Neuron 2 added to _presynaptic of Neuron 3
|14:30:0:926416| Neuron 2 is connected to:
|14:30:0:926481| - Neuron3
|14:30:0:926487| Edge from Neuron 1 to Neuron 2 added
|14:30:0:926491| Neuron 1 added to _presynaptic of Neuron 2

|14:30:0:926609| Neuron 1 is waiting
|14:30:0:926647| Neuron 3 is waiting
|14:30:0:926870| Neuron 2 is waiting

Activate neuron ( or [-1] to quit )
 Neuron 1
 Neuron 2
 Neuron 3
Input: 1

|14:30:7:536435| Neuron 1 is activated, accumulated equal to -55
|14:30:7:536569| Neuron 1 is sending a message to Neuron 2
|14:30:7:536580| Accumulated value for Neuron 1 is -55
|14:30:7:536587| Weight for Neuron 1 to Neuron 2 is 0.517304
|14:30:7:536596| Neuron 1 modifier is -1
|14:30:7:536602| Message is 28.4517
|14:30:7:536788| Neuron 2 is activated, accumulated equal to -26.5483
|14:30:7:536944| Neuron 2 is sending a message to Neuron 3
|14:30:7:536964| Accumulated value for Neuron 2 is -26.5483
|14:30:7:536964| Neuron 1 fired, entering refractory phase
|14:30:7:537079| Neuron 1 potential set to -70
|14:30:7:536983| Weight for Neuron 2 to Neuron 3 is 0.352867
|14:30:7:537319| Neuron 2 modifier is -1
|14:30:7:537327| Message is 9.36803
|14:30:7:537535| Neuron 3 is activated, accumulated equal to -45.632
|14:30:7:537721| Neuron 3 does not have any neigbors!
|14:30:7:537742| Neuron 3 is waiting
|14:30:7:537744| Neuron 2 fired, entering refractory phase
|14:30:7:537842| Neuron 2 potential set to -70
|14:30:7:539698| Neuron 1 completed refractory phase, running
|14:30:7:539861| Neuron 1 is waiting
|14:30:7:540311| Neuron 2 completed refractory phase, running
|14:30:7:540440| Neuron 2 is waiting

Activate neuron ( or [-1] to quit )
 Neuron 1
 Neuron 2
 Neuron 3
Input: 3

|14:30:10:979676| Neuron 3 is activated, accumulated equal to -45.632
|14:30:10:979797| Neuron 3 does not have any neigbors!
|14:30:10:979805| Neuron 3 is waiting

Activate neuron ( or [-1] to quit )
 Neuron 1
 Neuron 2
 Neuron 3
Input: -1

|14:30:15:945951| Exiting...
|14:30:15:946038| Neuron 1 is exiting
|14:30:15:946054| Neuron 2 is exiting
|14:30:15:946085| Neuron 3 is exiting

Final Neuron Values
-------------------

|14:30:15:946269| Neuron 1: -70
|14:30:15:946316| Neuron 2: -70
|14:30:15:946354| Neuron 3: -45.632

Deallocation
------------

|14:30:15:946390| Deleting Neuron 1
|14:30:15:946397| Deleting Neuron 2
|14:30:15:946400| Deleting Neuron 3
```
</details>

<details>
<summary>Valgrind Output</summary>
<br>

- No Memory Leaks
```
==229124==
==229124== HEAP SUMMARY:
==229124==     in use at exit: 0 bytes in 0 blocks
==229124==   total heap usage: 54 allocs, 54 frees, 86,137 bytes allocated
==229124==
==229124== All heap blocks were freed -- no leaks are possible
==229124==
==229124== For lists of detected and suppressed errors, rerun with: -s
```
</details>

### 📌 Update 3-3

**New addtions:**
- Time stamps on logging messages

<details>
<summary> Example output 4 </summary>
<br>


``` text
Time format is |HH:MM:SS:mircroseconds|

Adding Neurons
----------------

|11:52:26:434641| Neuron 1 added (excitatory type)
|11:52:26:434669| Neuron 2 added (inhibitory type)
|11:52:26:434672| Neuron 3 added (excitatory type)
|11:52:26:434674| Neuron 4 added (inhibitory type)
|11:52:26:434676| Neuron 5 added (excitatory type)
|11:52:26:434679| Neuron 6 added (excitatory type)

Adding Random Edges
--------------------------

|11:52:26:434707| Edge from Neuron 1 to Neuron 5 added
|11:52:26:434734| Neuron 1 added to _presynaptic of Neuron 5
|11:52:26:434742| Neuron 5 has connections from
|11:52:26:434746| - Neuron1
|11:52:26:434768| Edge from Neuron 5 to Neuron 2 added
|11:52:26:434774| Neuron 5 added to _presynaptic of Neuron 2
|11:52:26:434797| Edge from Neuron 3 to Neuron 6 added
|11:52:26:434804| Neuron 3 added to _presynaptic of Neuron 6
|11:52:26:434827| Neuron 2 has connections from
|11:52:26:434832| - Neuron5
|11:52:26:434835| Edge from Neuron 4 to Neuron 2 added
|11:52:26:434838| Neuron 4 added to _presynaptic of Neuron 2
|11:52:26:434841| Neuron 5 is connected to:
|11:52:26:434843| - Neuron2
|11:52:26:434845| Neuron 5 has connections from
|11:52:26:434847| - Neuron1
|11:52:26:434849| Neuron 6 has connections from
|11:52:26:434851| - Neuron3
|11:52:26:434853| Edge from Neuron 5 to Neuron 6 added
|11:52:26:434856| Neuron 5 added to _presynaptic of Neuron 6

|11:52:26:434943| Neuron 1 is waiting
|11:52:26:434982| Neuron 2 is waiting
|11:52:26:435038| Neuron 3 is waiting
|11:52:26:435085| Neuron 4 is waiting
|11:52:26:435141| Neuron 5 is waiting
|11:52:26:435197| Neuron 6 is waiting


Activate neuron ( or [-1] to quit )
 Neuron 1
 Neuron 2
 Neuron 3
 Neuron 4
 Neuron 5
 Neuron 6
Input: 1

|11:52:30:499664| Neuron 1 is activated, accumulated equal to -55
|11:52:30:500392| Neuron 1 is sending a message to Neuron 5
|11:52:30:500416| Accumulated value for Neuron 1 is -55
|11:52:30:500431| Weight for Neuron 1 to Neuron 5 is 0.833798
|11:52:30:500444| Neuron 1 modifier is -1
|11:52:30:500453| Message is 45.8589
|11:52:30:500645| Neuron 5 is activated, accumulated equal to -9.14112
|11:52:30:500827| Neuron 5 is sending a message to Neuron 2
|11:52:30:500848| Accumulated value for Neuron 5 is -9.14112
|11:52:30:500860| Weight for Neuron 5 to Neuron 2 is 0.0295779
|11:52:30:500868| Neuron 5 modifier is -1
|11:52:30:500927| Message is 0.270375
|11:52:30:501054| Neuron 2 is activated, accumulated equal to -54.7296
|11:52:30:501296| Neuron 2 does not have any neigbors!
|11:52:30:501326| Neuron 2 is waiting
|11:52:30:501347| Neuron 5 is sending a message to Neuron 6
|11:52:30:501615| Accumulated value for Neuron 5 is -9.14112
|11:52:30:501765| Weight for Neuron 5 to Neuron 6 is 0.22352
|11:52:30:501922| Neuron 5 modifier is -1
|11:52:30:501952| Message is 2.04322
|11:52:30:501349| Neuron 1 fired, entering refractory phase
|11:52:30:502118| Neuron 6 is activated, accumulated equal to -52.9568
|11:52:30:502135| Neuron 1 potential set to -70|
11:52:30:502389| Neuron 6 does not have any neigbors!
|11:52:30:502682| Neuron 6 is waiting
|11:52:30:502431| Neuron 5 fired, entering refractory phase
|11:52:30:502956| Neuron 5 potential set to -70
|11:52:30:505017| Neuron 1 completed refractory phase, running
|11:52:30:505085| Neuron 1 is waiting
|11:52:30:514351| Neuron 5 completed refractory phase, running
|11:52:30:514546| Neuron 5 is waiting

Activate neuron ( or [-1] to quit )
 Neuron 1
 Neuron 2
 Neuron 3
 Neuron 4
 Neuron 5
 Neuron 6
Input: 2

|11:52:31:823041| Neuron 2 is activated, accumulated equal to -54.7296
|11:52:31:823208| Neuron 2 does not have any neigbors!
|11:52:31:823215| Neuron 2 is waiting

Activate neuron ( or [-1] to quit )
 Neuron 1
 Neuron 2
 Neuron 3
 Neuron 4
 Neuron 5
 Neuron 6
Input: 3

|11:52:32:926770| Neuron 3 is activated, accumulated equal to -55
|11:52:32:926912| Neuron 3 is sending a message to Neuron 6
|11:52:32:926932| Accumulated value for Neuron 3 is -55
|11:52:32:926944| Weight for Neuron 3 to Neuron 6 is 0.191137
|11:52:32:926952| Neuron 3 modifier is -1
|11:52:32:927043| Message is 10.5125
|11:52:32:927120| Neuron 3 fired, entering refractory phase
|11:52:32:927190| Neuron 3 potential set to -70
|11:52:32:927203| Neuron 6 is activated, accumulated equal to -42.4443
|11:52:32:927395| Neuron 6 does not have any neigbors!
|11:52:32:927481| Neuron 6 is waiting
|11:52:32:929689| Neuron 3 completed refractory phase, running
|11:52:32:929789| Neuron 3 is waiting

Activate neuron ( or [-1] to quit )
 Neuron 1
 Neuron 2
 Neuron 3
 Neuron 4
 Neuron 5
 Neuron 6
Input:

```
</details>

### 📌 Update 2-29

**New Additions:**
- Choose neuron to activate
- Activation based on membrane potential
- Refractory period
- Edge weights are [0, 1]
- Constants are preprocessor defintions

<details>
<summary> Example Output 3 </summary>
<br>


```
Neuron 1 added (inhibitory type)
Neuron 2 added (inhibitory type)
Neuron 3 added (inhibitory type)
Adding Random Neighbors
Edge from Neuron 3 to Neuron 2 added
Edge from Neuron 3 to Neuron 1 added
Neuron 1 is waiting
Neuron 2 is waiting
Neuron 3 is waiting
Activate neuron ( or [-1] to quit )
 Neuron 1
 Neuron 2
 Neuron 3
Input: 3
Neuron 3 is activated, accumulated equal to -55
Neuron 3 is sending a message to Neuron 1
Accumulated value for Neuron 3 is -55
Weight for Neuron 3 to Neuron 1 is 0.080745
Neuron 3 modifier is -1
Message is 4.44097
Neuron 1 is activated, accumulated equal to -50.559
Neuron 1 does not have any neigbors!
Neuron 1 is waiting
Neuron 3 is sending a message to Neuron 2
Accumulated value for Neuron 3 is -55
Weight for Neuron 3 to Neuron 2 is 0.694781
Neuron 3 modifier is -1
Message is 38.213
Neuron 2 is activated, accumulated equal to -16.787
Neuron 2 does not have any neigbors!
Neuron 2 is waiting
Neuron 3 fired, entering refractory phase
Neuron 3 potential set to -70
Neuron 3 completed refractory phase, running
Neuron 3 is waiting
Activate neuron ( or [-1] to quit )
 Neuron 1
 Neuron 2
 Neuron 3
Input: 3
```
- If Neuron 3 is then activated again

```
Neuron 3 is activated, accumulated equal to -70
Membrane potential for Neuron 3 is below the threshold, not firing
Neuron 3 is waiting
Activate neuron ( or [-1] to quit )
 Neuron 1
 Neuron 2
 Neuron 3
Input: 
```
- Or if a neuron without edges is activated:
```
Neuron 2 is activated, accumulated equal to -16.787
Neuron 2 does not have any neigbors!
Neuron 2 is waiting
Activate neuron ( or [-1] to quit )
 Neuron 1
 Neuron 2
 Neuron 3
Input:

```
</details>

<details>
<summary>Upadated Neuron Class</summary>
<br>

```cpp
class Neuron {
private:
  double membrane_potential = INITIAL_MEMBRANE_POTENTIAL;
  int id;

  typedef std::map<Neuron *, double> weight_map;

  weight_map _postsynaptic;
  weight_map _presynaptic;

  pthread_t thread;
  pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

  bool active = false;
  bool recieved = false;

  int excit_inhib_value;

public:
  Neuron(int _id, int inhibitory);
  ~Neuron();
  void add_neighbor(Neuron *neighbor, double weight);
  void add_next(Neuron *neighbor, double weight);
  void add_previous(Neuron *neighbor, double weight);
  void *run();
  void start_thread();
  void join_thread();

  void refractory();

  void activate() { active = true; }
  void deactivate() { active = false; }

  //>>>>>>>>>>>>>> Access to private variables <<<<<<<<<<<
  pthread_cond_t *get_cond() { return &cond; }
  int get_id() { return id; }
  double get_potential() { return membrane_potential; }
  const weight_map *get_presynaptic() {
    const weight_map *p_presynaptic = &_presynaptic;
    return p_presynaptic;
  }

  const weight_map *get_postsynaptic() {
    const weight_map *p_postsynaptic = &_postsynaptic;
    return p_postsynaptic;
  }
  /*--------------------------------------------------------------*\
   *                  Thread helper:
   *    POSIX needs a void* (*)(void*) function signature
   *    This function allows us to use the run() member funciton
  \--------------------------------------------------------------*/
  static void *thread_helper(void *instance) {
    return ((Neuron *)instance)->run();
  }
};
```
</details>

##### 🐛 ~~Issue 2~~
- I think the quit functionality of the menu sticks executable in a thread lock. Not sure how to fix it.
- I tried
    - Adjusting the mutex positioning to inside the for loop even through it should be on the outside
    - using a pthread barrier to align thread execution before quitting

<details>
<summary> Issue 2 code </summary>
<br>

``` cpp
// main.cpp
while (!finish) {

// sleep for menu timing
    usleep(100000);
    cout << "Activate neuron ( or [-1] to quit )\n";
    for (Neuron *neuron : neurons) {
      cout << " Neuron " << neuron->get_id() << '\n';
    }
    cout << "Input: ";
    cin >> activate;

    if (activate == -1) {
      //locking mutex
      pthread_mutex_lock(&mutex);

      // adjusting variable
      finish = true;
      
      // signaling each neuron to pthread_exit()
      // At this point all neurons should be the in the "waiting state"
      for (Neuron *neuron : neurons) {

        // activate neuron and signal
        neuron->activate();
        pthread_cond_signal(neuron->get_cond());

      }

      // unlock
      pthread_mutex_unlock(&mutex);

    } else if (activate <= num_neurons && activate >= 0) {
      neurons[activate - 1]->activate();
      pthread_cond_signal(neurons[activate - 1]->get_cond());
    }
}

// neuron.cpp
//...
  pthread_mutex_lock(&mutex);
  while (!active) {
    cout << "Neuron " << id << " is waiting\n";
    pthread_cond_wait(&cond, &mutex);
  }

  if (finish) {
    pthread_exit(NULL);
  }

  pthread_mutex_unlock(&mutex);
//...
```
</details>


### 📌 Update 2-28

**New Additions:**
- First update
<details>
<summary> Example Output 1 </summary>
<br>

```
Node 1 added
Node 2 added
Node 3 added
Adding Random Neighbors
Edge from Node 1 to Node 2 added
Edge from Node 2 to Node 3 added
Node 1 is waiting
Node 3 is waiting
Node 1 is activated, setting accumulated to 1
Node 1 is running
Node 1 is sending a message to Node2
Accumulated value for Node 1 is 1
Weight for Node 1 to Node 2 is 4
Message is 4
Node 2 is activated, setting accumulated to 4
Node 2 is running
Node 2 is sending a message to Node3
Accumulated value for Node 2 is 4
Weight for Node 2 to Node 3 is 3
Message is 12
Node 3 is activated, setting accumulated to 12
Node 3 is running
Total Value is 12
```
</details>

<details>
<summary> Example Output 2 </summary>
<br>

```
Node 1 added
Node 2 added
Node 3 added
Edge from Node 1 to Node 2 added
Edge from Node 1 to Node 3 added
Node 1 is waiting
Node 2 is waiting
Node 3 is waiting
Activate? 1
Node 1 is activated, accumulated set to 1
Node 1 is running
Node 1 is sending a message to Node 2
Accumulated value for Node 1 is 1
Weight for Node 1 to Node 2 is 3
Message is 3
Node 2 is activated, accumulated set to 3
Node 2 is running
Node 2 does not have any neigbors!
Node 1 is sending a message to Node 3
Accumulated value for Node 1 is 1
Weight for Node 1 to Node 3 is 2
Message is 2
Node 3 is activated, accumulated set to 2
Node 3 is running
Node 3 does not have any neigbors!
Node 1 has an accumulated value of 1
Node 2 has an accumulated value of 3
Node 3 has an accumulated value of 2
```
</details>

###### 🐛~~Issue 1~~
- Random neighbor funciton still needs adjustments to avoid repeat edges
<details>
<summary> Random neighbor function </summary>
<br>


```cpp
void random_neighbors(vector<Node *> nodes, int number_neighbors) {
  cout << "Adding Random Neighbors\n";
  int size = nodes.size();
  int i = 0;
  while (i < number_neighbors) {
    int from = rand() % size;
    int to = rand() % size;
    if (from == to) {
      continue;
    }
    nodes[from]->add_neighbor(nodes[to], rand() % 5 + 1);
    i++;
  }
}
```
</details>

<details>
<summary>Basic Node class that can send and recieve messages from other nodes:</summary>
<br>

```cpp
class Node {
private:
  double accumulated = 4;
  int id;
  std::map<Node *, double> neighbors;
  pthread_t thread;
  pthread_cond_t cond;
  bool active = false;
  bool recieved = false;

public:
  Node(int _id) : id(_id) {}
  ~Node();
  void add_neighbor(Node *neighbor, double weight);
  void *run();
  void start_thread();
  void join_thread();

  void activate() { active = true; }
  void deactivate() { active = false; }

  //>>>>>>>>>>>>>> Access to private variables <<<<<<<<<<<
  pthread_cond_t *get_cond() { return &cond; }
  int get_id() { return id; }
  double get_accumulated() { return accumulated; }

  /*--------------------------------------------------------------*\
   *                  Thread helper:
   *    POSIX needs a void* (*)(void*) function signature
   *    This function allows us to use the run() member funciton
  \--------------------------------------------------------------*/
  static void *thread_helper(void *instance) {
    return ((Node *)instance)->run();
  }
};
```
</details>


