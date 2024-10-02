#include "snn.hpp"
#include "../../extern/pybind/include/pybind11/stl.h"
#include "../runtime.hpp"
#include <algorithm>
#include <chrono>
#include <climits>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <pthread.h>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <unordered_map>
#include <vector>

int add(int i, int j) { return i + j; }

pySNN::pySNN(ConfigDict dict) : SNN(), configDict(dict) {
  lg->setNetwork(this); // log gets allocated in SNN()
  config = new RuntimConfig(this);
  if (configDict.empty()) {
    configDict = getDefaultConfig();
  }
  config->setOptions(configDict);
  mutex = new Mutex;

  barrier = new Barrier(config->NUMBER_GROUPS + 1);
  // no need for input file reader

  gen = std::mt19937(rd());
  gen.seed(config->RAND_SEED);
  image = nullptr; // we have to wait to initalize the images until we know
  // size of the python buffer
}

ConfigDict pySNN::getDefaultConfig() {

  ConfigDict dict = {{"neuron_count", 0},
                     {"input_neuron_count", 0},
                     {"group_count", 1},
                     {"edge_count", 0},
                     {"refractory_duration", 5},
                     {"initial_membrane_potential", -6.0},
                     {"activation_threshold", -5.0},
                     {"refractory_membrane_potential", -7.0},
                     {"tau", 100.0},
                     {"max_latency", 10},
                     {"max_synapse_delay", 2},
                     {"min_synapse_delay", 1},
                     {"max_weight", 10.0},
                     {"poisson_prob_of_success", 0.8},
                     {"debug_level", LogLevel::NONE},
                     {"limit_log_size", true},
                     {"show_stimulus", false},
                     {"time_per_stimulus", 200},
                     {"seed", -1}};
  return dict;
}

pySNN::pySNN(std::string configFile) : SNN() {
  lg->setNetwork(this); // log gets allocated in SNN()
  config = new RuntimConfig(this);
  if (configFile == "") {
    config->parseArgs({""});
  } else {
    config->parseArgs({"", configFile});
  }
  config->checkStartCond();
  mutex = new Mutex;

  barrier = new Barrier(config->NUMBER_GROUPS + 1);
  // no need for input file reader

  gen = std::mt19937(rd());
  gen.seed(config->RAND_SEED);
  image = nullptr; // we have to wait to initalize the images until we know
  // the
  //  size of the python buffer
}

pySNN::pySNN(std::vector<std::string> args) : SNN() {
  lg->setNetwork(this); // log gets allocated in SNN()
  config = new RuntimConfig(this);
  config->parseArgs(args);
  config->checkStartCond();
  mutex = new Mutex;

  barrier = new Barrier(config->NUMBER_GROUPS + 1);
  // no need for input file reader

  gen = std::mt19937(rd());
  gen.seed(config->RAND_SEED);
  image = nullptr; // we have to wait to initalize the images until we know the
                   // size of the python buffer
}

/**
 * @brief Transform the coordinate of a 2D array into a 1D index.
 *
 *
 * Returns the index as if the rows of the 2D array were laid end to end
 *
 * Given (x,y) such that (x, y) identify a point in an n by m matrix,
 * return an index of a 1D matrix of n * m elements.
 *
 * (0,0) => 0
 * (m, n) => m*n - 1.
 *
 * For (a, b) => j, (c,d) => k, if a > c, j > k.
 *
 * @param pair coordinate (x,y) in a tuple
 * @param maxLayer the value of m (number of rows)
 * @return index of the 1D array
 */
int getIndex(const std::tuple<int, int> &pair, int maxLayer) {
  int x = std::get<0>(pair);
  int y = std::get<1>(pair);

  int index = x + y * maxLayer;
  return index;
}

/**
 * @brief update the Edge weights based on a dict of dicts.
 *
 * @param dict a AdjDict representing the graph
 *
 */
