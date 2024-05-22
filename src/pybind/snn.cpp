#include "snn.hpp"
#include "../../extern/pybind/include/pybind11/pybind11.h"
#include "../../extern/pybind/include/pybind11/stl.h"
#include "../runtime.hpp"
#include <sstream>

int add(int i, int j) { return i + j; }

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

  for (int i = 0; i < input_neurons.size(); i++) {
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
  for (int i = 0; i < input_neurons.size(); i++) {
    input_neurons.at(i)->setInputValue(data.at(*config->STIMULUS).at(i));
  }
};

void pySNN::pyWrite() { lg->writeData(); };
