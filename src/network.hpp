#ifndef NETWORK
#define NETWORK
#include "input_neuron.hpp"
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

  Barrier(unsigned int count) : count(count) {
    int ret = pthread_barrier_init(&barrier, NULL, this->count);
    if (ret) {
      throw std::runtime_error("Error initializing pthread barrier");
    }
  }
};

struct Image {
  int width;
  int height;
  double max_latency;

  struct Point {
    float x;
    float y;
    Point(float x, float y) : x(x), y(y){};
  };

  Point center;

  double max_distance;

  Image(int w, int h, double max_latency = 0.025)
      : width(w), height(h), max_latency(max_latency),
        center(float(w - 1) / 2, float(h - 1) / 2),
        max_distance(getDistance(0, 0)){};

  Image(int pixels, double max_latency = 0.025)
      : width(std::sqrt(pixels)), height(std::sqrt(pixels)),
        max_latency(max_latency), center(float(width) / 2, float(height) / 2),
        max_distance(getDistance(0, 0)) {
    if (!isSquare(pixels)) {
      throw std::runtime_error(
          "Error intialzing Image struct with pixel constructor. Need a square "
          "image or use constructor with signature Image(int, int)");
    }
  };

  double getDistance(double x, double y) {
    double distance =
        std::sqrt(std::pow(x - center.x, 2) + std::pow(y - center.y, 2));
    return distance;
  };
  double getLatency(int index) {
    Point pos = calculateCoords(index);
    double distance = getDistance(pos.x, pos.y);
    double latency = distance / max_distance * max_latency;
    return latency;
  }
  Point calculateCoords(int index) {
    int col = index % width;
    int row;
    if ((index - col) == 0) {
      row = 0;
    } else {
      row = (index - col) / width;
    }
    return Point(col, row);
  }

  static std::pair<int, int> bestRectangle(int pixels) {
    int min_per = INT_MAX;
    int w = 0;
    int h = 0;
    for (int i = 1; i < std::ceil(std::sqrt(pixels)) + 1; i++) {
      if (pixels % i == 0) {
        int width = pixels / i;
        int height = i;
        std::cout << "width: " << width << "height: " << height << " \n";
        if (2 * (width + height) < min_per) {
          w = width;
          h = height;
          std::cout << "w: " << w << "h: " << h << " \n";
        }
      }
    }
    std::pair<int, int> dim = std::make_pair(w, h);
    return dim;
  }

  static bool isSquare(double pixels) {
    if (int(std::sqrt(pixels)) * int(std::sqrt(pixels)) == pixels) {
      return true;
    }
    return false;
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
  void generateNeuronVec();
  void generateInputNeuronVec();
  void setInputNeuronLatency();
  void setNextStim();
  double getStimulusStart() { return stimlus_start; }
  void generateGraphiz();
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
  Barrier *getBarrier() { return barrier; }
  Image *getImage() { return image; }
};
#endif // !NETWORK
