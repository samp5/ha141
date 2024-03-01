#include "functions.hpp"
#include <cstdlib>

void print_maps(Neuron *neuron) {
  const weight_map *p_postsyntapic = neuron->get_postsynaptic();
  const weight_map *p_presyntapic = neuron->get_presynaptic();

  if (!p_postsyntapic->empty()) {
    // print post synaptic
    weight_map::const_iterator post_it = p_postsyntapic->begin();
    cout << "Neuron " << neuron->get_id() << " is connected to:\n";
    while (post_it != p_postsyntapic->end()) {
      cout << "- Neuron" << post_it->first->get_id() << '\n';
      ++post_it;
    }
  }

  if (!p_presyntapic->empty()) {
    // print pre synaptic
    weight_map::const_iterator pre_it = p_presyntapic->begin();
    cout << "Neuron " << neuron->get_id() << " has connections from\n";
    while (pre_it != p_presyntapic->end()) {
      cout << "- Neuron" << pre_it->first->get_id() << '\n';
      ++pre_it;
    }
  }
}

bool has_neighbor(Neuron *from_neuron, Neuron *to_neuron) {
  bool ret;
  const weight_map *p_postsyntapic = from_neuron->get_postsynaptic();
  const weight_map *p_presyntapic = to_neuron->get_presynaptic();

  print_maps(from_neuron);
  print_maps(to_neuron);

  if (p_postsyntapic->find(to_neuron) != p_postsyntapic->end()) {
    cout << "Neuron " << from_neuron->get_id() << " is already connected to "
         << to_neuron->get_id() << '\n';
    // if the to neuron is not already in the postsynaptic map
    ret = true;

  } else if (p_presyntapic->find(from_neuron) != p_presyntapic->end()) {

    cout << "Neuron " << to_neuron->get_id()
         << " already has a connection from " << from_neuron->get_id() << '\n';

    // if the to_neuron is not in the presynaptic list
    ret = true;

  } else {

    ret = false;
  }

  return ret;
}

void random_neighbors(vector<Neuron *> nodes, int number_neighbors) {

  cout << "Adding Random Neighbors\n";
  int size = nodes.size();
  int i = 0;
  if (number_neighbors > size) {
    number_neighbors = size;
    cout << "random_neighbors: Number of neighbors exceeds size, setting "
            "number of neighbors to size\n";
  }

  while (i < number_neighbors) {

    // Get random neurons
    int from = rand() % size;
    int to = rand() % size;

    // check for self connections
    if (from == to) {
      continue;
    }
    if (has_neighbor(nodes[from], nodes[to])) {
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
