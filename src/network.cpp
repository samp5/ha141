#include "network.hpp"
#include "neuron.hpp"
#include "neuron_group.hpp"
#include "runtime.hpp"
#include <algorithm>
#include <fstream>
#include <pthread.h>
#include <sstream>

SNN::~SNN() {
  for (auto group : groups) {
    delete group;
  }
}
SNN::SNN(RuntimConfig *config) : config(config) {
  int neuron_per_group = config->NUMBER_NEURONS / config->NUMBER_GROUPS;
  int input_neurons_per_group =
      config->NUMBER_INPUT_NEURONS / config->NUMBER_GROUPS;

  for (int i = 0; i < config->NUMBER_GROUPS; i++) {

    // allocate for this group
    NeuronGroup *this_group =
        new NeuronGroup(i + 1, neuron_per_group, input_neurons_per_group);

    // add to vector
    this->groups.push_back(this_group);
  }
  this->generateNeuronVec();
  this->generateInputNeuronVec();
}

void SNN::generateInputNeuronVec() {
  if (!input_neurons.empty()) {
    lg.log(ERROR,
           "get_input_neuron_vector: this constructs a new vector! "
           "emptying vector that was passed...\n If Neurons in this vector "
           "were dynamically allocated, that memory has NOT been freed");
    input_neurons.clear();
  }

  for (const auto &group : groups) {
    for (auto neuron : group->get_neruon_vector()) {
      if (neuron->getType() != Input) {
        continue;
      }
      input_neurons.push_back(dynamic_cast<InputNeuron *>(neuron));
    }
  }
}

void SNN::generateNeuronVec() {
  for (auto group : groups) {
    for (auto neuron : group->get_neruon_vector()) {
      this->neurons.push_back(neuron);
    }
  }
}

std::unordered_map<Neuron *, std::list<Neuron *>>
SNN::generateNeighborOptions() {
  std::unordered_map<Neuron *, std::list<Neuron *>> map;
  for (auto neuron_origin : this->neurons) {
    std::list<Neuron *> origin_list;
    for (auto neuron_destination : this->neurons) {
      if (neuron_origin == neuron_destination) {
        continue;
      }
      if (neuron_destination->getType() == Neuron_t::Input) {
        continue;
      }
      origin_list.push_back(neuron_destination);
    }
    map.insert({neuron_origin, origin_list});
  }
  return map;
}

void SNN::generateRandomSynapses() {
  auto start = lg.get_time_stamp();
  int synapses_formed = 0;
  if (this->neurons.empty()) {
    generateNeuronVec();
  }
  auto map = generateNeighborOptions();

  for (auto neuron : this->neurons) {

    // get the neighbor_options for this neuron from the map
    std::list<Neuron *> &neighbor_options = map.at(neuron);
    if (neighbor_options.empty()) {
      continue;
    }

    // get a random neuron in this list
    auto target = neighbor_options.begin();
    std::advance(target, rand() % neighbor_options.size());

    // get the neighbor_options for the postsynaptic neuron and an iter to
    // this neuron
    std::list<Neuron *> &post_opts = map.at(*target);
    auto this_neuron = std::find(post_opts.begin(), post_opts.end(), neuron);

    // add postsynaptic neuron as a neighbor
    neuron->addNeighbor(*target, this->weight_function());

    // erase the postsynaptic neuron from the list and this neuron from the
    // postsynaptic list
    neighbor_options.erase(target);
    if (this_neuron != post_opts.end()) {
      post_opts.erase(this_neuron);
    }

    // increment the synapses
    synapses_formed += 1;

    // check to make sure we still need edges
    if (synapses_formed >= cf.NUMBER_EDGES) {
      break;
    }
  }
  auto end = lg.get_time_stamp();
  std::string msg = "Adding random synapses done: took " +
                    std::to_string(end - start) + " seconds";
  lg.log(ESSENTIAL, msg.c_str());
}

double SNN::weight_function() { return ((double)rand() / RAND_MAX) * 0.1; }

