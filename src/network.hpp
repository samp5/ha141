#ifndef NETWORK
#define NETWORK
#include "input_neuron.hpp"
#include <list>
#include <unordered_map>
#include <vector>
class Neuron;
class NeuronGroup;

/**
 * @brief Spiking Neural Network.
 *
 * Runs all operations for the spiking neural network.
 *
 */
class SNN {
private:
  std::vector<NeuronGroup *> groups;
  std::vector<Neuron *> neurons;
  std::vector<InputNeuron *> input_neurons;
  RuntimConfig *config;

public:
  SNN(RuntimConfig *config);
  ~SNN();
  double generateSynapseWeight();
  void start();
  void join();
  void reset();
  void generateRandomSynapses();
  void generateNeuronVec();
  void generateInputNeuronVec();
  void setStimLineX(int target);
  void getLineX(std::string &line, int target);
  void getNextLine(std::string &line);
  void setNextStim();
  static int maximum_edges();
  std::vector<InputNeuron *> &getMutInputNeurons() {
    return this->input_neurons;
  }
  std::unordered_map<Neuron *, std::list<Neuron *>> generateNeighborOptions();
};
#endif // !NETWORK
