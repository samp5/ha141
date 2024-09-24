#include "network.hpp"
#include "file_reader.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include "neuron_group.hpp"
#include "runtime.hpp"
#include <algorithm>
#include <asm-generic/ioctls.h>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <pthread.h>
#include <random>
#include <sstream>
#include <string>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
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
  delete lg;
  delete config;
  delete image;

  mutex->destroy_mutexes();
  delete mutex;

  pthread_barrier_destroy(&barrier->barrier);
  delete barrier;

  pthread_cond_destroy(&stimulus_switch_cond);
}

/**
 * @brief Constructor for SNN.
 *
 * Allocate all `NeuronGroup`s which in turn allocate all `Neuron`s.
 *
 * @param config RuntimConfig MUST be generated before calling this constructor
 * \sa RuntimConfig
 */
SNN::SNN(std::vector<std::string> args) {
  lg = new Log(this);
  config = new RuntimConfig(this);
  config->parseArgs(args);
  config->checkStartCond();
  srand(config->RAND_SEED);
  mutex = new Mutex;
  // number of group threads plus the main thread
  barrier = new Barrier(config->NUMBER_GROUPS + 1);
  inputFileReader =
      new InputFileReader(config->INPUT_FILE, config->STIMULUS_VEC.front());

  gen = std::mt19937(rd());
  gen.seed(config->RAND_SEED);

  if (Image::isSquare(config->NUMBER_INPUT_NEURONS)) {
    lg->log(ESSENTIAL, "Assuming square input image");
    image = new Image(config->NUMBER_INPUT_NEURONS, config->max_latency);
  } else {
    lg->log(ESSENTIAL,
            "Input image not square, using smallest perimeter rectangle");

    auto dimensions = Image::bestRectangle(config->NUMBER_INPUT_NEURONS);
    int x = dimensions.first;
    int y = dimensions.second;
    image = new Image(x, y, config->max_latency);
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
  generateAllNeuronVec();
  generateNonInputNeuronVec();
  generateInputNeuronVec();
  setInputNeuronLatency();
}

// default constructor for pySNN
SNN::SNN() { lg = new Log(NULL); }

void SNN::initializeFromSynapseFile(const std::vector<std::string> &args,
                                    const std::string &adjListFile) {
  lg->setNetwork(this);
  config = new RuntimConfig(this);
  config->parseArgs(args);
  config->checkStartCond();
  mutex = new Mutex;
  // number of group threads plus the main thread
  barrier = new Barrier(config->NUMBER_GROUPS + 1);

  inputFileReader =
      new InputFileReader(config->INPUT_FILE, config->STIMULUS_VEC.front());

  gen = std::mt19937(rd());
  gen.seed(config->RAND_SEED);

  if (Image::isSquare(config->NUMBER_INPUT_NEURONS)) {
    lg->log(ESSENTIAL, "Assuming square input image");
    image = new Image(config->NUMBER_INPUT_NEURONS, config->max_latency);
  } else {
    lg->log(ESSENTIAL,
            "Input image not square, using smallest perimeter rectangle");

    auto dimensions = Image::bestRectangle(config->NUMBER_INPUT_NEURONS);
    int x = dimensions.first;
    int y = dimensions.second;
    image = new Image(x, y, config->max_latency);
  }

  AdjListParser::AdjListInfo adjListInfo;
  getAdjancyListInfo(adjListFile, adjListInfo);
  lg->value(DEBUG4, "adjListInfo numberNodes is %d", adjListInfo.numberNodes);
  lg->value(DEBUG4, "adjListInfo numberEdges is %d", adjListInfo.numberEdges);

  // The total number of neurons will be the number of Nodes from our Synapse
  // file plus the number of specified input neurons
  config->NUMBER_NEURONS =
      adjListInfo.numberNodes + config->NUMBER_INPUT_NEURONS;
  lg->value(DEBUG4, "NUMBER_NEURONS now set to %d", config->NUMBER_NEURONS);

  int neuron_per_group = config->NUMBER_NEURONS / config->NUMBER_GROUPS;
  int input_neurons_per_group =
      config->NUMBER_INPUT_NEURONS / config->NUMBER_GROUPS;

  /*
   * The relation between the index of the neuron in the SNN::neurons and Group
   * is important. The index returned by the synapse file maps to the index in
   * SNN::nonInputNeurons, which maps to the order in which the group is added.
   *
   * SNN::groups
   *         _______________
   *        |  1 |  2  | 3  |
   *         ¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯
   * NeuronGroup::neurons, see that these include InputNeurons!
   *           ____      _____      ____
   *      G1  |0123| G2 |01234| G2 |0123|
   *           ¯¯¯¯      ¯¯¯¯¯      ¯¯¯¯
   * So we introduce SNN::nonInputNeurons (index)
   *         _________
   *        |012345678|
   *         ¯¯¯¯¯¯¯¯¯
   * This matches with the index reported in the keys of
   * AdjListParser::AdjListInfo::AdjList
   */
  for (int i = 0; i < config->NUMBER_GROUPS; i++) {
    // allocate for this group
    NeuronGroup *this_group =
        new NeuronGroup(i + 1, neuron_per_group, input_neurons_per_group, this);

    // add to vector
    groups.push_back(this_group);
  }
  // Generate vectors differentiated by type
  generateAllNeuronVec();
  generateInputNeuronVec();
  generateNonInputNeuronVec();

  // Generate synapses
  generateSynapsesFromAdjList(adjListInfo.adjList);

  // Set latency
  setInputNeuronLatency();
}

void SNN::generateNonInputNeuronVec() {
  if (!nonInputNeurons.empty()) {
    lg->log(ERROR,
            "generateNonInputNeuronVec: this constructs a new vector! "
            "emptying vector that was passed...\n If Neurons in this vector "
            "were dynamically allocated, that memory has NOT been freed");
    nonInputNeurons.clear();
  }

  for (const auto &group : groups) {
    for (auto neuron : group->getMutNeuronVec()) {
      if (neuron->getType() == Input) {
        continue;
      }
      nonInputNeurons.push_back(neuron);
    }
  }
}
/**
 * @brief Generate a vector of all `InputNeuron`.
 *
 */
void SNN::generateInputNeuronVec() {
  if (!input_neurons.empty()) {
    lg->log(ERROR,
            "generateInputNeuronVec: this constructs a new vector! "
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
    int latency = image->getLatency(i);
    input_neurons.at(i)->setLatency(latency);
  }
}

/**
 * @brief Generate a vector of all `Neuron`s.
 *
 */
void SNN::generateAllNeuronVec() {
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

struct gRSAMGS_ThreadArgs {
  SNN *snn;
  std::vector<NeuronGroup *>::size_type index;
  int edges;
  gRSAMGS_ThreadArgs(SNN *n, std::vector<NeuronGroup *>::size_type i, int e)
      : snn(n), index(i), edges(e) {}
};

void *SNN::generateRandomSynapsesAdjMatrixGS_Helper(void *arg) {
  gRSAMGS_ThreadArgs *args = static_cast<gRSAMGS_ThreadArgs *>(arg);
  long edges =
      args->snn->groups.at(args->index)->generateRandomSynapses(args->edges);
  delete args;
  pthread_exit((void *)edges);
}
/**
 * @brief generate Synapse connections between all `Neuron`s in SNN::groups.
 *
 * Add an amount of `Synapse`s consistent with RuntimConfig::NUMBER_EDGES
 * according to restrictions placed upon viable connections
 *
 *
 */
void SNN::generateRandomSynapsesAdjMatrixGS() {
  auto start = lg->time();
  int edges_per_group = config->NUMBER_EDGES / config->NUMBER_GROUPS;
  int rem = config->NUMBER_EDGES % config->NUMBER_GROUPS;

  // Generate intraGroup connections
  typedef std::vector<NeuronGroup *>::size_type vecSz;
  pthread_t *threads = new pthread_t[config->NUMBER_GROUPS];
  for (vecSz i = 0; i < groups.size(); i++) {
    int edges = rem ? edges_per_group + 1 : edges_per_group;
    if (rem > 0)
      rem--;
    gRSAMGS_ThreadArgs *arg = new gRSAMGS_ThreadArgs(this, i, edges);
    pthread_create(&threads[i], nullptr,
                   SNN::generateRandomSynapsesAdjMatrixGS_Helper,
                   static_cast<void *>(arg));
  }

  // Calculate how many edges were formed total
  void *ret = NULL;
  int intragroup_formed = 0;
  for (vecSz i = 0; i < groups.size(); i++) {
    pthread_join(threads[i], &ret);
    intragroup_formed += (long)ret;
  }
  delete[] threads;

  // To be formed
  // We want to minmize this so we divide by intragroup_formed
  int intergroup_edges = std::floor((config->NUMBER_EDGES - intragroup_formed) /
                                    intragroup_formed);

  if (intergroup_edges > 0) {
    std::uniform_int_distribution<> group(0, groups.size() - 1);
    int intergroup_formed = 0;
    while (intergroup_formed < intergroup_edges) {
      vecSz from = group(gen);
      vecSz to = group(gen);
      if (from == to) {
        continue;
      }
      auto origin = groups[from]->getRandNeuron();
      auto destination = groups[to]->getNonInputNeuron();
      origin->addIGNeighbor(destination);

      // increment intergroup edges
      intergroup_formed += 1;
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
 * according to restrictions placed upon viable connections
 *
 *
 */
void SNN::generateRandomSynapsesAdjMatrix() {
  auto start = lg->time();
  if (neurons.empty()) {
    generateAllNeuronVec();
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
        origin->addNeighbor(nonInput.at(c));
      }
    }
  }
  // Add connections for input neurons
  for (std::size_t r = non_input_count; r < num_n; r++) {
    InputNeuron *origin = input_neurons.at(r - non_input_count);
    for (std::size_t c = 0; c < non_input_count; c++) {
      if (mat.at(r - non_input_count).at(c) == 1) {
        origin->addNeighbor(nonInput.at(c));
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
    generateAllNeuronVec();
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
      neuron->addNeighbor(*target);

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
  gen.seed(config->RAND_SEED);
}

/**
 * @brief Resets the network to a fresh state, as if it had encountered no
 * input.
 *
 * Removes all log data. If the log data is needed from the previous batch,
 * collect it before calling this function
 *
 */
void SNN::batchReset() {
  // normal reset
  reset();

  // delete all log data
  lg->batchReset();

  // clear stim vec
  config->STIMULUS_VEC.clear();
  config->STIMULUS = config->STIMULUS_VEC.end();
}

void SNN::runChildProcess(const std::vector<int> &stimulus, int fd) {
  // lg->value(LogLevel::INFO, "Child process running, PID: %d",
  //           static_cast<int>(getpid()));
  config->STIMULUS = stimulus.begin();
  // lg->value(LogLevel::DEBUG, "Child Process: stimulus set to line %d",
  //           *config->STIMULUS);
  config->num_stimulus = stimulus.size();
  inputFileReader->setToLine(*config->STIMULUS);
  setNextStim();
  generateInputNeuronEvents();

  for (int i = 1; i < config->num_stimulus + 1; i++) {
    for (auto group : groups) {
      group->startThread();
    }
    for (auto group : groups) {
      pthread_join(group->getThreadID(), NULL);
    }
    if (i < config->num_stimulus) {
      config->STIMULUS++;
      setNextStim();
      lg->value(LogLevel::DEBUG4, "Child Process: stimulus set to line %d",
                *config->STIMULUS);
      reset();
      generateInputNeuronEvents();
    }
  }
  lg->writeToFD(fd, groups);
  exit(EXIT_SUCCESS);
}

void SNN::forkRun(const std::vector<std::vector<int>> &stimulusBatches) {
  config->STIMULUS_VEC.clear();
  std::vector<pid_t> children;
  std::vector<int *> pipes;

  // All the stimulus "lines" get inserted into the stimulus vector
  // for the following reason:
  //
  // When we access activation data, the line number passed as the
  // stimulus number is connected to the activation log. The way the
  // max and min stimulus number is determined is by the max and min element
  // in the config->STIMULUS_VEC

  for (size_t i = 0; i < stimulusBatches.size(); i++) {
    config->STIMULUS_VEC.insert(config->STIMULUS_VEC.end(),
                                stimulusBatches.at(i).begin(),
                                stimulusBatches.at(i).end());
    int *pipefd = (int *)malloc(sizeof(int) * 2);
    int r = pipe(pipefd);
    if (r == -1) {
      lg->log(LogLevel::ERROR, "SNN::forkRun: pipe failed to create pipe "
                               "file descriptors, pipe() returned -1");
      lg->string(LogLevel::ERROR, "erno reports %s", strerror(errno));
    }
    pipes.push_back(pipefd);
  }
  for (size_t i = 0; i < stimulusBatches.size(); i++) {
    pid_t cPID = fork();
    switch (cPID) {
    case -1: // error state
      lg->value(LogLevel::ERROR,
                "SNN::forkRun: failed to spawn child process, fork() "
                "returned -1, batch with first stimulus of line %d NOT run",
                stimulusBatches.at(i).front());
      lg->string(LogLevel::ERROR, "erno reports %s", strerror(errno));
      break;
    case 0: {                    // child process
      int *pipefd = pipes.at(i); // get the corresponding pipe array
      close(pipefd[0]);          // close the read end
      runChildProcess(stimulusBatches.at(i), pipefd[1]);
      break;
    }
    default: // parent process
      children.push_back(cPID);
      break;
    }
  }
  auto start = std::chrono::high_resolution_clock::now();
  forkRead(children, pipes);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::cout << "forkread time elapsed is " << elapsed.count() << "\n";
}

void setNonBlocking(int fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void SNN::forkRead(std::vector<pid_t> &childrenPIDs,
                   std::vector<int *> &pipes) {
  bool done = false;
  for (auto pipefd : pipes) {
    setNonBlocking(pipefd[0]);
    close(pipefd[1]); // close write pipe
  }

  while (!done) {
    for (size_t i = 0; i < childrenPIDs.size(); i++) {
      // Ignore completed processes
      pid_t cPID = childrenPIDs.at(i);
      if (cPID == -1) {
        continue;
      }

      int *pipefd = pipes.at(i); // get corresponding pipe
      LogData4_t buf;

      while (read(pipefd[0], &buf, sizeof(LogData4_t)) > 0) {
        LogData *toAdd = new LogData(buf);
        lg->addData(toAdd);
      }

      int wstatus;
      if (waitpid(cPID, &wstatus, WNOHANG)) {
        close(pipefd[0]);
        childrenPIDs.at(i) = -1;
      }
    }
    done = std::all_of(childrenPIDs.begin(), childrenPIDs.end(),
                       [](pid_t x) { return x == -1; });
  }

  for (auto pPipeArr : pipes) {
    free(pPipeArr);
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

  setNextStim();
  generateInputNeuronEvents();
  lg->value(LogLevel::INFO, "InputNeuronEvents Generated, size %d",
            config->INPUT_PROB_SUCCESS * config->time_per_stimulus);

  if (config->show_stimulus) {
    lg->value(ESSENTIAL, "Set stimulus to line %d", *config->STIMULUS);
  }

  float progress = 0.0;
  int pos = 0;

  for (int i = 1; i < config->num_stimulus + 1; i++) {
    if (!config->show_stimulus) {
      int bar_width = 50;
      progress = (float)i / config->num_stimulus;
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
    }

    lg->log(LogLevel::INFO, "Starting Groups");
    for (auto group : groups) {
      group->startThread();
    }
    for (auto group : groups) {
      pthread_join(group->getThreadID(), NULL);
    }
    if (i < config->num_stimulus) {
      config->STIMULUS++;
      setNextStim();
      if (config->show_stimulus) {
        lg->value(ESSENTIAL, "Set stimulus to line %d", *config->STIMULUS);
      }
      lg->log(LogLevel::INFO, "Resetting Network");
      reset();
      generateInputNeuronEvents();
      lg->value(LogLevel::INFO, "InputNeuronEvents Generated, size %d",
                config->INPUT_PROB_SUCCESS * config->time_per_stimulus);
    }
  }
  for (auto group : groups) {
    for (auto n : group->getMutNeuronVec()) {
      n->transferData();
    }
  }
}

/**
 * @brief Set stimulus to the next line of RuntimConfig::INPUT_FILE.
 *
 */
void SNN::setNextStim() {
  if (input_neurons.empty()) {
    lg->log(ESSENTIAL, "set_next_line: passed empty input neuron vector?");
    return;
  }

  std::string line = inputFileReader->nextLine();

  std::stringstream s(line);
  long double value;
  char discard;

  for (InputNeuron *input_neuron : input_neurons) {
    s >> value >> discard;
    long double set = value;
    input_neuron->setInputValue(set);
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

  return std::floor(max_edges / 2);
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

/**
 * @brief Generate InputNeuron events.
 *
 * For all input neurons, generate INPUT_PROB_SUCCESS * time_per_stimulus
 * events. For each simulus the timestamps for all InputNeuron are the same For
 * each stimulus the timestamps are random.
 *
 *
 * i.e.
 * time_per_stimulus = 10
 * INPUT_PROB_SUCCESS = 0.5;
 *
 * Stimulus 1: all InputNeuron recieve something like
 * { 1, 4, 7, 8, 9}
 *
 * Stimulus 2: all InputNeuron recieve something like
 * { 2, 3, 6, 9}
 */
void SNN::generateInputNeuronEvents() {

  int num_events = config->INPUT_PROB_SUCCESS * config->time_per_stimulus;

  std::vector<int> timestamps(num_events);

  for (int i = 0; i < num_events; i++) {
    timestamps.at(i) = std::abs(getRandom()) % config->time_per_stimulus;
  }

  for (auto in : input_neurons) {
    if (in->getInputValue() < 0.00001) {
      continue;
    }
    in->generateEvents(timestamps);
  }
}

int SNN::generateCSV() {
  std::sort(config->STIMULUS_VEC.begin(), config->STIMULUS_VEC.end());
  int max_stim = config->STIMULUS_VEC.back();
  int min_stim = config->STIMULUS_VEC.front();
  int totalActivations = 0;

  std::unordered_map<int, std::vector<LogData *>> stim_data;
  const std::vector<LogData *> &lg_data = lg->getLogData();

  for (std::vector<LogData *>::size_type i = 0; i < lg_data.size(); i++) {
    LogData *td = lg_data.at(i);
    if (td->message_type == Message_t::Refractory) {
      totalActivations++;
      if (stim_data.find(td->stimulus_number) != stim_data.end()) {
        stim_data[td->stimulus_number].push_back(td);
      } else {
        stim_data[td->stimulus_number] = {td};
      }
    }
  }

  int bins = config->time_per_stimulus;
  std::vector<std::vector<int>> ret(max_stim - min_stim + 1);

  for (int s = min_stim; s <= max_stim; s++) {
    // if the key does not exist (there were no activations for this stimulus)
    if (stim_data.find(s) == stim_data.end()) {
      ret.at(s - min_stim) = std::vector<int>(bins, 0);
      continue;
    }
    std::vector<LogData *> &sd = stim_data[s];

    std::sort(sd.begin(), sd.end(), [](LogData *a, LogData *b) {
      return a->timestamp < b->timestamp;
    });

    double timestep =
        (double)(sd.back()->timestamp - sd.front()->timestamp) / bins;
    double l = sd.front()->timestamp;
    double u = l + timestep;

    std::vector<int> row(bins);
    for (int i = 0; i < bins; i++) {
      row.at(i) = std::count_if(sd.begin(), sd.end(), [l, u](LogData *ld) {
        return (ld->timestamp < u) && (ld->timestamp >= l);
      });
      l = u;
      u = u + timestep;
    }
    ret.at(s - min_stim) = row;
  }
  lg->writeCSV(ret);
  this->totalActivations = totalActivations;
  return totalActivations;
}

void SNN::generateSynapsesFromAdjList(const AdjListParser::AdjList &adjList) {
  int numEdges = 0;
  for (auto pair : adjList) {
    size_t originIndex = pair.first;
    Neuron *origin = nonInputNeurons.at(originIndex);
    for (auto destinationIndex : pair.second) {
      origin->addNeighbor(nonInputNeurons.at(destinationIndex));
      numEdges++;
    }
  }
  size_t minIndex = std::min(input_neurons.size(), nonInputNeurons.size());
  for (size_t i = 0; i < minIndex; i++) {
    InputNeuron *origin = input_neurons.at(i);
    Neuron *destination = nonInputNeurons.at(i);
    origin->addNeighbor(destination);
    numEdges++;
  }
  config->NUMBER_EDGES = numEdges;
}

void SNN::getAdjancyListInfo(const std::string &file_path,
                             AdjListParser::AdjListInfo &info) {
  AdjListParser parser(file_path);
  info = parser.parseAdjList();
}
