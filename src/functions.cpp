#include "functions.hpp"
#include <cstdlib>

void random_neighbors(vector<Neuron *> nodes, int number_neighbors) {

  cout << "Adding Random Neighbors\n";
  int size = nodes.size();
  int i = 0;

  while (i < number_neighbors) {

    // Get random neurons
    int from = rand() % size;
    int to = rand() % size;

    // check for self connections
    if (from == to) {
      continue;
    }
    nodes[from]->add_neighbor(nodes[to], weight_function());
    i++;
  }
}

void print_node_values(vector<Neuron *> nodes) {
  for (Neuron *node : nodes) {
    cout << "Neuron " << node->get_id() << " has an accumulated value of "
         << node->get_potential() << '\n';
  }
}

double weight_function() { return (double)rand() / RAND_MAX; }

int get_inhibitory_status() {
  int ret;
  double x = (double)rand() / RAND_MAX;
  if (x >= 0.5) {
    ret = -1;
  } else {
    ret = 1;
  }

  return ret;
}
