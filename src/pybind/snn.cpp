#include "snn.hpp"
#include "../../extern/pybind/include/pybind11/stl.h"
#include "../runtime.hpp"
#include <algorithm>
#include <climits>
#include <unordered_map>
#include <vector>

int add(int i, int j) { return i + j; }

pySNN::pySNN(std::vector<std::string> args) : SNN() {
  lg = new Log(this);
  config = new RuntimConfig(this);
  config->parseArgs(args);
  // input_data = std::nullopt;
  config->checkStartCond();
  srand(config->RAND_SEED);
  mutex = new Mutex;

  int neuron_per_group = config->NUMBER_NEURONS / config->NUMBER_GROUPS;
  int input_neurons_per_group =
      config->NUMBER_INPUT_NEURONS / config->NUMBER_GROUPS;

  for (int i = 0; i < config->NUMBER_GROUPS; i++) {

    // allocate for this group
    NeuronGroup *this_group =
        new NeuronGroup(i + 1, neuron_per_group, input_neurons_per_group, this);

    // add to vector
    this->groups.push_back(this_group);
  }
  this->generateNeuronVec();
  this->generateInputNeuronVec();
}

void pySNN::pyStart(py::buffer buff) {

  py::buffer_info info = buff.request();

  for (auto i = 0; i < info.shape.at(0); i++) {
    const double *row_ptr =
        (double *)info.ptr + i * info.strides.at(0) / sizeof(double);
    std::vector<double> row;
    for (auto j = 0; j < info.shape.at(1); j++) {
      row.push_back(*(row_ptr + j * info.strides.at(1) / sizeof(double)));
    }
    data.push_back(row);
  }

  active = true;
  pySetStimLineX(*config->STIMULUS);

  for (auto group : this->groups) {
    group->startThread();
  }

  for (int i = 1; i < config->num_stimulus + 1; i++) {

    usleep(config->time_per_stimulus);

    if (i < config->num_stimulus) {
      auto start = std::chrono::high_resolution_clock::now();
      switching_stimulus = true;

      config->STIMULUS++;
      this->pySetNextStim();
      lg->value(ESSENTIAL, "Set stimulus to line %d", *config->STIMULUS);

      this->reset();

      switching_stimulus = false;
      pthread_cond_broadcast(&stimulus_switch_cond);
      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(
          end - start);
      lg->addOffset(duration.count());
    }
  }
  active = false;
}

/**
 * @brief Set stimulus to line X of RuntimConfig::INPUT_FILE.
 *
 * @param target Line number (zero-indexed)
 */
void pySNN::pySetStimLineX(int target) {

  lg->value(ESSENTIAL, "Set stimulus to line %d", *config->STIMULUS);

  if (input_neurons.empty()) {
    lg->log(ESSENTIAL, "set_line_x: passed empty input neuron vector?");
    return;
  }

  for (std::vector<InputNeuron *>::size_type i = 0; i < input_neurons.size();
       i++) {
    input_neurons.at(i)->setInputValue(data.at(target).at(i));
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

  int max_stim = -1;
  int min_stim = INT_MAX;

  std::unordered_map<int, std::vector<LogData *>> stim_data;
  std::vector<LogData *> lg_data = lg->getLogData();
  for (std::vector<LogData *>::size_type i = 0; i < lg_data.size(); i++) {
    LogData *td = lg_data.at(i);
    if (td->message_type == Message_t::Refractory) {
      max_stim = std::max(max_stim, td->stimulus_number);
      min_stim = std::min(min_stim, td->stimulus_number);

      if (stim_data.find(td->stimulus_number) != stim_data.end()) {
        stim_data[td->stimulus_number].push_back(td);
      } else {
        stim_data[td->stimulus_number] = {td};
      }
    }
  }

  int bins = 300;
  auto ret =
      py::array_t<int, py::array::c_style>((max_stim - min_stim + 1) * bins);

  auto info = ret.request();
  int *pRet = reinterpret_cast<int *>(info.ptr);

  for (int s = min_stim; s <= max_stim; s++) {
    std::vector<LogData *> &sd = stim_data[s];

    std::sort(sd.begin(), sd.end(), [](LogData *a, LogData *b) {
      return a->timestamp < b->timestamp;
    });

    double timestep = (sd.back()->timestamp - sd.front()->timestamp) / bins;
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
