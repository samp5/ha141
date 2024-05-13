#ifndef GLOBALS
#include "log.hpp"
#include <vector>

struct Mutex {
public:
  void destroy_mutexes();
  // Protects Stimulus
  pthread_mutex_t stimulus = PTHREAD_MUTEX_INITIALIZER;
  // Protects the membrane_potential
  pthread_mutex_t potential = PTHREAD_MUTEX_INITIALIZER;
  // Protects output stream
  pthread_mutex_t log = PTHREAD_MUTEX_INITIALIZER;
  // Protects activation
  pthread_mutex_t activation = PTHREAD_MUTEX_INITIALIZER;
  // Protects messaging
  pthread_mutex_t message = PTHREAD_MUTEX_INITIALIZER;
};

struct RuntimConfig {
  double INITIAL_MEMBRANE_POTENTIAL;
  double ACTIVATION_THRESHOLD;
  double REFRACTORY_MEMBRANE_POTENTIAL;
  int RAND_SEED;
  int NUMBER_EDGES;
  int NUMBER_GROUPS;
  int NUMBER_NEURONS;
  int NUMBER_INPUT_NEURONS;
  std::vector<int> STIMULUS_VEC;
  std::vector<int>::const_iterator STIMULUS;
  double TAU;
  unsigned long RUN_TIME;
  double REFRACTORY_DURATION;
  double DECAY_VALUE;
  double INPUT_PROB_SUCCESS;
  LogLevel DEBUG_LEVEL;
  std::string INPUT_FILE;
  std::string CONFIG_FILE;
  ostream &STREAM = std::cout;
};

#endif // !GLOBALS