void pySNN::updateEdgeWeights(AdjDict dict) {

  using std::get;

  for (const auto &adjacencyPair : dict) {

    int originIndex = getIndex(adjacencyPair.first, maxLayer);

    const auto &synapseRef =
        nonInputNeurons.at(originIndex)
            ->getPostSynaptic(); // get outgoing connections

    for (const auto &edgeWeightPair : adjacencyPair.second) {

      int destinationIndex = getIndex(edgeWeightPair.first, maxLayer);
      double weight = 1;
      if (edgeWeightPair.second.count("weight")) {
        weight = edgeWeightPair.second.at("weight");
      }
      int delay = 1;
      if (edgeWeightPair.second.count("delay")) {
        delay = edgeWeightPair.second.at("delay");
      }

      Neuron *destination = nonInputNeurons.at(destinationIndex);
      bool updated = false;
      for (auto &synapse : synapseRef) {
        if (synapse->getPostSynaptic() == destination) {
          synapse->updateWeight(weight);
          synapse->updateDelay(delay);
          updated = true;
          break;
        }
      }

      if (!updated) {
        lg->log(LogLevel::ERROR,
                "pySNN::updateEdgeWeights: While updating weight, destination "
                "neuron not found in origin neurons edge list");
      }
    }
  }
}

/**
 * @brief Set RuntimConfig::STIMULUS_VEC and RuntimConfig::Stimulus to align
 * with pySNN::data.
 *
 *  description
 *
 */
void pySNN::updateStimulusVectorToBuffDim() {
  config->STIMULUS_VEC.clear();
  std::vector<int>::size_type number_lines = data.size();
  for (std::vector<int>::size_type i = 0; i < number_lines; i++) {
    config->STIMULUS_VEC.push_back(i);
  }
  config->STIMULUS = config->STIMULUS_VEC.begin();
  config->num_stimulus = config->STIMULUS_VEC.size();
}

void pySNN::generateImage() {
  /*
   * The passed buffer (which is at this point stored in pySNN::data
   * Has dimensions m x n, where m is the number of stimulus in this batch
   * and n is the number of input neurons required to observe the stimulus
   *
   * Here, we update the number of inputNeurons (that was previously parsed
   * from the configuraiton file) to match the required number specifed by
   * the passed buffer
   */
  if (data.empty()) {
    lg->log(
        LogLevel::INFO,
        "pySNN::data is empty, trusting "
        "RuntimConfig::NUMBER_INPUT_NEURONS is correct (this value is set by "
        "either configuration file, or overridden by pySNN::initialize)");
  } else if (static_cast<size_t>(config->NUMBER_INPUT_NEURONS) !=
             data.front().size()) {
    lg->value(WARNING,
              "Number of input neurons does not equal the number of "
              "elements per stimulus, setting number of input neurons to %d",
              (int)data.front().size());
    config->NUMBER_INPUT_NEURONS = data.front().size();
  }

  lg->value(LogLevel::INFO, "NUMBER_INPUT_NEURONS is %d",
            config->NUMBER_INPUT_NEURONS);

  /*
   * The number of input neurons affects the calulated latency
   * via Image::getLatency.
   *
   * Test if the number of input neurons is a perfect square
   * otherwise using a rectangle with the smallest possible perimeter
   */
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
}

void pySNN::updateConfigToAdjList(const AdjDict &dict) {
  /*
   * The passed dictionary determines the number of neurons
   * (as opposed to the configuration file).
   *
   * Here we update those values and associated values
   */
  int numberNonInput = dict.size();

  lg->value(LogLevel::INFO, "numberNonInput is %d", numberNonInput);

  config->NUMBER_NEURONS = config->NUMBER_INPUT_NEURONS + numberNonInput;

  lg->value(LogLevel::DEBUG, "NUMBER_NEURONS is %d", config->NUMBER_NEURONS);
}

