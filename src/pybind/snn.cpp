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
  lg = new Log(this);
  config = new RuntimConfig(this);
  config->parseArgs(args);
  config->checkStartCond();
  srand(config->RAND_SEED);
  mutex = new Mutex;
  barrier = new Barrier(config->NUMBER_GROUPS + 1);

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
  generateNeuronVec();
  generateInputNeuronVec();
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

void pySNN::overrideConfigValues() {

  // override stimulus
  config->STIMULUS_VEC.clear();
  std::vector<int>::size_type number_lines = data.size();
  for (std::vector<int>::size_type i = 0; i < number_lines; i++) {
    config->STIMULUS_VEC.push_back(i);
  }
  config->STIMULUS = config->STIMULUS_VEC.begin();
  config->num_stimulus = config->STIMULUS_VEC.size();

  if (config->NUMBER_INPUT_NEURONS != data.front().size()) {
    lg->log(WARNING, "Number of input neurons does not equal the number of "
                     "elements per stimulus");
  }
}

void pySNN::pyStart(py::buffer buff) {

  processPyBuff(buff);
  overrideConfigValues();

  pySetNextStim();
  generateInputNeuronEvents();
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
      generateInputNeuronEvents();
      reset();
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
