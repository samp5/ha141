#ifndef GLOBALS
#define GLOBALS
#include "log.hpp"
#include <map>
#include <vector>

/**
 * \struct Mutex
 *
 * @brief Holds all mutexes
 */
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

/**
 * @brief Holds all configuration options.
 *
 * The RuntimConfig is based on a toml configuration file located in run_config/
 *
 * If no configuration file is specified and run_config/base_config.toml is not
 * found, the following configuration file is created as
 * run_config/base_config.toml and used.
 *
 * <details>
 *
 * <summary> base_config.toml </summary> <br>
 *
 * ```toml
 * [neuron]
 * # NOTE: both the number of neurons and the number of input neurons
 * #       must be divisible by the number of groups
 *
 * # number of neurons
 * neuron_count = 10
 *
 * # number of input type neurons
 * input_neuron_count = 2
 *
 * # number of groups
 * group_count = 2
 *
 * # number of connections
 * # option can be "MAX" for maximum edges
 * edge_count = 1
 *
 * # refractory_duration
 * refractory_duration = 0.5
 *
 * # value each neuron is initialized with
 * initial_membrane_potential = -55.0
 *
 * # minimum potential at which a neuron will fire
 * activation_threshold = -55.0
 *
 * # value that each neuron is set to after firing
 * refractory_membrane_potential = -70.0
 *
 * #  tau for the linearlization of the decay function
 * tau = 100.0
 *
 * #  poisson_prob_of_success
 * poisson_prob_of_success = 0.0001
 * [debug]
 * # Options are
 * # NONE
 * # INFO
 * # DEBUG
 * # DEBUG2
 * # DEBUG3
 * # DEBUG4
 * level = "NONE"
 *
 * [random]
 * # options are 'time' for using the current time, or an integer (as a string
 * e.g. "1")
 * seed = "time"
 *
 * [runtime_vars]
 * # in seconds
 * runtime = 20
 * # file to read input from
 * input_file = "./input_files/test"
 * # format should be "x..y" for reading lines x to y (inclusive) or just x for
 * a single line
 * line_range = "1..10"
 * ```
 *
 * </details>
 *
 *
 */
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
  bool LIMIT_LOG_OUTPUT;
  bool show_stimulus;
  double TAU;
  int REFRACTORY_DURATION;
  double DECAY_VALUE;
  double INPUT_PROB_SUCCESS;
  LogLevel DEBUG_LEVEL;
  std::string INPUT_FILE;
  std::string CONFIG_FILE;
  std::string OUTPUT_FILE;
  ostream &STREAM = std::cout;
  int num_stimulus;
  int time_per_stimulus;
  int max_latency;
  int max_synapse_delay;
  int min_synapse_delay;
  double max_weight;

public:
  vector<int> parse_line_range(const std::string &in);
  int setOptions();
  void setOptions(std::map<std::string, double> &);
  void generateNewConfig();
  void useBaseToml();
  int parseArgs(std::vector<std::string>);
  void checkStartCond();

  RuntimConfig(SNN *snn) : snn(snn){};

private:
  SNN *snn;
};

#endif // !GLOBALS
