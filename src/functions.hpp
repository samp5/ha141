#ifndef FUNCTIONS
#define FUNCTIONS

#include "log.hpp"
#include "neuron.hpp"
#include <iostream>
#include <vector>

using std::cout;
using std::vector;
extern Log lg;
extern bool active;
extern const int WAIT_TIME;
extern const int WAIT_INCREMENT;
extern const double DECAY_VALUE;

typedef std::map<Neuron *, double> weight_map;

// Assign random neighbors to all nodes in vector
//
// Reflexive edges and duplicate edges are not allowed.
// Edge weightes are randomly generated and are between [0, 1]
// @param1: vector of Neuron pointers
// @param2: desired number of edges in the entire graph
void random_neighbors(vector<Neuron *> nodes, int number_neighbors);

// Assign random neighbors to all nodes in each group in group
// vector
//
// Reflexive edges and duplicate edges are not allowed.
// Edge weightes are randomly generated and are between [0, 1]
// @param1: vector of NeuronGroup pointers
// @param2: desired number of edges in the entire graph
void random_group_neighbors(vector<NeuronGroup *> groups, int numbe_neighbors);

// Print membrane potential of all neurons in vector
//
// @param1: vector of Neuron pointers
void print_node_values(vector<Neuron *> nodes);

// Print out presynaptic and postsynapic connections of a neuron
//
// @param1: Neuron pointer
void print_maps(Neuron *neuron);

// Print out presynaptic and postsynapic connections of a neuron
// with group data
//
// @param1: Neuron pointer
void print_group_maps(Neuron *neuron);

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
Neuron *get_random_neuron(const vector<NeuronGroup *> &group);

// Get the string form of the active status of a neuron
//
//@param1 bool (Neuron::active)
//@return const char* of "active" or "inactive"
const char *get_active_status_string(bool active);

void construct_messages_from_file();

Message *construct_message(double value, Neuron *target);
vector<Message *>
construct_message_vector_from_file(vector<NeuronGroup *> groups,
                                   const char *file_name);
void print_message(Message *message);

void *send_message_helper(void *messages);
void send_messages(const vector<Message *> *messages);
void *send_message_helper(void *messages);
void deallocate_message_vector(const vector<Message *> *messages);
void *decay_helper(void *groups);
void decay_neurons(vector<NeuronGroup *> *groups);
#endif // !FUNCTIONS
