#ifndef NETWORK
#define NETWORK
#include "input_neuron.hpp"
#include "stimulus.hpp"
#include <climits>
#include <cmath>
#include <list>
#include <stdexcept>
#include <unordered_map>
#include <vector>

class Neuron;
class NeuronGroup;
struct RuntimConfig;
struct Mutex;

struct Barrier {
  pthread_barrier_t barrier;
  unsigned int count;

  Barrier(unsigned int _c) : count(_c) {
    int ret = pthread_barrier_init(&barrier, NULL, count);
    if (ret) {
      throw std::runtime_error("Error initializing pthread barrier");
    }
  }
};

/**
 * @brief Spiking Neural Network.
 *
 * Runs all operations for the spiking neural network.
 *
 */
class SNN {
protected:
  bool switching_stimulus;
  pthread_cond_t stimulus_switch_cond = PTHREAD_COND_INITIALIZER;
  double stimlus_start;
  bool active;
  std::vector<NeuronGroup *>
      groups;                    /**< Holds pointers to all `NeuronGroup`s */
  std::vector<Neuron *> neurons; /**< Holds pointers to all `Neuron`s */
  std::vector<InputNeuron *>
      input_neurons;    /**< Holds pointers to all `InputNeuron`s */
  RuntimConfig *config; /**< Holds pointer to RuntimConfig */
  Mutex *mutex;         /**< Holds pointer to Mutex structure */
  Barrier *barrier;
  Image *image;

public:
  Log *lg;
  SNN(std::vector<std::string> args);
  SNN(){};
  ~SNN();
  double generateSynapseWeight();
  void start();
  void join();
  void reset();
  void generateRandomSynapses();
  void generateRandomSynapsesAdjMatrix();
  void generateNeuronVec();
  void generateInputNeuronVec();
  void setInputNeuronLatency();
  void setNextStim();
  double getStimulusStart() { return stimlus_start; }
  void generateGraphiz(bool weights = false);
  bool isActive() { return active; }
  bool switchingStimulus() { return switching_stimulus; }
  pthread_cond_t *switchCond() { return &stimulus_switch_cond; }
  static int maximum_edges(int num_i, int num_n);
  std::vector<InputNeuron *> &getMutInputNeurons() { return input_neurons; }
  void generateNeighborOptions(
      std::unordered_map<Neuron *, std::list<Neuron *>> &map);
  RuntimConfig *getConfig() { return config; }
  Mutex *getMutex() { return mutex; }
  Barrier *getBarrier() { return barrier; }
  Image *getImage() { return image; }
};
#endif // !NETWORK
