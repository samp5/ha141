#include "network.hpp"
#include "file_reader.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include "neuron_group.hpp"
#include "runtime.hpp"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <ios>
#include <pthread.h>
#include <sstream>
#include <vector>

/**
 * @brief Destructor for SNN.
 *
 * Deletes each group and destroys pthread objects
 *
 */
SNN::~SNN() {
  for (auto group : groups) {
    if (group) {
      delete group;
      group = nullptr;
    }
  }

  mutex->destroy_mutexes();
  pthread_cond_destroy(&stimulus_switch_cond);
  pthread_barrier_destroy(&barrier->barrier);
}

/**
 * @brief Constructor for SNN.
 *
 * Allocate all `NeuronGroup`s which in turn allocate all `Neuron`s.
 *
 * @param config RuntimConfig MUST be generated before calling this constructor
 * \sa RuntimConfig
 */
SNN::SNN(std::vector<std::string> args) : active(false) {
  lg = new Log(this);
  config = new RuntimConfig(this);
  config->parseArgs(args);
  config->checkStartCond();
  srand(config->RAND_SEED);
  mutex = new Mutex;
  // number of group threads plus the main thread
  barrier = new Barrier(config->NUMBER_GROUPS + 1);

  if (Image::isSquare(config->NUMBER_INPUT_NEURONS)) {
    lg->log(ESSENTIAL, "Assuming square input image");
    image = new Image(config->NUMBER_INPUT_NEURONS);
  } else {
    lg->log(ESSENTIAL,
            "Input image not square, using smallest perimeter rectangle");

    auto dimensions = Image::bestRectangle(config->NUMBER_INPUT_NEURONS);
    int x = dimensions.first;
    int y = dimensions.second;
    image = new Image(x, y);
  }

  int neuron_per_group = config->NUMBER_NEURONS / config->NUMBER_GROUPS;
  int input_neurons_per_group =
      config->NUMBER_INPUT_NEURONS / config->NUMBER_GROUPS;

  for (int i = 0; i < config->NUMBER_GROUPS; i++) {

    // allocate for this group
    NeuronGroup *this_group =
        new NeuronGroup(i + 1, neuron_per_group, input_neurons_per_group, this);

    // add to vector
    groups.push_back(this_group);
  }
  generateNeuronVec();
  generateInputNeuronVec();
  setInputNeuronLatency();
}

/**
 * @brief Generate a vector of all `InputNeuron`.
 *
 */
void SNN::generateInputNeuronVec() {
  if (!input_neurons.empty()) {
    lg->log(ERROR,
            "get_input_neuron_vector: this constructs a new vector! "
            "emptying vector that was passed...\n If Neurons in this vector "
            "were dynamically allocated, that memory has NOT been freed");
    input_neurons.clear();
  }

  for (const auto &group : groups) {
    for (auto neuron : group->getMutNeuronVec()) {
      if (neuron->getType() != Input) {
        continue;
      }
      input_neurons.push_back(dynamic_cast<InputNeuron *>(neuron));
    }
  }
}

/**
 * @brief Sets the latency for all input neurons.
 *
 * The latency for an input neuron is based on its position in the
 * SNN::input_neurons reinterpretted as a 2D array.
 *
 * \sa Image::calculateCoords
 */
void SNN::setInputNeuronLatency() {
  for (std::vector<InputNeuron *>::size_type i = 0; i < input_neurons.size();
       i++) {
    double latency = image->getLatency(i);
    input_neurons.at(i)->setLatency(latency);
  }
}

/**
 * @brief Generate a vector of all `Neuron`s.
 *
 */
void SNN::generateNeuronVec() {
  for (auto group : groups) {
    for (auto neuron : group->getMutNeuronVec()) {
      neurons.push_back(neuron);
    }
  }
}

/**
 * @brief Generate a map associating `Neuron`s with all possible connections.
 *
 * The following restrictions are applied to Synapse formation
 * - `InputNeuron`s cannot have incoming connections
 * - There are no reflextive or symmetric edges allowed
 *    - i.e. No self-connections or connections between
 *      Neurons for which a connection already exists
 *
 *
 *
 */
