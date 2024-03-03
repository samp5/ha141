#include "functions.hpp"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <ostream>

void print_time(std::ostream &os) {
  unsigned long long now =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  time_t time_now = time(0);
  long int microseconds = now - 1e6 * time_now;

  tm *ltm = localtime(&time_now);

  os << "|" << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec << ":"
     << microseconds << "|"
     << " ";
}

void print_maps(Neuron *neuron) {
  const weight_map *p_postsyntapic = neuron->get_postsynaptic();
  const weight_map *p_presyntapic = neuron->get_presynaptic();

  if (!p_postsyntapic->empty()) {
    // print post synaptic
    weight_map::const_iterator post_it = p_postsyntapic->begin();
    print_time();
    cout << "Neuron " << neuron->get_id() << " is connected to:\n";
    while (post_it != p_postsyntapic->end()) {
      print_time();
      cout << "- Neuron" << post_it->first->get_id() << '\n';
      ++post_it;
    }
  }

  if (!p_presyntapic->empty()) {
    // print pre synaptic
    weight_map::const_iterator pre_it = p_presyntapic->begin();
    print_time();
    cout << "Neuron " << neuron->get_id() << " has connections from\n";
    while (pre_it != p_presyntapic->end()) {
      print_time();
      cout << "- Neuron" << pre_it->first->get_id() << '\n';
      ++pre_it;
    }
  }
}

bool has_neighbor(Neuron *from_neuron, Neuron *to_neuron) {
  bool ret;
  const weight_map *p_postsyntapic = from_neuron->get_postsynaptic();
  const weight_map *p_presyntapic = from_neuron->get_presynaptic();

  // print maps as debugging measure
  print_maps(from_neuron);
  print_maps(to_neuron);

  // check for connection FROM from_neuron TO to_neuron
  if (p_postsyntapic->find(to_neuron) != p_postsyntapic->end()) {
    cout << "Neuron " << from_neuron->get_id() << " is already connected to "
         << to_neuron->get_id() << '\n';
    // if the to neuron is not already in the postsynaptic map
    ret = true;
  }
  // check for connections FROM to_neuron TO from_neuron
  else if (p_presyntapic->find(to_neuron) != p_presyntapic->end()) {
    cout << "Neuron " << to_neuron->get_id()
         << " already has a connection from " << from_neuron->get_id() << '\n';
    // if the to_neuron is in the presynaptic list
    ret = true;
  } else {
    ret = false;
  }
  return ret;
}

void random_neighbors(vector<Neuron *> nodes, int number_neighbors) {

  cout << "\nAdding Random Edges\n";
  cout << "--------------------------\n\n";
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
  cout << '\n';
}

void print_node_values(vector<Neuron *> nodes) {
  cout << "\nFinal Neuron Values\n";
  cout << "-------------------\n\n";
  for (Neuron *node : nodes) {
    print_time();
    cout << "Neuron " << node->get_id() << ": " << node->get_potential()
         << '\n';
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
