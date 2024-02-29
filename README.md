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
| 2/29   | Updated Neuron Class with with membrane potentials, refractory phases, Update to edge weights, fixed issue 1, guard clauses on header files.   | "Quit" functionality does not work for the menu ([Issue 2](#issue-2))|
| 2/28   | Basic Node class that sends and recieves messages   | `random_neighbors` may repeat edges. [~~Issue 1~~](#issue-1)|

### Update 2/29

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


### Update 2/28

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