void SNN::generateNeighborOptions(
    std::unordered_map<Neuron *, std::list<Neuron *>> &map) {
  for (auto neuron_origin : neurons) {
    std::list<Neuron *> origin_list;
    for (auto neuron_destination : neurons) {
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
}

/**
 * @brief generate Synapse connections between all `Neuron`s in SNN::groups.
 *
 * Add an amount of `Synapse`s consistent with RuntimConfig::NUMBER_EDGES
 * according to restrictions placed upon viable connections
 *
 *
 */
void SNN::generateRandomSynapsesAdjMatrix() {
  auto start = lg->time();
  if (neurons.empty()) {
    generateNeuronVec();
  }
  auto num_n = neurons.size();

  // find non input neurons
  std::vector<Neuron *> nonInput(config->NUMBER_NEURONS -
                                 config->NUMBER_INPUT_NEURONS);
  std::vector<Neuron *>::size_type i = 0;
  for (auto n : neurons) {
    if (n->getType() != Neuron_t::Input) {
      nonInput.at(i) = n;
      i++;
    }
  }

  std::size_t non_input_count =
      config->NUMBER_NEURONS - config->NUMBER_INPUT_NEURONS;

  // Initialize adjacency matrix
  typedef std::vector<std::vector<int>> Matrix;
  Matrix mat(num_n);
  for (Matrix::size_type i = 0; i < num_n; i++) {
    mat.at(i) = std::vector<int>(non_input_count);
  }

  int number_connections = 0;
  while (number_connections < config->NUMBER_EDGES) {
    Matrix::size_type row = rand() % config->NUMBER_NEURONS;
    Matrix::size_type col = rand() % non_input_count;

    if (mat.at(row).at(col) || row == col) {
      continue;
    } else {
      mat.at(row).at(col) = 1;
      if (row < non_input_count) {
        mat.at(col).at(row) = -1;
      }
      number_connections += 1;
    }
  }

  // Add normal neurons
  for (std::size_t r = 0; r < non_input_count; r++) {
    Neuron *origin = nonInput.at(r);
    for (std::size_t c = 0; c < non_input_count; c++) {
      if (mat.at(r).at(c) == 1) {
        origin->addNeighbor(nonInput.at(c), generateSynapseWeight());
      }
    }
  }
  // Add connections for input neurons
  for (std::size_t r = non_input_count; r < num_n; r++) {
    InputNeuron *origin = input_neurons.at(r - non_input_count);
    for (std::size_t c = 0; c < non_input_count; c++) {
      if (mat.at(r - non_input_count).at(c) == 1) {
        origin->addNeighbor(nonInput.at(c), generateSynapseWeight());
      }
    }
  }
  auto end = lg->time();
  std::string msg = "Adding random synapses done: took " +
                    std::to_string(end - start) + " seconds";
  lg->log(ESSENTIAL, msg.c_str());
}

/**
 * @brief generate Synapse connections between all `Neuron`s in SNN::groups.
 *
 * Add an amount of `Synapse`s consistent with RuntimConfig::NUMBER_EDGES
 * according to restrictions placed upon viable connections in
 * SNN::generateNeighborOptions
 *
 * \sa SNN::generateNeighborOptions
 *
 */
void SNN::generateRandomSynapses() {
  auto start = lg->time();
  int synapses_formed = 0;
  if (neurons.empty()) {
    generateNeuronVec();
  }
  std::unordered_map<Neuron *, std::list<Neuron *>> map;
  generateNeighborOptions(map);

  float progress = 0.0;
  int pos = 0;
  while (synapses_formed < config->NUMBER_EDGES) {
    int bar_width = 50;
    progress = (float)synapses_formed / config->NUMBER_EDGES;
    if (int(progress * bar_width) > pos + 5) {
      std::cout << "[";
      pos = progress * bar_width;
      for (int i = 0; i < bar_width; i++) {
        if (i < pos) {
          std::cout << "=";
        } else if (i == pos) {
          std::cout << ">";
        } else {
          std::cout << " ";
        }
      }
      std::cout << "]" << int(progress * 100.0) << "%\n";
    }

    for (auto neuron : neurons) {
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
      neuron->addNeighbor(*target, generateSynapseWeight());

      // erase the postsynaptic neuron from the list and this neuron from the
      // postsynaptic list
      neighbor_options.erase(target);
      if (this_neuron != post_opts.end()) {
        post_opts.erase(this_neuron);
      }

      // increment the synapses
      synapses_formed += 1;

      // check to make sure we still need edges
      if (synapses_formed >= config->NUMBER_EDGES) {
        break;
      }
    }
  }
  auto end = lg->time();
  std::string msg = "Adding random synapses done: took " +
                    std::to_string(end - start) + " seconds";
  lg->log(ESSENTIAL, msg.c_str());
}

/**
 * @brief Generate a random weight.
 */
double SNN::generateSynapseWeight() {
  return ((double)rand() / RAND_MAX) * 0.1;
}

/**
 * @brief Join thread.
 */
void SNN::join() {
  for (auto group : groups) {
    pthread_join(group->getThreadID(), NULL);
  }
}

/**
 * @brief Reset the Network.
 *
 * Calls NeuronGroup::reset which in turn calls Neuron::reset
 *
 */
void SNN::reset() {
  for (auto group : groups) {
    group->reset();
  }
}

/**
 * @brief Start network.
 *
 * Set stimulus values of all `InputNeuron`s and starts threads for
 * each NeuronGroup. Cycle through each stimulus, reseting the NeuronGroup after
 * stimulus.
 *
 */
void SNN::start() {
  active = true;

  setNextStim();
  lg->value(ESSENTIAL, "Set stimulus to line %d", *config->STIMULUS);

  for (auto group : groups) {
    group->startThread();
  }

  for (int i = 1; i < config->num_stimulus + 1; i++) {

    usleep(config->time_per_stimulus);

    if (i < config->num_stimulus) {
      auto start = std::chrono::high_resolution_clock::now();
      switching_stimulus = true;

      pthread_barrier_wait(&getBarrier()->barrier);

      setNextStim();
      config->STIMULUS++;
      lg->value(ESSENTIAL, "Set stimulus to line %d", *config->STIMULUS);

      reset();

      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(
          end - start);
      lg->addOffset(duration.count());

      stimlus_start = lg->time();
      switching_stimulus = false;
      pthread_cond_broadcast(&stimulus_switch_cond);
    }
  }
  active = false;
}

/**
 * @brief Set stimulus to the next line of RuntimConfig::INPUT_FILE.
 *
 */
void SNN::setNextStim() {
  static FileReader reader =
      FileReader(config->INPUT_FILE, config->STIMULUS_VEC.front());

  if (input_neurons.empty()) {
    lg->log(ESSENTIAL, "set_next_line: passed empty input neuron vector?");
    return;
  }

  std::string line = reader.nextLine();

  std::stringstream s(line);
  double value;

  for (InputNeuron *input_neuron : input_neurons) {
    s >> value;
    input_neuron->setInputValue(value);
  }
}

/**
 * @brief  return maximum number of edges.
 *
 * Find the maximum possible edges in a graph with
 * RuntimConfig::NUMBER_NEURONS and RuntimConfig::NUMBER_INPUT_NEURONS
 *
 * @return Number of edges
 */
int SNN::maximum_edges(int num_input, int num_n) {
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

  int n_i = num_input;
  int n_t = num_n;

  // always even so division is fine
  int edges_lost_to_input = n_i * (n_i - 1) / 2;

  // maximum possible edges
  int max_edges = (n_t * (n_t - 1) / 2) - edges_lost_to_input;

  return std::floor(max_edges / 4);
}

/**
 * @brief Generate a neuron string for SNN::generateGraphiz.
 *
 * Input neurons get I{groupID}_{neuronID}
 * Regular neurons get n{groupID}_{neuronID}
 *
 * @param n pointer to neuron
 * @return Formatted string
 */
std::string generate_neuron_string(Neuron *n) {
  std::string str =
      std::to_string(n->getGroup()->getID()) + "_" + std::to_string(n->getID());
  if (n->getType() == Neuron_t::Input) {
    str = "I" + str;
  } else {
    str = "n" + str;
  }
  return str;
}

/**
 * @brief Genearte a graphiz formatted file to display network.
 *
 */
void SNN::generateGraphiz(bool weights) {
  std::ofstream file("network.dot");
  file << "digraph {\n";
  if (!weights) {
    for (auto n : neurons) {
      std::string fstr = generate_neuron_string(n);
      std::vector<std::string> to;
      for (auto s : n->getPostSynaptic()) {
        to.push_back(generate_neuron_string(s->getPostSynaptic()));
      }

      if (n->getType() == Neuron_t::Input) {
        file << fstr << " [fontcolor=green]\n";
      }

      file << fstr << " -> { ";
      for (auto s : to) {
        file << s << " ";
      }
      file << "}\n";
    }

  } else {
    for (auto n : neurons) {
      std::string fstr = generate_neuron_string(n);
      if (n->getType() == Neuron_t::Input) {
        file << fstr << " [fontcolor=green]\n";
      }

      for (auto t : n->getPostSynaptic()) {
        std::string tstr = generate_neuron_string(t->getPostSynaptic());
        file << fstr << " -> " << tstr << " [label= " << std::setprecision(3)
             << t->getWeight() << " ]\n";
      }
    }
  }
  file << "}";
}
