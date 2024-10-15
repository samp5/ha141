#ifndef PYSNN_H
#define PYSNN_H
#include "../../extern/pybind/include/pybind11/numpy.h"
#include "../../extern/pybind/include/pybind11/pybind11.h"
#include "../funciton_id.h"
#include "../network.hpp"
#include "snn_connect.h"
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
  std::vector<double> dataToRun;
  AdjDict adjList;
  size_t maxLayer; // maximum "layer", aka maximum number of columns
  ConfigDict configDict;
  bool rpc = false;
  ;

public:
  pySNN(std::vector<std::string> args);
  pySNN(std::string configFile);
  pySNN(ConfigDict dict = {});
  void rpc_construct(ConfigDict dict = {});

  static ConfigDict getDefaultConfig();

  void pyStart();
  void pySetNextStim();
  void pyWrite();
  void initialize(AdjDict dict, py::buffer buff);
  void initialize(AdjDict dict, size_t NUMBER_INPUT_NEURONS);
  void initialize(AdjDict &dict);
  void runBatch_rpc(py::buffer &buff);
  void runBatch(py::buffer &buff);
  void updateEdgeWeights(AdjDict dict);
  void processPyBuff(py::buffer &buff);
  void forkRun();
  void runChildProcess(int fd);
  void updateStimulusVectorToBuffDim();
  void generateImage();
  void updateConfigToAdjList(const AdjDict &dict);
  void batchReset();
  py::array_t<int> getActivations(int bins = -1);
  py::array_t<int> getIndividualActivations(int bins = -1);
  void outputState();

  void updateImage();
  void updateNeuronParameters();
  void updateConfig(ConfigDict dict);

  // setters
  void setProbabilityOfSuccess(double pSuccess);
  void setMaxLatency(double mLatency, bool update = true);
  void setTau(double Tau);
  void setRefractoryDuration(int duration, bool update = true);
  void setTimePerStimulus(int timePerStimulus);
  void setSeed(int seed);

  void setInitialMembranePotential(double initialMembranePotential);
  void setRefractoryMembranePotential(double refractoryMembranePotential,
                                      bool update = true);
  void setActivationThreshold(double activationThreshold, bool update = true);

  // getters
  double getProbabilityOfSucess();
  double getMaxLatency();
  double getTau();
  int getRefractoryDuration();
  int getTimePerStimulus();
  double getInitialMembranePotential();
  double getRefractoryMembranePotential();
  double getActivationThreshold();
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
      .def(py::init<std::string>(), py::arg("configFile") = "base_config.toml")
      .def(py::init<ConfigDict &>())
      .def("generateSynapses", &pySNN::generateRandomSynapsesAdjMatrixGS,
           "Generate random neural connections")
      .def("updateSynapses", &pySNN::updateEdgeWeights,
           "Update edge weights based on dict of dicts")
      .def("start", &pySNN::pyStart, "Start the neural network")
      .def("join", &SNN::join, "Wait for all threads to join")
      .def("writeData", &pySNN::pyWrite,
           "Write activation data to a file in ./logs")
      .def("getActivation", &pySNN::getActivations, py::arg("bins") = -1,
           "Get the activation data in the form of a numpy array")
      .def("getIndividualActivation", &pySNN::getIndividualActivations,
           py::arg("bins") = -1,
           "Get the activation data for individual neurons in the form of a "
           "numpy tensor")
      .def("initialize",
           py::overload_cast<AdjDict, py::buffer>(&pySNN::initialize),
           "Initilize network from dict of dicts")
      .def("initialize", py::overload_cast<AdjDict, size_t>(&pySNN::initialize),
           "Initilize network from dict of dicts")
      .def("runBatch", &pySNN::runBatch, "Run a batch in a child process")
      .def("batchReset", &pySNN::batchReset, "Reset network after a batch run")
      .def("outputState", &pySNN::outputState, "Output state")
      .def_static("getDefaultConfig", &pySNN::getDefaultConfig,
                  "build a ConfigurationDictionary")
      .def("updateConfig", &pySNN::updateConfig,
           "update the configuration based on the passed dictionary")
      .def("setProbabilityOfSuccess", &pySNN::setProbabilityOfSuccess, "")
      .def("setMaxLatency", &pySNN::setMaxLatency, py::arg("mLatency"),
           py::arg("update") = true, "")
      .def("setTau", &pySNN::setTau, "")
      .def("setRefractoryDuration", &pySNN::setRefractoryDuration, "")
      .def("setTimePerStimulus", &pySNN::setTimePerStimulus, "")
      .def("setSeed", &pySNN::setSeed, "")
      .def("setInitialMembranePotential", &pySNN::setInitialMembranePotential,
           "")
      .def("setRefractoryMembranePotential",
           &pySNN::setRefractoryMembranePotential,
           py::arg("refractoryMembranePotential"), py::arg("update") = true, "")
      .def("setActivationThreshold", &pySNN::setActivationThreshold,
           py::arg("activationThreshold"), py::arg("update") = true, "")
      .def("getProbabilityOfSucess", &pySNN::getProbabilityOfSucess, "")
      .def("getMaxLatency", &pySNN::getMaxLatency, "")
      .def("getTau", &pySNN::getTau, "")
      .def("getRefractoryDuration", &pySNN::getRefractoryDuration, "")
      .def("getTimePerStimulus", &pySNN::getTimePerStimulus, "")
      .def("getInitialMembranePotential", &pySNN::getInitialMembranePotential,
           "")
      .def("getRefractoryMembranePotential",
           &pySNN::getRefractoryMembranePotential, "")
      .def("getActivationThreshold", &pySNN::getActivationThreshold, "");
}

int add(int i, int j);

template <typename T> void test(py::buffer buff);
#endif
