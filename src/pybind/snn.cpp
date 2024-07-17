#include "snn.hpp"
#include "../../extern/pybind/include/pybind11/stl.h"
#include "../runtime.hpp"
#include <algorithm>
#include <climits>
#include <numeric>
#include <pthread.h>
#include <stdexcept>
#include <unordered_map>
#include <vector>

int add(int i, int j) { return i + j; }

pySNN::pySNN(std::vector<std::string> args) : SNN() {
  lg->setNetwork(this);
  config = new RuntimConfig(this);
  config->parseArgs(args);
  config->checkStartCond();
  mutex = new Mutex;
  barrier = new Barrier(config->NUMBER_GROUPS + 1);

  gen = std::mt19937(rd());
  gen.seed(config->RAND_SEED);
  image = nullptr; // we have to wait to initalize the images until we know the
                   // size of the python buffer
}
int getIndex(const std::tuple<int, int> &pair, int maxLayer) {
  int x = std::get<0>(pair);
  int y = std::get<1>(pair);

  int index = x + y * maxLayer;
  return index;
}

void pySNN::updateEdgeWeights(AdjDict dict) {

  using std::get;

  for (auto adjacencyPair : dict) {

    // !DEBUG
    std::cout << "Updating weights for (" << std::get<0>(adjacencyPair.first)
              << ", " << std::get<1>(adjacencyPair.first) << ")\n";

    int originIndex = getIndex(adjacencyPair.first, maxLayer);

    const auto &synapseRef =
        nonInputNeurons.at(originIndex)
            ->getPostSynaptic(); // get outgoing connections

    for (auto edgeWeightPair : adjacencyPair.second) {

      int destinationIndex = getIndex(edgeWeightPair.first, maxLayer);
      double weight = edgeWeightPair.second.at("weight");

      // !DEBUG
      std::cout << "\t(" << std::get<0>(edgeWeightPair.first) << ", "
                << std::get<1>(edgeWeightPair.first) << ") updated weight to "
                << weight << "\n";

      Neuron *destination = nonInputNeurons.at(destinationIndex);
      bool updated = false;
      for (auto &synapse : synapseRef) {
        if (synapse->getPostSynaptic() == destination) {
          synapse->updateWeight(weight);
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

void pySNN::initialize(AdjDict dict, py::buffer buff) {

  processPyBuff(buff);
  overrideConfigValues();

  // overrides based on input
  int numberNonInput = dict.size();
  lg->value(LogLevel::INFO, "numberNonInput is %d", numberNonInput);
  config->NUMBER_INPUT_NEURONS = image->width * image->height;
  lg->value(LogLevel::INFO, "NUMBER_INPUT_NEURONS is %d",
            config->NUMBER_INPUT_NEURONS);
  config->NUMBER_NEURONS = config->NUMBER_INPUT_NEURONS + numberNonInput;
  lg->value(LogLevel::DEBUG, "NUMBER_NEURONS is %d", config->NUMBER_NEURONS);

  int neuronPerGroup = config->NUMBER_NEURONS / config->NUMBER_GROUPS;
  int inputNeuronPerGroup =
      config->NUMBER_INPUT_NEURONS / config->NUMBER_GROUPS;
  int neuronPerGroupRe = config->NUMBER_NEURONS % config->NUMBER_GROUPS;
  int inputNeuronPerGroupRe =
      config->NUMBER_INPUT_NEURONS % config->NUMBER_GROUPS;

  lg->log(LogLevel::INFO, "Adding NeuronGroups");
  for (int i = 0; i < config->NUMBER_GROUPS; i++) {
    int npgRe = neuronPerGroupRe ? 1 : 0;
    int inpgRe = inputNeuronPerGroupRe ? 1 : 0;
    // allocate for this group
    NeuronGroup *this_group = new NeuronGroup(
        i + 1, neuronPerGroup + npgRe, inputNeuronPerGroup + inpgRe, this);

    // add to vector
    groups.push_back(this_group);
  }

  // Generate vectors differentiated by type
  lg->log(LogLevel::INFO, "Generating all neuron vector");
  generateAllNeuronVec();
  lg->value(LogLevel::INFO, "all neuron vector has size %d",
            (int)neurons.size());
  lg->log(LogLevel::INFO, "Generating input neuron vector");
  generateInputNeuronVec();
  lg->value(LogLevel::INFO, "Input neuron vector has size %d",
            (int)input_neurons.size());
  lg->log(LogLevel::INFO, "Generating non-input neuron vector");
  generateNonInputNeuronVec();
  lg->value(LogLevel::INFO, "non-input neuron vector has size %d",
            (int)nonInputNeurons.size());

  using std::get;
  maxLayer = std::get<0>((*dict.end()).first); // set the max layer
  int numEdges = 0;

  for (auto adjacencyPair : dict) {

    // !DEBUG
    std::cout << "(" << std::get<0>(adjacencyPair.first) << ", "
              << std::get<1>(adjacencyPair.first) << ")\n";

    int originIndex = getIndex(adjacencyPair.first, maxLayer);

    for (auto edgeWeightPair : adjacencyPair.second) {

      // !DEBUG
      std::cout << "\t(" << std::get<0>(edgeWeightPair.first) << ", "
                << std::get<1>(edgeWeightPair.first) << ")\n";

      int destinationIndex = getIndex(edgeWeightPair.first, maxLayer);
      double weight = edgeWeightPair.second.at("weight");

      nonInputNeurons.at(originIndex)
          ->addNeighbor(nonInputNeurons.at(destinationIndex), weight);
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
    origin->addNeighbor(destination, 1);
    numEdges++;
  }

  setInputNeuronLatency();
}

void pySNN::processPyBuff(py::buffer &buff) {
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

// !TODO This funciton is bad and should probably be moved elsewhere
void pySNN::overrideConfigValues() {

  // override stimulus
  config->STIMULUS_VEC.clear();
  std::vector<int>::size_type number_lines = data.size();
  for (std::vector<int>::size_type i = 0; i < number_lines; i++) {
    config->STIMULUS_VEC.push_back(i);
  }
  config->STIMULUS = config->STIMULUS_VEC.begin();
  config->num_stimulus = config->STIMULUS_VEC.size();

  if (static_cast<size_t>(config->NUMBER_INPUT_NEURONS) !=
      data.front().size()) {
    lg->value(WARNING,
              "Number of input neurons does not equal the number of "
              "elements per stimulus, setting to %d",
              (int)data.front().size());
  }
  config->NUMBER_INPUT_NEURONS = data.front().size();

  // This is just here so that we can ensure that the images is updated based on
  // the buffer
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

void pySNN::pyStart() {

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

py::array_t<int> pySNN::pyOutput() {

  int max_stim = config->STIMULUS_VEC.back();
  int min_stim = config->STIMULUS_VEC.front();

  std::unordered_map<int, std::vector<LogData *>> stim_data;
  const std::vector<LogData *> &lg_data = lg->getLogData();

  for (std::vector<LogData *>::size_type i = 0; i < lg_data.size(); i++) {
    LogData *td = lg_data.at(i);
    if (td->message_type == Message_t::Refractory) {
      if (stim_data.find(td->stimulus_number) != stim_data.end()) {
        stim_data[td->stimulus_number].push_back(td);
      } else {
        stim_data[td->stimulus_number] = {td};
      }
    }
  }

  int bins = config->time_per_stimulus;
  auto ret =
      py::array_t<int, py::array::c_style>((max_stim - min_stim + 1) * bins);

  auto info = ret.request();
  int *pRet = reinterpret_cast<int *>(info.ptr);

  for (int s = min_stim; s <= max_stim; s++) {
    // if the key does not exist (there were no activations for this stimulus)
    if (stim_data.find(s) == stim_data.end()) {
      for (int i = 0; i < bins; i++) {
        ssize_t index = ((s - min_stim) * bins + i);
        pRet[index] = 0.0;
      }
      continue;
    }
    std::vector<LogData *> &sd = stim_data[s];

    std::sort(sd.begin(), sd.end(), [](LogData *a, LogData *b) {
      return a->timestamp < b->timestamp;
    });

    double timestep =
        double(sd.back()->timestamp - sd.front()->timestamp) / bins;
    double l = sd.front()->timestamp;
    double u = l + timestep;

    for (int i = 0; i < bins; i++) {
      ssize_t index = ((s - min_stim) * bins + i);
      pRet[index] = std::count_if(sd.begin(), sd.end(), [l, u](LogData *ld) {
        return (ld->timestamp < u) && (ld->timestamp >= l);
      });
      l = u;
      u = u + timestep;
    }
  }
  ret.resize({(max_stim - min_stim + 1), bins});
  return ret;
}

void pySNN::pyWrite() { lg->writeData(); };
