#ifndef PYSNN_H
#define PYSNN_H
#include "../../extern/pybind/include/pybind11/numpy.h"
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

#endif
