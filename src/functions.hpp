#ifndef FUNCTIONS
#define FUNCTIONS

#include "neuron.hpp"
#include <iostream>
#include <vector>

using std::cout;
using std::vector;

typedef std::map<Neuron *, double> weight_map;

// Assign random neighbors to all nodes in vector
//
// Reflexive edges and duplicate edges are not allowed.
// Edge weightes are randomly generated and are between [0, 1]
// @param1: vector of Neuron Pointers
// @param2: desired number of edges in the entire graph
void random_neighbors(vector<Neuron *> nodes, int number_neighbors);

// Print membrane potential of all neurons in vector
//
//
void print_node_values(vector<Neuron *> nodes);

void print_maps(Neuron *neuron);

void print_time(std::ostream &os = std::cout);
double weight_function();
int get_inhibitory_status();

#endif // !FUNCTIONS
