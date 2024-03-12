#include "functions.hpp"
#include "log.hpp"
#include <cstdlib>
#include <ostream>
#include <vector>

void print_group_maps(Neuron *neuron) {
  const weight_map *p_postsyntapic = neuron->get_postsynaptic();
  const weight_map *p_presyntapic = neuron->get_presynaptic();

  if (!p_postsyntapic->empty()) {
    // print post synaptic
    weight_map::const_iterator post_it = p_postsyntapic->begin();

    lg.log_group_neuron_state(DEBUG2, "      (%d) Neuron %d is connected to:",
                              neuron->get_group()->get_id(), neuron->get_id());

    while (post_it != p_postsyntapic->end()) {
      lg.log_group_neuron_state(DEBUG2, "         (%d) Neuron %d",
                                post_it->first->get_group()->get_id(),
                                post_it->first->get_id());
      ++post_it;
    }
  } else {
    lg.log_group_neuron_state(
        DEBUG3, "         (%d) Neuron %d has no outgoing connections",
        neuron->get_group()->get_id(), neuron->get_id());
  }

  if (!p_presyntapic->empty()) {
    // print pre synaptic
    weight_map::const_iterator pre_it = p_presyntapic->begin();
    lg.log_group_neuron_state(DEBUG2,
                              "      (%d) Neuron %d has connections from:",
                              neuron->get_group()->get_id(), neuron->get_id());
    while (pre_it != p_presyntapic->end()) {
      lg.log_group_neuron_state(DEBUG2, "        (%d) Neuron %d",
                                pre_it->first->get_group()->get_id(),
                                pre_it->first->get_id());
      ++pre_it;
    }
  } else {
    lg.log_group_neuron_state(
        DEBUG3, "         (%d) Neuron %d has no incoming connections",
        neuron->get_group()->get_id(), neuron->get_id());
  }
}

void print_maps(Neuron *neuron) {
  const weight_map *p_postsyntapic = neuron->get_postsynaptic();
  const weight_map *p_presyntapic = neuron->get_presynaptic();

  if (!p_postsyntapic->empty()) {
    // print post synaptic
    weight_map::const_iterator post_it = p_postsyntapic->begin();

    lg.log_neuron_state(DEBUG, "Neuron %d is connected to:", neuron->get_id());

    while (post_it != p_postsyntapic->end()) {
      lg.log_neuron_state(DEBUG, "    - Neuron %d", post_it->first->get_id());
      ++post_it;
    }
  }

  if (!p_presyntapic->empty()) {
    // print pre synaptic
    weight_map::const_iterator pre_it = p_presyntapic->begin();
    lg.log_neuron_state(DEBUG,
                        "Neuron %d has connections from:", neuron->get_id());
    while (pre_it != p_presyntapic->end()) {
      lg.log_neuron_state(DEBUG, "    - Neuron %d", pre_it->first->get_id());
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

    lg.log_neuron_interaction(
        DEBUG, "has_neighbor: Neuron %d is already connected to Neuron %d",
        from_neuron->get_id(), to_neuron->get_id());

    // if the to neuron is not already in the postsynaptic map
    ret = true;
  }
  // check for connections FROM to_neuron TO from_neuron
  else if (p_presyntapic->find(to_neuron) != p_presyntapic->end()) {
    lg.log_neuron_interaction(
        DEBUG, "has_neighbor: Neuron %d already has connection from Neuron %d",
        to_neuron->get_id(), from_neuron->get_id());

    // if the to_neuron is in the presynaptic list
    ret = true;
  } else {
    ret = false;
  }
  return ret;
}

bool has_neighbor_group(Neuron *from_neuron, Neuron *to_neuron) {
  bool ret;
  const weight_map *p_postsyntapic = from_neuron->get_postsynaptic();
  const weight_map *p_presyntapic = from_neuron->get_presynaptic();

  // print maps as debugging measure
  print_group_maps(from_neuron);
  print_group_maps(to_neuron);

  // check for connection FROM from_neuron TO to_neuron
  if (p_postsyntapic->find(to_neuron) != p_postsyntapic->end()) {

    lg.log_neuron_interaction(
        DEBUG, "has_neighbor: Neuron %d is already connected to Neuron %d",
        from_neuron->get_id(), to_neuron->get_id());

    // if the to neuron is not already in the postsynaptic map
    ret = true;
  }
  // check for connections FROM to_neuron TO from_neuron
  else if (p_presyntapic->find(to_neuron) != p_presyntapic->end()) {
    lg.log_neuron_interaction(
        DEBUG, "has_neighbor: Neuron %d already has connection from Neuron %d",
        to_neuron->get_id(), from_neuron->get_id());

    // if the to_neuron is in the presynaptic list
    ret = true;
  } else {
    ret = false;
  }
  return ret;
}

void random_neighbors(vector<Neuron *> nodes, int number_neighbors) {

  lg.print("\nAdding Random Edges");
  lg.print("======================\n\n");
  int size = nodes.size();
  int i = 0;
  if (number_neighbors > size) {
    number_neighbors = size;
    lg.log(WARNING, "random_neighbors: Number of neighbors exceeds size, "
                    "setting number of neighbors to size");
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

void random_group_neighbors(vector<NeuronGroup *> groups,
                            int number_neighbors) {
  lg.print("\nAdding Random Edges");
  lg.print("======================\n");

  int neuron_count = get_neuron_count(groups);
  int i = 0;

  if (number_neighbors > neuron_count) {
    number_neighbors = neuron_count;
    lg.log(WARNING, "random_neighbors: Number of neighbors exceeds size, "
                    "setting number of neighbors to size");
  }

  while (i < number_neighbors) {

    // Get random neurons
    Neuron *from = get_random_neuron(groups);
    Neuron *to = get_random_neuron(groups);

    // check for self connections
    if (from == to) {
      continue;
    }
    if (has_neighbor_group(from, to)) {
      continue;
    }

    from->add_neighbor(to, weight_function());
    i++;
  }
}

int get_neuron_count(const vector<NeuronGroup *> &groups) {
  int size = 0;
  for (auto group : groups) {
    size += group->neuron_count();
  }
  return size;
}

Neuron *get_random_neuron(const vector<NeuronGroup *> &groups) {
  int group_number = rand() % groups.size();
  int neuron_number = rand() % groups[group_number]->neuron_count();
  return groups[group_number]->get_neruon_vector()[neuron_number];
}

void print_node_values(vector<Neuron *> nodes) {
  cout << "\nFinal Neuron Values\n";
  cout << "-------------------\n\n";
  for (Neuron *node : nodes) {
    lg.log_neuron_value(INFO, "Neuron %d : %f", node->get_id(),
                        node->get_potential());
  }
}

double weight_function() { return (double)rand() / RAND_MAX; }

int get_inhibitory_status() {
  int ret;
  double x = (double)rand() / RAND_MAX;
  if (x >= 0.2) {
    ret = -1;
  } else {
    ret = 1;
  }

  return ret;
}
char *get_active_status_string(bool active) {
  char *ret;
  if (active) {
    ret = "active"; // NOLINT
    return ret;
  } else {
    ret = "inactive"; // NOLINT
    return ret;
  }
}
