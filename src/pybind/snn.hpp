#include "../../extern/pybind/include/pybind11/numpy.h"
#include "../../extern/pybind/include/pybind11/pybind11.h"
#include "../network.hpp"
#include <vector>

namespace py = pybind11;

class pySNN : public SNN {
private:
  std::vector<std::vector<double>> data;

public:
  pySNN(std::vector<std::string> args);
  void pyStart(py::buffer);
  void pySetNextStim();
  void pyWrite();
  py::array_t<int> pyOutput();
};

PYBIND11_MODULE(snn, m) {
  m.doc() = "Python API for a Spiking Neural network implemented in C++";
  py::class_<SNN>(m, "SNN")
      .def(py::init<std::vector<std::string>>())
      .def("generateSynapses", &SNN::generateRandomSynapses)
      .def("start", &SNN::start)
      .def("join", &SNN::join);
  py::class_<pySNN>(m, "pySNN")
      .def(py::init<std::vector<std::string>>())
      .def("generateSynapses", &pySNN::generateRandomSynapses,
           "Generate random neural connections")
      .def("start", &pySNN::pyStart, "Start the neural network")
      .def("join", &SNN::join, "Wait for all threads to join")
      .def("writeData", &pySNN::pyWrite,
           "Write activation data to a file in ./logs")
      .def("getActivation", &pySNN::pyOutput,
           "Get the activation data in the form of a numpy array");
}

int add(int i, int j);

template <typename T> void test(py::buffer buff);
