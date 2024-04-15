#ifndef FUNCTIONS
#define FUNCTIONS

#include "../run_config/toml.hpp"
#include "input_neuron.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <vector>

using std::copy_if;
using std::cout;
using std::distance;
using std::vector;

extern Log lg;
extern bool active;
extern int WAIT_TIME;
extern int WAIT_LOOPS;
extern double DECAY_VALUE;
extern int RAND_SEED;
extern int NUMBER_NEURONS;
extern int NUMBER_EDGES;
extern int NUMBER_GROUPS;
extern unsigned long RUN_TIME;
extern double DECAY_VALUE;
extern ostream &STREAM;
extern int INITIAL_MEMBRANE_POTENTIAL;
extern int ACTIVATION_THRESHOLD;
extern int REFRACTORY_MEMBRANE_POTENTIAL;
extern int NUMBER_INPUT_NEURONS;
extern double TAU;
extern double INPUT_PROB_SUCCESS;
extern double REFRACTORY_DURATION;
extern std::string INPUT_FILE;
extern std::string CONFIG_FILE;

typedef std::map<Neuron *, double> weight_map;

int maximum_edges();

void check_start_conditions();

void destroy_mutexes();

// Assign random neighbors to all nodes in vector
//
// Reflexive edges and duplicate edges are not allowed.
// Edge weightes are randomly generated and are between [0, 1]
// @param1: vector of Neuron pointers
// @param2: desired number of edges in the entire graph
void random_synapses(vector<NeuronGroup *> groups, int number_neighbors);

// Print membrane potential of all neurons in vector
//
// @param1: vector of Neuron pointers
void print_node_values(vector<Neuron *> nodes);

// Prints out the current time with microsecond granularity
//
void print_time(std::ostream &os = std::cout);

// Returns random weight for an edge
//
// Returns a value in the range [0,1] inclusive
// based on the rand() function. The seed is set in ./src/main
// Seed is default the time since epoch
//
// @return: double in range [0,1]
double weight_function();

// Returns either -1 or 1
//
// Returns either -1 or 1 based on the rand() function.
// The seed is set in ./src/main
// Seed is default the time since epoch
//
// @return: 1 or -1
int get_inhibitory_status();

// Get total number of neurons in a group
//
//@param1 const reference to the NeuronGroup vector
//@returns total neuron count
int get_neuron_count(const vector<NeuronGroup *> &groups);

// Get a random neuron from the group list
//
//@param1 const reference to the NeuronGroup vector
//@return pointer to neuron
Neuron *get_random_neuron(const vector<NeuronGroup *> &group,
                          bool input_type_allowed = true);

// Get the string form of the active status of a neuron
//
//@param1 bool (Neuron::active)
//@return const char* of "active" or "inactive"
const char *get_active_status_string(bool active);

// Construct a message
//
//@param1: value for message
//@param2: target neuron
//@returns: pointer to dynamically allocated Message
Message *construct_message(double value, Neuron *target);

// Construct a message
//
//@param1: value for message
//@param2: target neuron
//@param3: Message_t
//@returns: pointer to dynamically allocated Message
Message *construct_message(double value, Neuron *target, Message_t type);

// Prints out a message via the Log class
//
//@param1: pointer to message
void print_message(Message *message);

// Thread helper to run message thread
//
// Correct function signature so that
// `send_messages` can be run in a thread
//
// @param1 MUST BE `void*` to a `vector<Message*>`
void *send_message_helper(void *messages);

// Send messages on loop every WAIT_TIME * WAIT_LOOPS
// to every neuron
//
// While ::active, sends the repective message to every
// neuron every WAIT_TIME * WAIT_LOOPS
// activates the recieving neuron
//
// @param1: pointer to the message vector
void send_messages(const vector<Message *> *messages);

// Deallocates the memory held in the message vector
//
// @param1: pointer to the message vector
void deallocate_message_vector(const vector<Message *> *messages);

// Thread helper to run decay thread
//
// Correct function signature so that
// `decay_neurons` can be run in a thread
//
// @param1 MUST BE `void*` to a `vector<NeuronGroup*>`
void *decay_helper(void *groups);

// Decays every neuron on timer
//
// While ::active, decays every neruon every
// `WAIT_LOOPS` * `WAIT_TIME` by `DECAY_VALUE`
//
// @param1 pointer to the neuron group vector
void decay_neurons(vector<NeuronGroup *> *groups);

// Parse command line arguements
//
// Looks for config.toml file or help command
// If the toml is found, tries to use it; otherwise,
// uses the default
int parse_command_line_args(char **argv, int argc);

// Creates base_toml if needed and sets settings
//
void use_base_toml();

// Creates base_toml
void create_base_toml();

// Sets all global variables based on
// keys in toml file
//
// @param1 file_name
int set_options(const char *file_name);

// Checks to see if a file file_exists
//
// @param1 file_name
bool file_exists(const char *file_name);

// Get enum member from LogLevel from string
//
// @param1 string version of enum members
LogLevel get_level_from_string(std::string level);

// Assign neurons to groups based on global variable counts
//
// Accounts for non-devisible neuron amounts and creates
// the most evenly distruted groups possible
//
// @param1 reference to neuron groups vector
void assign_groups(vector<NeuronGroup *> &neuron_groups);

void assign_neuron_types(vector<NeuronGroup *> &groups);

std::string io_type_to_string(Neuron_t type);
std::string message_type_to_string(Message_t type);
void construct_input_neuron_vector(const vector<NeuronGroup *> &groups,
                                   vector<InputNeuron *> &input_neurons);
void get_next_line(std::string &line);
void set_next_line(const vector<InputNeuron *> &input_neurons);
#endif // !FUNCTIONS
