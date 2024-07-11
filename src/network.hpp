#ifndef NETWORK
#define NETWORK
#include "file_reader.hpp"
#include "input_neuron.hpp"
#include "stimulus.hpp"
#include <climits>
#include <cmath>
#include <list>
#include <random>
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
  std::vector<NeuronGroup *>
      groups;                    /**< Holds pointers to all `NeuronGroup`s */
  std::vector<Neuron *> neurons; /**< Holds pointers to all `Neuron`s */
  std::vector<Neuron *>
      nonInputNeurons; /**< Holds pointers to all nonInput`Neuron`s */
  std::vector<InputNeuron *>
      input_neurons;    /**< Holds pointers to all `InputNeuron`s */
  RuntimConfig *config; /**< Holds pointer to RuntimConfig */
  Mutex *mutex;         /**< Holds pointer to Mutex structure */
  Barrier *barrier;
  Image *image;
  InputFileReader *inputFileReader;
  std::mt19937 gen;
  std::random_device rd;

public:
  Log *lg;
  int totalActivations;

  SNN(std::vector<std::string> args);
  SNN();
  ~SNN();

  // initialization
  void initializeFromSynapseFile(const std::vector<std::string> &args,
                                 const std::string &adjListFile);
  void getAdjancyListInfo(const std::string &file_path,
                          AdjListParser::AdjListInfo &info);
  void generateSynapsesFromAdjList(const AdjListParser::AdjList &adjList);
  void setInputNeuronLatency();

  // In-house synapse generation algorithms
  void generateRandomSynapses();
  void generateRandomSynapsesAdjMatrix();
  void generateRandomSynapsesAdjMatrixGS();
  static void *generateRandomSynapsesAdjMatrixGS_Helper(void *arg);
  void generateNeighborOptions(
      std::unordered_map<Neuron *, std::list<Neuron *>> &map);

  // genereating vectors
  void generateAllNeuronVec();
  void generateNonInputNeuronVec();
  void generateInputNeuronVec();
  void generateInputNeuronEvents();

  // runtime operations
  void setNextStim();
  std::list<pid_t> forkStart(const std::vector<std::vector<int>> &stimulusSets);
  void forkJoin(std::list<pid_t> &childrenPIDs);
  void runChildProcess(const std::vector<int> &stimulus,
                       std::ofstream &tmpFile);
  void start();
  void join();
  void reset();

  // output
  void generateGraphiz(bool weights = false);
  int generateCSV();

  // Getters
  double getStimulusStart() { return stimlus_start; }
  bool switchingStimulus() { return switching_stimulus; }
  pthread_cond_t *getSwitchCond() { return &stimulus_switch_cond; }
  static int maximum_edges(int num_i, int num_n);
  std::vector<InputNeuron *> &getMutInputNeurons() { return input_neurons; }
  RuntimConfig *getConfig() { return config; }
  Mutex *getMutex() { return mutex; }
  Barrier *getBarrier() { return barrier; }
  Image *getImage() { return image; }
  int getRandom() { return gen(); }
  std::mt19937 &getGen() { return gen; }
};
#endif // !NETWORK