void pySNN::initialize(AdjDict &dict) {
  /*
   * Generate and images based on the buffer dimensions
   */
  generateImage();

  /*
   * The graph generation is completely decided based on the passed
   * NetworkX dict of dicts.
   *
   * Update the Neuron counts based on that dictionary
   */
  updateConfigToAdjList(dict);

  // Neurons per group
  int neuronPerGroup = config->NUMBER_NEURONS / config->NUMBER_GROUPS;
  int inputNeuronPerGroup =
      config->NUMBER_INPUT_NEURONS / config->NUMBER_GROUPS;

  // Remainders
  int neuronPerGroupRe = config->NUMBER_NEURONS % config->NUMBER_GROUPS;
  int inputNeuronPerGroupRe =
      config->NUMBER_INPUT_NEURONS % config->NUMBER_GROUPS;

  lg->log(LogLevel::INFO, "Adding NeuronGroups");

  for (int i = 0; i < config->NUMBER_GROUPS; i++) {
    int npgRe = neuronPerGroupRe ? 1 : 0;       // apply a paritial remainder?
    int inpgRe = inputNeuronPerGroupRe ? 1 : 0; // apply a paritial remainder?

    NeuronGroup *this_group = new NeuronGroup(
        i + 1, neuronPerGroup + npgRe, inputNeuronPerGroup + inpgRe, this);

    groups.push_back(this_group);
  }

  // Generate vectors differentiated by type
  // lg->log(LogLevel::INFO, "Generating all neuron vector");

  generateAllNeuronVec();

  // lg->value(LogLevel::INFO, "all neuron vector has size %d",
  //           (int)neurons.size());

  // lg->log(LogLevel::INFO, "Generating input neuron vector");

  generateInputNeuronVec();
  setInputNeuronLatency();

  // lg->value(LogLevel::INFO, "Input neuron vector has size %d",
  //           (int)input_neurons.size());

  //  lg->log(LogLevel::INFO, "Generating non-input neuron vector");

  generateNonInputNeuronVec();

  // lg->value(LogLevel::INFO, "non-input neuron vector has size %d",
  //           (int)nonInputNeurons.size());

  using std::get;
  maxLayer = std::get<0>((*dict.end()).first); // set the max layer
  int numEdges = 0;

  for (auto adjacencyPair : dict) {

    int originIndex = getIndex(adjacencyPair.first, maxLayer);

    for (auto edgeWeightPair : adjacencyPair.second) {

      int destinationIndex = getIndex(edgeWeightPair.first, maxLayer);
      double weight = -1;
      if (edgeWeightPair.second.count("weight")) {
        weight = edgeWeightPair.second.at("weight");
      }
      int delay = -1;
      if (edgeWeightPair.second.count("delay")) {
        delay = edgeWeightPair.second.at("delay");
      }
      nonInputNeurons.at(originIndex)
          ->addNeighbor(nonInputNeurons.at(destinationIndex), weight, delay);
      numEdges++;
    }
  }

  // add inputNeuron Connections
  size_t max_size = std::min(input_neurons.size(), nonInputNeurons.size());
  if (max_size == 0) {
    lg->log(LogLevel::ERROR,
            "pySNN::updateEdgeWeights: computed min size of inputNeuron vector "
            "and nonInputNeurons vector is 0?");
    return;
  }

  for (size_t i = 0; i < max_size; i++) {
    InputNeuron *origin = input_neurons.at(i);
    Neuron *destination = nonInputNeurons.at(i);
    origin->addNeighbor(destination, 1, 0);
    numEdges++;
  }

  config->NUMBER_EDGES = numEdges;
}

void pySNN::initialize(AdjDict dict, size_t nInputNeurons) {
  config->NUMBER_INPUT_NEURONS = nInputNeurons;
  initialize(dict);
}

void pySNN::initialize(AdjDict dict, py::buffer buff) {
  processPyBuff(buff);
  initialize(dict);
}

/**
 * @brief initialize pySNN::data with data from buffer
 *
 * Given a numpy array of type py::buffer, generate a matrix
 * (std::vector<std::vector<double>>) that is a copy of that
 * matrix and store it in pySNN::data
 *
 * @param buff reference to py::buffer (numpy array)
 */
