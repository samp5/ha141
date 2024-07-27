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

typedef std::map<std::string, double> ConfigDict;

class pySNN : public SNN {
private:
  std::vector<std::vector<double>> data;
  AdjDict adjList;
  size_t maxLayer; // maximum "layer", aka maximum number of columns
  ConfigDict configDict;

public:
  pySNN(std::vector<std::string> args);
  pySNN(std::string configFile);
  pySNN(ConfigDict dict = {});

  static ConfigDict getDefaultConfig();

  void pyStart();
  void pySetNextStim();
  void pyWrite();
  void initialize(AdjDict dict, py::buffer buff);
  void initialize(AdjDict dict, size_t NUMBER_INPUT_NEURONS);
  void initialize(AdjDict &dict);
  void runBatch(py::buffer &buff);
  void updateEdgeWeights(AdjDict dict);
  void processPyBuff(py::buffer &buff);
  void forkRun();
  void runChildProcess(int fd);
  void updateStimulusVectorToBuffDim();
  void generateImage();
  void updateConfigToAdjList(const AdjDict &dict);
  void batchReset();
  py::array_t<int> pyOutput();
  void outputState();
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
      .def("initialize",
           py::overload_cast<AdjDict, py::buffer>(&pySNN::initialize),
           "Initilize network from dict of dicts")
      .def("initialize", py::overload_cast<AdjDict, size_t>(&pySNN::initialize),
           "Initilize network from dict of dicts")
      .def("runBatch", &pySNN::runBatch, "Run a batch in a child process")
      .def("batchReset", &pySNN::batchReset, "Reset network after a batch run")
      .def("outputState", &pySNN::outputState, "Output state");
}

int add(int i, int j);

template <typename T> void test(py::buffer buff);
