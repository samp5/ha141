#ifndef FUNCTIONS
#define FUNCTIONS

#include "neuron.hpp"
#include <iostream>
#include <vector>

using std::cout;
using std::vector;

void random_neighbors(vector<Neuron *> nodes, int number_neighbors);
void print_node_values(vector<Neuron *> nodes);
double weight_function();
int get_inhibitory_status();

#endif // !FUNCTIONS
