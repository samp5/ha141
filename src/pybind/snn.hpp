#include "../../extern/pybind/include/pybind11/numpy.h"
#include "../../extern/pybind/include/pybind11/pybind11.h"
#include "../network.hpp"
#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace py = pybind11;
using AdjDict =
    std::map<std::tuple<int, int>,
             std::map<std::tuple<int, int>, std::map<std::string, float>>>;

class pySNN : public SNN {
private:
  std::vector<std::vector<double>> data;
  AdjDict adjList;
  size_t maxLayer; // maximum "layer", aka maximum number of columns

public:
  pySNN(std::vector<std::string> args);
  void pyStart();
  void pySetNextStim();
  void pyWrite();
  void initialize(AdjDict dict, py::buffer buff);
  void updateEdgeWeights(AdjDict dict);
  void processPyBuff(py::buffer &buff);
  void updateConfigToBuffDim();
  void generateImageFromBuff();
  void updateConfigToAdjList(const AdjDict &dict);
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
      .def("generateSynapses", &pySNN::generateRandomSynapsesAdjMatrixGS,
           "Generate random neural connections")
      .def("updateWeights", &pySNN::updateEdgeWeights,
           "Update edge weights based on dict of dicts")
      .def("start", &pySNN::pyStart, "Start the neural network")
      .def("join", &SNN::join, "Wait for all threads to join")
      .def("writeData", &pySNN::pyWrite,
           "Write activation data to a file in ./logs")
      .def("getActivation", &pySNN::pyOutput,
           "Get the activation data in the form of a numpy array")
      .def("initialize", &pySNN::initialize,
           "Initilize network from dict of dicts");
}

int add(int i, int j);

template <typename T> void test(py::buffer buff);
