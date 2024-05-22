#include "../../extern/pybind/include/pybind11/pybind11.h"
#include "../network.hpp"
#include <vector>

namespace py = pybind11;

class pySNN : public SNN {
private:
  std::vector<std::vector<double>> data;

public:
  pySNN(std::vector<std::string> args) : SNN(args){};
  void pyStart(py::buffer);
  void pySetStimLineX(int target);
  void pySetStim(int target);
  void pySetNextStim();
  void pyWrite();
};

PYBIND11_MODULE(snn, m) {
  py::class_<SNN>(m, "SNN")
      .def(py::init<std::vector<std::string>>())
      .def("generateSynapses", &SNN::generateRandomSynapses)
      .def("start", &SNN::start)
      .def("join", &SNN::join);
  py::class_<pySNN>(m, "pySNN")
      .def(py::init<std::vector<std::string>>())
      .def("generateSynapses", &pySNN::generateRandomSynapses)
      .def("start", &pySNN::pyStart)
      .def("join", &SNN::join)
      .def("writeData", &pySNN::pyWrite);
}

int add(int i, int j);

template <typename T> void test(py::buffer buff);
