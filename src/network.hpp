#ifndef NETWORK
#define NETWORK
#include "input_neuron.hpp"
#include <list>
#include <optional>
#include <unordered_map>
#include <vector>

class Neuron;
class NeuronGroup;
struct RuntimConfig;
struct Mutex;

/**
 * @brief Spiking Neural Network.
 *
 * Runs all operations for the spiking neural network.
 *
 */
class SNN {
private:
  bool switching_stimulus;
  pthread_cond_t stimulus_switch_cond = PTHREAD_COND_INITIALIZER;
  bool active;
  std::vector<NeuronGroup *>
      groups;                    /**< Holds pointers to all `NeuronGroup`s */
  std::vector<Neuron *> neurons; /**< Holds pointers to all `Neuron`s */
  std::vector<InputNeuron *>
      input_neurons;    /**< Holds pointers to all `InputNeuron`s */
  RuntimConfig *config; /**< Holds pointer to RuntimConfig */
  Mutex *mutex;         /**< Holds pointer to Mutex structure */
  // std::optional<py::array_t<double>> input_data;

public:
  Log *lg;
  SNN(std::vector<std::string> args);
  ~SNN();
  double generateSynapseWeight();
  void start();
  void pyStart();
  void join();
  void reset();
  void generateRandomSynapses();
  void generateNeuronVec();
  void generateInputNeuronVec();
  void setStimLineX(int target);
  void setStim(int target);
  void getLineX(std::string &line, int target);
  void getNextLine(std::string &line);
  void setNextStim();
  void pyWrite();
  bool isActive() { return active; }
  bool switchingStimulus() { return switching_stimulus; }
  pthread_cond_t *switchCond() { return &stimulus_switch_cond; }
  static int maximum_edges(int num_i, int num_n);
  std::vector<InputNeuron *> &getMutInputNeurons() {
    return this->input_neurons;
  }
  std::unordered_map<Neuron *, std::list<Neuron *>> generateNeighborOptions();
  RuntimConfig *getConfig() { return config; }
  Mutex *getMutex() { return mutex; }
};
#endif // !NETWORK
