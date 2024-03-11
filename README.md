# Honors Supplement for CS 141
Project for CS 141 Honors Supplement: Toy spiking neural network using a multithreaded approach.

### 📁 Folder Structure

```
├── src
│   ├── main.cpp
│   ├── functions.hpp
│   ├── functions.cpp
│   ├── neuron.hpp
│   └── neuron.cpp
├── pthred_ex //practice pthread examples
│   └── ...
├── logs 
│   └── // all .log files ignored 
├── .gitignore
├── README.md
└── makefile

```

### In-Progress 🚀
- [x] ~~Create and integrate Log Class~~
- [ ] Neuron Group Class ( In progress on `nclass` branch)
- [ ] Neuron Types for differentiated functionality (input, output)
- [ ] Possion Process for Neuron Activation
- [ ] Activate Neuron from file inputs (or other arbitrary input)

| Date  | Key Points 🔑   |  Issues 🐛   |
|--------------- | --------------- |--------------- |
| [3-11](#-update-3-11)   | Start of Neruon Group Class| None |
| [3-5](#-update-3-5)   | New fully integrated Log class. Write neuron ids and potential values to a `<current_time>.log` file. | None |
| [3-3 pt.2](#-update-3-3-part-2)   | Fixed [Issue 2](#-issue-2)| None |
| [3-3](#-update-3-3)   | Added time stamps to logging messages. Added function descriptions.| None |
| [2-29](#-update-2-29)   | Updated Neuron Class with with membrane potentials, refractory phases, Update to edge weights, fixed issue 1, guard clauses on header files.   | "Quit" functionality does not work for the menu [~~Issue 2~~](#-issue-2)|
| [2-28](#-update-2-28)   | Basic Node class that sends and recieves messages   | `random_neighbors` may repeat edges. [~~Issue 1~~](#-issue-1)|

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