void pySNN::processPyBuff(py::buffer &buff) {
  if (!data.empty()) {
    data.clear();
  }
  // get buffer
  py::buffer_info info = buff.request();

  if (info.format != py::format_descriptor<double>::format()) {
    lg->string(ERROR,
               "Invalid datatype in numpy array: expected double, got python "
               "format: %s",
               info.format.c_str());
    throw std::runtime_error("");
  }

  for (auto i = 0; i < info.shape.at(0); i++) {
    const double *row_ptr =
        (double *)info.ptr + i * info.strides.at(0) / sizeof(double);
    std::vector<double> row;
    for (auto j = 0; j < info.shape.at(1); j++) {
      row.push_back(*(row_ptr + j * info.strides.at(1) / sizeof(double)));
    }
    data.push_back(row);
  }
}
void pySNN::runChildProcess(int fd) {
  auto start = std::chrono::high_resolution_clock::now();
  // set input neuron options
  for (std::vector<InputNeuron *>::size_type i = 0; i < input_neurons.size();
       i++) {
    input_neurons.at(i)->setInputValue(dataToRun.at(i));
  }

  generateInputNeuronEvents();

  for (auto group : groups) {
    group->run();
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;
  std::ostringstream oSS;

  if (elapsed.count() > 0.09) {
    for (const auto &d : dataToRun) {
      oSS << d << " ";
    }
    oSS << "\n";
  }

  // pid,evaluationtime
  oSS << getpid() << "," << elapsed.count();

  int sum = 0;
  for (const auto &g : groups) {
    for (const auto &n : g->getNeuronVec()) {
      sum += std::count_if(
          n->getLogData().begin(), n->getLogData().end(),
          [](LogData *d) { return d->message_type == Message_t::Refractory; });
    }
  }
  // ,activations
  oSS << "," << sum;
  start = std::chrono::high_resolution_clock::now();
  lg->writeToFD(fd, groups);
  end = std::chrono::high_resolution_clock::now();
  elapsed = end - start;
  // ,writetime\n
  oSS << "," << elapsed.count() << "\n";
  cout << oSS.str();

  exit(EXIT_SUCCESS);
}

void pySNN::forkRun() {
  std::vector<pid_t> children;
  std::vector<int *> pipes;

  for (auto &v : data) {
    dataToRun = v; // set the childs data
    int *pipefd = (int *)malloc(sizeof(int) * 2);
    int r = pipe(pipefd);
    if (r == -1) {
      lg->log(LogLevel::ERROR, "pySNN::forkRun: pipe failed to create pipe "
                               "file descriptors, pipe() returned -1");
      lg->string(LogLevel::ERROR, "erno reports %s", strerror(errno));
    }
    pid_t cPID = fork();
    switch (cPID) {
    case -1: // error state
      lg->log(LogLevel::ERROR,
              "pySNN::forkRun: failed to spawn child process, fork() "
              "returned -1");
      lg->string(LogLevel::ERROR, "erno reports %s", strerror(errno));
      break;
    case 0: {           // child process
      close(pipefd[0]); // close the read end
      runChildProcess(pipefd[1]);
      break;
    }
    default: // parent process

      config->STIMULUS++; // this is so each child processes "config->STIMULUS"
                          // references the right stimulus number
      pipes.push_back(&pipefd[0]);
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
void pySNN::runBatch(py::buffer &buff) {
  processPyBuff(buff);

  /*
   * Here we break the normal flow to update the configuration values based on
   * the passed buffer and dictionary
   */
  updateStimulusVectorToBuffDim();
  forkRun();
}

void pySNN::pyStart() {

  /*
   * Here we break the normal flow to update the configuration values based on
   * the passed buffer and dictionary
   */
  updateStimulusVectorToBuffDim();

  pySetNextStim();
  generateInputNeuronEvents();

  if (config->show_stimulus) {
    lg->value(ESSENTIAL, "Set stimulus to line %d", *config->STIMULUS);
  }

  for (int i = 1; i < config->num_stimulus + 1; i++) {
    for (auto group : groups) {
      group->startThread();
    }
    for (auto group : groups) {
      pthread_join(group->getThreadID(), NULL);
    }
    if (i < config->num_stimulus) {
      config->STIMULUS++;
      pySetNextStim();
      if (config->show_stimulus) {
        lg->value(ESSENTIAL, "Set stimulus to line %d", *config->STIMULUS);
      }
      reset();
      generateInputNeuronEvents();
    }
  }
  for (auto group : groups) {
    for (auto n : group->getMutNeuronVec()) {
      n->transferData();
    }
  }
}

template <typename T> void test(py::buffer buff) {
  typedef std::vector<std::vector<T>> matrix;

  py::buffer_info info = buff.request();

  matrix m;

  for (auto i = 0; i < info.shape.at(0); i++) {
    const T *row_ptr = (T *)info.ptr + i * info.strides.at(0) / sizeof(T);
    std::vector<T> row;
    for (auto j = 0; j < info.shape.at(1); j++) {
      row.push_back(*(row_ptr + j * info.strides.at(1) / sizeof(T)));
    }
    m.push_back(row);
  }

  for (auto r : m) {
    for (auto e : r) {
      std::cout << e << " ";
    }
    std::cout << '\n';
  }
}

void pySNN::pySetNextStim() {
  if (input_neurons.empty()) {
    lg->log(ESSENTIAL, "set_next_line: passed empty input neuron vector?");
    return;
  }

  for (std::vector<InputNeuron *>::size_type i = 0; i < input_neurons.size();
       i++) {
    input_neurons.at(i)->setInputValue(data.at(*config->STIMULUS).at(i));
  }
};

std::vector<size_t> get_thresholds(size_t num_bins, size_t max_timestamp) {
  size_t range = max_timestamp + 1; // number of possible values
  size_t bin_size = range / num_bins;
  size_t remainder = range % num_bins;
  std::vector<size_t> thresholds(num_bins, 0);
  size_t sum = 0;
  for (auto &el : thresholds) {
    if (remainder) {
      el += 1;
      remainder -= 1;
    }
    el += bin_size;
    int t = el;
    el += sum;
    sum += t;
  }
  return thresholds;
}
size_t bindex(size_t time_stamp, const std::vector<size_t> &thresholds) {
  for (size_t i = 0; i < thresholds.size(); i++) {
    if (time_stamp < thresholds.at(i)) {
      return i;
    }
  }

  return thresholds.size() - 1;
}

py::array_t<int> pySNN::getIndividualActivations(int bins) {
  int activations = 0;
  size_t time_bins = bins < 0 ? config->time_per_stimulus + 1 : bins;
  size_t neuron_id = neurons.size();
  size_t num_stimulus = config->STIMULUS_VEC.size();

  // cout << "time_bins: " << time_bins << "\n";
  // cout << "neuron_id: " << neurons.size() << "\n";
  // cout << "num_stimulus: " << config->STIMULUS_VEC.size() << "\n";

  size_t shape[3] = {num_stimulus, neuron_id, time_bins};
  auto ret = py::array_t<int, py::array::c_style>(shape);
  ret[py::make_tuple(py::ellipsis())] = 0;

  auto info = ret.request();

  // cout << "shape: (" << info.shape.at(0) << ", " << info.shape.at(1) << ", "
  //      << info.shape.at(2) << ")\n";

  const std::vector<LogData *> &lg_data = lg->getLogData();

  const std::vector<size_t> thresholds =
      get_thresholds(time_bins, config->time_per_stimulus);

  for (auto &data_point : lg_data) {
    if (data_point->message_type == Message_t::Refractory) {
      activations += 1;
      size_t stim = data_point->stimulus_number;
      size_t neuron = data_point->neuron_id;
      size_t time_bin = bindex(data_point->timestamp, thresholds);
      // cout << "stim " << stim << "\n";
      // cout << "neuron: " << neuron << "\n";
      // cout << "time_bin: " << time_bin << "\n";
      // cout << "calc index: " << ret.index_at(stim, neuron, time_bin) <<
      // "\n";
      ret.mutable_at(stim, neuron, time_bin) += 1;
    }
  }
  // cout << "num activations = " << activations << "\n";
  return ret;
}

py::array_t<int> pySNN::getActivations(int bins) {
  int activations = 0;
  size_t time_bins = bins < 0 ? config->time_per_stimulus + 1 : bins;
  size_t num_stimulus = config->STIMULUS_VEC.size();

  // cout << "time_bins: " << time_bins << "\n";
  // cout << "neuron_id: " << neurons.size() << "\n";
  // cout << "num_stimulus: " << config->STIMULUS_VEC.size() << "\n";

  size_t shape[2] = {num_stimulus, time_bins};
  auto ret = py::array_t<int, py::array::c_style>(shape);
  ret[py::make_tuple(py::ellipsis())] = 0;

  auto info = ret.request();

  // cout << "shape: (" << info.shape.at(0) << ", " << info.shape.at(1) << ", "
  //      << info.shape.at(2) << ")\n";

  const std::vector<LogData *> &lg_data = lg->getLogData();

  const std::vector<size_t> thresholds =
      get_thresholds(time_bins, config->time_per_stimulus);

  for (auto &data_point : lg_data) {
    if (data_point->message_type == Message_t::Refractory) {
      activations += 1;
      size_t stim = data_point->stimulus_number;
      size_t neuron = data_point->neuron_id;
      size_t time_bin = bindex(data_point->timestamp, thresholds);
      // cout << "stim " << stim << "\n";
      // cout << "neuron: " << neuron << "\n";
      // cout << "time_bin: " << time_bin << "\n";
      // cout << "calc index: " << ret.index_at(stim, neuron, time_bin) <<
      // "\n";
      ret.mutable_at(stim, time_bin) += 1;
    }
  }
  // cout << "num activations = " << activations << "\n";
  return ret;
}

// py::array_t<int> pySNN::getActivations() {
//
//   int max_stim = config->STIMULUS_VEC.back();
//   int min_stim = config->STIMULUS_VEC.front();
//
//   std::unordered_map<int, std::vector<LogData *>> stim_data;
//   const std::vector<LogData *> &lg_data = lg->getLogData();
//
//   for (std::vector<LogData *>::size_type i = 0; i < lg_data.size(); i++) {
//     LogData *td = lg_data.at(i);
//     if (td->message_type == Message_t::Refractory) {
//       if (stim_data.find(td->stimulus_number) != stim_data.end()) {
//         stim_data[td->stimulus_number].push_back(td);
//       } else {
//         stim_data[td->stimulus_number] = {td};
//       }
//     }
//   }
//
//   int bins = config->time_per_stimulus;
//   auto ret =
//       py::array_t<int, py::array::c_style>((max_stim - min_stim + 1) * bins);
//
//   auto info = ret.request();
//   int *pRet = reinterpret_cast<int *>(info.ptr);
//
//   for (int s = min_stim; s <= max_stim; s++) {
//     // if the key does not exist (there were no activations for this
//     stimulus) if (stim_data.find(s) == stim_data.end()) {
//       for (int i = 0; i < bins; i++) {
//         ssize_t index = ((s - min_stim) * bins + i);
//         pRet[index] = 0.0;
//       }
//       continue;
//     }
//     std::vector<LogData *> &sd = stim_data[s];
//
//     std::sort(sd.begin(), sd.end(), [](LogData *a, LogData *b) {
//       return a->timestamp < b->timestamp;
//     });
//
//     double timestep =
//         double(sd.back()->timestamp - sd.front()->timestamp) / bins;
//     double l = sd.front()->timestamp;
//     double u = l + timestep;
//
//     for (int i = 0; i < bins; i++) {
//       ssize_t index = ((s - min_stim) * bins + i);
//       pRet[index] = std::count_if(sd.begin(), sd.end(), [l, u](LogData *ld) {
//         return (ld->timestamp < u) && (ld->timestamp >= l);
//       });
//       l = u;
//       u = u + timestep;
//     }
//   }
//   ret.resize({(max_stim - min_stim + 1), bins});
//   return ret;
// }

void pySNN::pyWrite() { lg->writeData(); };

void pySNN::batchReset() {
  reset();
  lg->batchReset();
  data.clear();
}

void pySNN::outputState() {
  std::string underline(60, '=');
  std::cout << std::setw(30 - strlen("Python SNN State") / 2) << " ";
  std::cout << "Python SNN State\n";
  std::cout << std::setw(30 - strlen("Python SNN State") / 2) << " ";
  std::cout << std::setw(0) << underline;
  std::cout << std::setw(35) << "Number of Neurons: " << std::setw(10)
            << config->NUMBER_NEURONS << "\n";
  std::cout << std::setw(35) << "Number of InputNeurons: " << std::setw(10)
            << config->NUMBER_INPUT_NEURONS << "\n";
  std::cout << std::setw(35) << "Number of non-input Neurons: " << std::setw(10)
            << config->NUMBER_NEURONS - config->NUMBER_INPUT_NEURONS << "\n";
  std::cout << std::setw(35) << "Number of NeuronGroups: " << std::setw(10)
            << config->NUMBER_GROUPS << "\n";
  std::cout << std::setw(35) << "Number of Edges: " << std::setw(10)
            << config->NUMBER_EDGES << "\n";
  std::cout << std::setw(35) << "Data dimensions: " << std::setw(10)
            << data.size() << " stimulus, " << data.front().size()
            << " inputs per stimulus" << "\n";
  std::cout << std::setw(35) << "Log size: " << std::setw(10)
            << lg->getLogData().size() << "\n";
}

void pySNN::setProbabilityOfSuccess(double pSuccess) {
  config->INPUT_PROB_SUCCESS = pSuccess;
}

void pySNN::setMaxLatency(double mLatency, bool update) {
  config->max_latency = mLatency;
  if (update) {
    updateImage();
  }
}

void pySNN::setTau(double Tau) { config->TAU = Tau; }

void pySNN::setRefractoryDuration(int refractory_duration, bool update) {
  config->REFRACTORY_DURATION = refractory_duration;
  if (update) {
    updateNeuronParameters();
  }
}

void pySNN::setTimePerStimulus(int timePer) {
  config->time_per_stimulus = timePer;
}

double pySNN::getProbabilityOfSucess() { return config->INPUT_PROB_SUCCESS; }

double pySNN::getMaxLatency() { return config->max_latency; }

double pySNN::getTau() { return config->TAU; }

int pySNN::getRefractoryDuration() { return config->REFRACTORY_DURATION; }

int pySNN::getTimePerStimulus() { return config->time_per_stimulus; }

void pySNN::updateImage() {
  image->max_latency = config->max_latency;
  setInputNeuronLatency();
}

void pySNN::setSeed(int seed) {
  config->RAND_SEED = seed;
  gen.seed(seed);
}
void pySNN::setInitialMembranePotential(double initialMembranePotential) {
  config->INITIAL_MEMBRANE_POTENTIAL = initialMembranePotential;
}
void pySNN::setRefractoryMembranePotential(double refractoryMembranePotential,
                                           bool update) {
  config->REFRACTORY_MEMBRANE_POTENTIAL = refractoryMembranePotential;
  if (update) {
    updateNeuronParameters();
  }
}

double pySNN::getInitialMembranePotential() {
  return config->INITIAL_MEMBRANE_POTENTIAL;
}

double pySNN::getRefractoryMembranePotential() {
  return config->REFRACTORY_MEMBRANE_POTENTIAL;
}

double pySNN::getActivationThreshold() { return config->ACTIVATION_THRESHOLD; }

void pySNN::setActivationThreshold(double activationThreshold, bool update) {
  config->ACTIVATION_THRESHOLD = activationThreshold;
  if (update) {
    updateNeuronParameters();
  }
}

/**
 * @brief Update data members of Neuron with new values.
 *
 * Some of the configuration parameters are copied and held
 * as data members of the Neuron class because they are accessed so often.
 * This updates the values of those data members based on the current
 * configuraiton
 *
 */
void pySNN::updateNeuronParameters() {
  for (auto &n : neurons) {
    n->setActivationThreshold(config->ACTIVATION_THRESHOLD);
    n->setRefractoryDuration(config->REFRACTORY_DURATION);
    n->setRefractoryMembranePotential(config->REFRACTORY_MEMBRANE_POTENTIAL);
  }
}

void pySNN::updateConfig(ConfigDict dict) {
  config->setOptions(dict);

  if (dict.at("activation_threshold") !=
          configDict.at("activation_threshold") ||
      dict.at("refractory_duration") != configDict.at("refractory_duration") ||
      dict.at("refractory_membrane_potential") !=
          configDict.at("refractory_membrane_potential")) {
    updateNeuronParameters();
  }

  if (dict.at("max_latency") != configDict.at("max_latency")) {
    updateImage();
  }

  configDict = dict;
}
