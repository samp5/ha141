#ifndef FUNCTIONS
#define FUNCTIONS

#include "log.hpp"
#include "neuron.hpp"
#include <iostream>
#include <vector>

using std::cout;
using std::vector;
extern Log lg;

typedef std::map<Neuron *, double> weight_map;

// Assign random neighbors to all nodes in vector
//
// Reflexive edges and duplicate edges are not allowed.
// Edge weightes are randomly generated and are between [0, 1]
// @param1: vector of Neuron pointers
// @param2: desired number of edges in the entire graph
void random_neighbors(vector<Neuron *> nodes, int number_neighbors);

// Print membrane potential of all neurons in vector
//
// @param1: vector of Neuron pointers
void print_node_values(vector<Neuron *> nodes);

// Print out presynaptic and postsynapic connections of a neuron
//
// @param1: Neuron pointer
void print_maps(Neuron *neuron);

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

#endif // !FUNCTIONS