void SNN::join() {
  for (auto group : this->groups) {
    pthread_join(group->get_thread_id(), NULL);
  }
}
void SNN::reset() {
  for (auto group : this->groups) {
    group->reset();
  }
}
void SNN::start() {

  this->setStimLineX(*config->STIMULUS);

  for (auto group : this->groups) {
    group->start_thread();
  }

  for (int i = 1; i < config->num_stimulus + 1; i++) {

    usleep(config->time_per_stimulus);

    if (i < config->num_stimulus) {
      auto start = std::chrono::high_resolution_clock::now();
      switching_stimulus = true;

      config->STIMULUS++;
      this->setNextStim();
      lg.log_value(ESSENTIAL, "Set stimulus to line %d", *cf.STIMULUS);

      this->reset();

      pthread_mutex_lock(&mx.stimulus);
      switching_stimulus = false;
      pthread_cond_broadcast(&stimulus_switch_cond);
      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(
          end - start);
      lg.set_offset(duration.count());
      pthread_mutex_unlock(&mx.stimulus);
    }
  }
  active = false;
}

void SNN::setStimLineX(int target) {

  lg.log_value(ESSENTIAL, "Set stimulus to line %d", *cf.STIMULUS);

  if (input_neurons.empty()) {
    lg.log(ESSENTIAL, "set_line_x: passed empty input neuron vector?");
    return;
  }

  std::string line;
  getLineX(line, target);

  std::stringstream s(line);
  double value;

  for (InputNeuron *input_neuron : input_neurons) {
    s >> value;
    input_neuron->set_input_value(value);
  }
}

void SNN::getLineX(std::string &line, int target) {
  static std::string file_name = cf.INPUT_FILE;
  static std::ifstream file(file_name);
  std::string temp;
  if (!file.is_open()) {
    lg.log(ERROR, "get_next_line: Unable to open file");
    return;
  }

  file.seekg(std::ios::beg);
  for (int i = 1; i < target; i++) {
    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }

  std::getline(file, temp);
  for (char ch : temp) {
    if (ch == ',') {
      continue;
    }
    line += ' ';
    line += ch;
  }
}

void SNN::getNextLine(std::string &line) {
  static std::string file_name = cf.INPUT_FILE;
  static std::ifstream file(file_name);
  std::string temp;

  if (!file.is_open()) {
    lg.log(ERROR, "get_next_line: Unable to open file");
    return;
  }

  // Any previous contents of @a \_\_str are erased.
  std::getline(file, temp);

  for (char ch : temp) {
    if (ch == ',') {
      continue;
    }
    line += ' ';
    line += ch;
  }
}

void SNN::setNextStim() {
  if (input_neurons.empty()) {
    lg.log(ESSENTIAL, "set_next_line: passed empty input neuron vector?");
    return;
  }

  std::string line;
  getNextLine(line);

  std::stringstream s(line);
  double value;

  for (InputNeuron *input_neuron : input_neurons) {
    s >> value;
    input_neuron->set_input_value(value);
  }
}

int SNN::maximum_edges() {
  // for an undirected graph there are n(n-1) / 2 edges
  //
  // Since our connections are only allowed to go one way, the network is
  // essentially an undirected graph.
  //
  // input neurons can only have outgoing edges is
  // the parameter NUMBER_NEURONS represents the total number of neurons
  // then n_t = NUMBER_NEURONS, n_r = NUMBER_NEURONS - NUMBER_INPUT_NEURONS
  // n_i = NUMBER_INPUT_NEURONS
  //
  // the max edges are the total number of maximum edges minus the number of
  // edges that would be possible in a undirected graph of only the input
  // neurons maximum_edges = n_t(n_t) / 2 - n_i(n_i-1) / 2

  int n_i = cf.NUMBER_INPUT_NEURONS;
  int n_t = cf.NUMBER_NEURONS;

  // always even so division is fine
  int edges_lost_to_input = n_i * (n_i - 1) / 2;

  // maximum possible edges
  int max_edges = (n_t * (n_t - 1) / 2) - edges_lost_to_input;

  return max_edges;
}
