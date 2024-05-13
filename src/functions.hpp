/**
 * @file
 */
#ifndef FUNCTIONS
#define FUNCTIONS

#include "input_neuron.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include <algorithm>
#include <iostream>
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

typedef std::map<Neuron *, double> weight_map;

/**
 * Calculates the maximum possible eges in a graph of N neurons.
 *
 * Adjusts for input neurons having no incoming edges. Uses the formula
 * n (n-1) / 2.
 *
 */
int maximum_edges();

/**
 * Checks various run conditions and exits if any fail.
 */
void check_start_conditions();

/**
 * Assign random neighbors to all nodes in vector
 * Reflexive edges and duplicate edges are not allowed.
 *
 * Edge weightes are randomly generated and are between [0, 1]
 *
 * @param groups: vector of Neuron pointers
 */
void random_synapses(vector<NeuronGroup *> &groups);

/**
 * Print membrane potential of all neurons in vector.
 * @param nodes: vector of Neuron pointers
 */
void print_node_values(vector<Neuron *> nodes);

/** Returns random weight for an edge
 *
 * Returns a value in the range [0,1] inclusive
 * based on the rand() function.
 * @return: double in range [0,1]
 */
double weight_function();

/** Returns either -1 or 1
 *
 * Returns either -1 or 1 based on the rand() function.
 * Seed is in config file and stored in RuntimeConfig
 * @return: 1 or -1
 *
 */
int get_inhibitory_value();

int get_neuron_count(const vector<NeuronGroup *> &groups);

/** Get the string form of the active status of a neuron
 *
 * @param active (Neuron::active)
 * @return const char* of "active" or "inactive"
 */
const char *get_active_status_string(bool active);

// Deallocates the memory held in the message vector
//
// @param1: pointer to the message vector
void deallocate_message_vector(const vector<Message *> *messages);

// Parse command line arguements
//
// Looks for config.toml file or help command
// If the toml is found, tries to use it; otherwise,
// uses the default
int parse_command_line_args(char **argv, int argc);

// Creates base_toml if needed and sets settings
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

void set_line_x(const vector<InputNeuron *> &input_neurons, int target);

void get_line_x(std::string &line, int target);

#endif // !FUNCTIONS
