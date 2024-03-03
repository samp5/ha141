## Honors Supplement for CS 141
Project for CS 141 Honors Supplement

### Folder Structure

```
├── src
│   ├── main.cpp
│   ├── functions.hpp
│   ├── functions.cpp
│   ├── neuron.hpp
│   └── neuron.cpp
├── pthred_ex //practice pthread examples
│   └── ...
├── .gitignore
├── README.md
└── makefile

```
| Date   | Key Points    |  Issues   |
|--------------- | --------------- |--------------- |
| [3/3](#update-3-3)   | Added time stamps to logging messages| |
| [2/29](#update-2-29)   | Updated Neuron Class with with membrane potentials, refractory phases, Update to edge weights, fixed issue 1, guard clauses on header files.   | "Quit" functionality does not work for the menu [Issue 2](#issue-2)|
| [2/28](#update-2-28)   | Basic Node class that sends and recieves messages   | `random_neighbors` may repeat edges. [~~Issue 1~~](#issue-1)|

### Update 3/3

<details>
<summary> Example Output 4 </summary>
<br>

New addtions:
- Time stamps on logging messages


``` cpp
Time format is |HH:MM:SS:mircroseconds|

Adding Neurons
----------------

|10:24:53:508828| Neuron 1 added (excitatory type)
|10:24:53:508890| Neuron 2 added (excitatory type)
|10:24:53:508894| Neuron 3 added (excitatory type)
|10:24:53:508897| Neuron 4 added (excitatory type)
|10:24:53:508901| Neuron 5 added (excitatory type)
|10:24:53:508904| Neuron 6 added (excitatory type)

Adding Random Neighbors
--------------------------

|10:24:53:508911| Edge from Neuron 5 to Neuron 6 added
|10:24:53:508918| Neuron 5 added to _presynaptic of Neuron 6
|10:24:53:508970| Neuron 6 has connections from
|10:24:53:508980| - Neuron5
|10:24:53:508987| Edge from Neuron 6 to Neuron 3 added
|10:24:53:508998| Neuron 6 added to _presynaptic of Neuron 3
|10:24:53:509087| Neuron 5 is connected to:
|10:24:53:509101| - Neuron6
|10:24:53:509107| Edge from Neuron 5 to Neuron 4 added
|10:24:53:509115| Neuron 5 added to _presynaptic of Neuron 4
|10:24:53:509126| Neuron 4 has connections from
|10:24:53:509137| - Neuron5
|10:24:53:509142| Edge from Neuron 4 to Neuron 2 added
|10:24:53:509149| Neuron 4 added to _presynaptic of Neuron 2
|10:24:53:509154| Neuron 3 has connections from
|10:24:53:509157| - Neuron6
|10:24:53:509163| Edge from Neuron 3 to Neuron 1 added
|10:24:53:509168| Neuron 3 added to _presynaptic of Neuron 1
|10:24:53:509479| Neuron 1 is waiting
|10:24:53:509687| Neuron 3 is waiting
|10:24:53:509840| Neuron 2 is waiting
|10:24:53:510043| Neuron 6 is waiting
|10:24:53:510116| Neuron 5 is waiting
|10:24:53:510304| Neuron 4 is waiting

Activate neuron ( or [-1] to quit )
 Neuron 1
 Neuron 2
 Neuron 3
 Neuron 4
 Neuron 5
 Neuron 6
Input: 2

|11:8:53:669752| Neuron 2 is activated, accumulated equal to -55
|11:8:53:670240| Neuron 2 is sending a message to Neuron 3
|11:8:53:670262| Accumulated value for Neuron 2 is -55
|11:8:53:670355| Weight for Neuron 2 to Neuron 3 is 0.628871
|11:8:53:670472| Neuron 2 modifier is 1
|11:8:53:670492| Message is -34.5879
|11:8:53:670632| Neuron 3 is activated, accumulated equal to -89.5879
|11:8:53:670852| Membrane potential for Neuron 3 is below the threshold, not firing
|11:8:53:670879| Neuron 3 is waiting
|11:8:53:670959| Neuron 2 fired, entering refractory phase
|11:8:53:671058| Neuron 2 potential set to -70
|11:8:53:673609| Neuron 2 completed refractory phase, running
|11:8:53:673867| Neuron 2 is waiting

Activate neuron ( or [-1] to quit )
 Neuron 1
 Neuron 2
 Neuron 3
 Neuron 4
 Neuron 5
 Neuron 6
Input: 6
|11:9:36:226487| Neuron 6 is activated, accumulated equal to -55
|11:9:36:226578| Neuron 6 is sending a message to Neuron 1
|11:9:36:226584| Accumulated value for Neuron 6 is -55
|11:9:36:226589| Weight for Neuron 6 to Neuron 1 is 0.61264
|11:9:36:226594| Neuron 6 modifier is 1
|11:9:36:226598| Message is -33.6952
|11:9:36:226711| Neuron 1 is activated, accumulated equal to -88.6952
|11:9:36:226833| Neuron 1 does not have any neigbors!
|11:9:36:226931| Neuron 1 is waiting
|11:9:36:226865| Neuron 6 is sending a message to Neuron 5
|11:9:36:226987| Accumulated value for Neuron 6 is -55
|11:9:36:227038| Weight for Neuron 6 to Neuron 5 is 0.635712
|11:9:36:227046| Neuron 6 modifier is 1
|11:9:36:227050| Message is -34.9641
|11:9:36:227141| Neuron 5 is activated, accumulated equal to -89.9641
|11:9:36:227227| Membrane potential for Neuron 5 is below the threshold, not firing
|11:9:36:227242| Neuron 5 is waiting
|11:9:36:227253| Neuron 6 fired, entering refractory phase
|11:9:36:227277| Neuron 6 potential set to -70
|11:9:36:230011| Neuron 6 completed refractory phase, running
|11:9:36:230090| Neuron 6 is waiting

```
</details>

### Update 2-29

<details>
<summary> Example Output 3 </summary>
<br>

New addtions:
- Choose neuron to activate
- Activation based on membrane potential
- Refractory period
- Edge weights are [0, 1]
- Constants are preprocessor defintions

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

##### Issue 2
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


### Update 2-28

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

###### ~~Issue 1~~
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


