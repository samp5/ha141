#include "runtime.hpp"
#include "../run_config/toml.hpp"
#include "network.hpp"
#include <algorithm>

extern Log lg;

/**
 * @brief Destroy all mutexes in use.
 */
void Mutex::destroy_mutexes() {
  pthread_mutex_destroy(&potential);
  pthread_mutex_destroy(&log);
  pthread_mutex_destroy(&message);
  pthread_mutex_destroy(&activation);
  pthread_mutex_destroy(&stimulus);
}

/**
 * @brief Write a new config file to run_config/base_config.toml.
 *
 */
void RuntimConfig::generateNewConfig() {
  const char *file_name = "./run_config/base_config.toml";

  std::ofstream file;
  file.open(file_name);

  if (!file.is_open()) {
    lg.log(ERROR, "create_base_toml: Unable to open file");
    return;
  }

  file << "[neuron]" << '\n';
  file << "# NOTE: both the number of neurons and the number of input neurons"
       << '\n';
  file << "#       must be divisible by the number of groups" << '\n';
  file << '\n';
  file << "# number of neurons" << '\n';
  file << "neuron_count = 10" << '\n';
  file << '\n';
  file << "# number of input type neurons" << '\n';
  file << "input_neuron_count = 2" << '\n';
  file << '\n';
  file << "# number of groups" << '\n';
  file << "group_count = 2" << '\n';
  file << '\n';
  file << "# number of connections" << '\n';
  file << "# option can be \"MAX\" for maximum edges" << '\n';
  file << "edge_count = 1" << '\n';
  file << '\n';
  file << "# refractory_duration" << '\n';
  file << "refractory_duration = 0.5" << '\n';
  file << '\n';
  file << "# value each neuron is initialized with" << '\n';
  file << "initial_membrane_potential = -55.0" << '\n';
  file << '\n';
  file << "# minimum potential at which a neuron will fire" << '\n';
  file << "activation_threshold = -55.0" << '\n';
  file << '\n';
  file << "# value that each neuron is set to after firing" << '\n';
  file << "refractory_membrane_potential = -70.0" << '\n';
  file << '\n';
  file << "#  tau for the linearlization of the decay function\n";
  file << "tau = 100.0\n";
  file << '\n';
  file << "#  poisson_prob_of_success\n";
  file << "poisson_prob_of_success = 0.0001";
  file << '\n';
  file << "[debug]" << '\n';
  file << "# Options are" << '\n';
  file << "# NONE" << '\n';
  file << "# INFO" << '\n';
  file << "# DEBUG" << '\n';
  file << "# DEBUG2" << '\n';
  file << "# DEBUG3" << '\n';
  file << "# DEBUG4" << '\n';
  file << "level = \"INFO\"" << '\n';
  file << '\n';
  file << "[random]" << '\n';
  file << "# options are 'time' for using the current time, or an integer (as "
          "a string e.g. \"1\")"
       << '\n';
  file << "seed = \"time\"" << '\n';
  file << '\n';
  file << "[runtime_vars]" << '\n';
  file << "# in seconds" << '\n';
  file << "runtime = 1" << '\n';
  file << "# file to read input from" << '\n';
  file << "input_file = \"./input_files/test\"" << '\n';
  file << "# format should be \"x..y\" for reading lines x to y (inclusive) or "
          "just x for a single line"
       << '\n';
  file << "line_range = \"1\"" << '\n';

  file.close();
}

/**
 * Parse input line range in config file.
 *
 * @param in: string value of line range.
 * @return: Vector of line numbers to be read
 */
vector<int> RuntimConfig::parse_line_range(const std::string &in) {
  vector<int> ret;

  bool check1 = std::find(in.begin(), in.end(), '.') == in.end();
  bool check2 = std::all_of(in.begin(), in.end(), ::isdigit);

  if (check1) {
    if (check2) {
      ret.push_back(std::stoi(in));
    }
  } else {
    auto pos1 = in.find_first_of('.');
    auto pos2 = in.find_last_of('.');
    if (pos1 == std::string::npos || pos2 == std::string::npos) {
      lg.log(ERROR, "Cannot parse configuratio file. Not a valid range");
      return ret;
    }
    int start = std::stoi(in.substr(0, pos1));
    int end = std::stoi(in.substr(pos2 + 1));

    for (int i = start; i <= end; i++) {
      ret.push_back(i);
    }
  }
  return ret;
}

/**
 * Initializes all variables in RuntimeConfig based on
 * keys in toml file
 * @param file_name Name of confguration file
 */
int RuntimConfig::setOptions() {
  std::string file_name = this->CONFIG_FILE;

  toml::table tbl;

  try {
    tbl = toml::parse_file(file_name);
  } catch (const toml::parse_error &err) {
    lg.string(ERROR, "Parsing failed:", err.what());
    return 0;
  }

  if (tbl["neuron"]["poisson_prob_of_success"].as_floating_point()) {
    INPUT_PROB_SUCCESS =
        tbl["neuron"]["poisson_prob_of_success"].as_floating_point()->get();
  } else {
    lg.string(ERROR, "Failed to parse: %s", "poisson_prob_of_success");
  }

  if (tbl["neuron"]["refractory_duration"].as_floating_point()) {
    // convert to seconds
    REFRACTORY_DURATION =
        tbl["neuron"]["refractory_duration"].as_floating_point()->get() /
        1000.0f;
  } else {
    lg.string(ERROR, "Failed to parse: %s", "refractory_duration");
  }

  if (tbl["neuron"]["tau"].as_floating_point()) {
    TAU = tbl["neuron"]["tau"].as_floating_point()->get();
  } else {
    lg.string(ERROR, "Failed to parse: %s", "tau");
  }

  if (tbl["neuron"]["input_neuron_count"].as_integer()) {
    NUMBER_INPUT_NEURONS =
        tbl["neuron"]["input_neuron_count"].as_integer()->get();
  } else {
    lg.string(ERROR, "Failed to parse: %s", "input_neuron_count");
  }

  if (tbl["runtime_vars"]["line_range"].as_string()) {
    STIMULUS_VEC =
        parse_line_range(tbl["runtime_vars"]["line_range"].as_string()->get());
  } else {
    lg.string(ERROR, "Failed to parse: %s", "line_range");
  }

  if (tbl["runtime_vars"]["runtime"].as_integer()) {
    RUN_TIME = 1e6 * tbl["runtime_vars"]["runtime"].as_integer()->get();
  } else {
    lg.string(ERROR, "Failed to parse: %s", "runtime");
  }

  if (tbl["random"]["seed"].as_string()) {
    std::string seed = tbl["random"]["seed"].as_string()->get();
    if (seed == "time") {
      RAND_SEED = time(0);
    } else if (tbl["random"]["seed"].as_integer()) {
      RAND_SEED = tbl["random"]["seed"].as_integer()->get();
    }
  } else {
    lg.string(ERROR, "Failed to parse: %s", "random seed");
  }

  if (tbl["runtime_vars"]["input_file"].as_string()) {
    std::string file = tbl["runtime_vars"]["input_file"].as_string()->get();
    INPUT_FILE = file;
  } else {
    lg.string(ERROR, "Failed to parse: %s", "input_file");
  }

  if (tbl["debug"]["level"].as_string()) {
    std::string level = tbl["debug"]["level"].as_string()->get();
    DEBUG_LEVEL = lg.debugLevelString(level);
  } else {
    lg.string(ERROR, "Failed to parse: %s", "Debug level");
  }

  if (tbl["neuron"]["refractory_membrane_potential"].as_floating_point()) {
    REFRACTORY_MEMBRANE_POTENTIAL =
        tbl["neuron"]["refractory_membrane_potential"]
            .as_floating_point()
            ->get();
  } else {
    lg.string(ERROR, "Failed to parse: %s", "refractory_membrane_potential");
  }

  if (tbl["neuron"]["activation_threshold"].as_floating_point()) {
    ACTIVATION_THRESHOLD =
        tbl["neuron"]["activation_threshold"].as_floating_point()->get();
  } else {
    lg.string(ERROR, "Failed to parse: %s", "activation_threshold");
  }

  if (tbl["neuron"]["initial_membrane_potential"].as_floating_point()) {
    INITIAL_MEMBRANE_POTENTIAL =
        tbl["neuron"]["initial_membrane_potential"].as_floating_point()->get();
  } else {
    lg.string(ERROR, "Failed to parse: %s", "initial_membrane_potential");
  }

  if (tbl["neuron"]["group_count"].as_integer()) {
    NUMBER_GROUPS = tbl["neuron"]["group_count"].as_integer()->get();
  } else {
    lg.string(ERROR, "Failed to parse: %s", "group_count");
  }

  if (tbl["neuron"]["neuron_count"].as_integer()) {
    NUMBER_NEURONS = tbl["neuron"]["neuron_count"].as_integer()->get();
  } else {
    lg.string(ERROR, "Failed to parse: %s", "neuron_count");
  }

  if (tbl["neuron"]["edge_count"].as_string()) {
    std::string seed = tbl["neuron"]["edge_count"].as_string()->get();
    if (seed == "MAX") {
      NUMBER_EDGES = SNN::maximum_edges();
      lg.string(INFO, "Maximum edges selected, setting to %s",
                std::to_string(NUMBER_EDGES).c_str());
    }
  } else if (tbl["neuron"]["edge_count"].as_integer()) {
    NUMBER_EDGES = tbl["neuron"]["edge_count"].as_integer()->get();
  } else {
    lg.string(ERROR, "Failed to parse: %s", "edge_count");
  }

  num_stimulus = STIMULUS_VEC.size();
  time_per_stimulus = (double)cf.RUN_TIME / num_stimulus;
  STIMULUS = STIMULUS_VEC.begin();

  return 1;
}

bool file_exists(const char *file_name) {
  std::ifstream f(file_name);
  return f.good();
}

/**
 * @brief Use base_config.toml.
 *
 */
void RuntimConfig::useBaseToml() {
  if (!file_exists("./run_config/base_config.toml")) {
    generateNewConfig();
  } else {
    lg.log(DEBUG, "base_config.toml exists, using base_config.toml");
  }
  CONFIG_FILE = "./run_config/base_config.toml";
  setOptions();
}

/**
 * @brief Parse command line arguments.
 *
 * Valid command line arguments are
 *    - <file_name>
 *    - --help
 *
 * @return
 */
int RuntimConfig::parseArgs(char **argv, int argc) {
  if (argc == 1) {
    lg.log(ESSENTIAL,
           "No command line arguements detected: Using base_config.toml:");
    lg.log(ESSENTIAL,
           "Usage: build/ex2 <filename> run build/ex2 --help for file format");
    useBaseToml();
    return 1;

  } else if (argc > 2) {
    lg.log(
        ESSENTIAL,
        "Too many command line arguements detected: Using base_config.toml:");
    lg.log(ESSENTIAL,
           "Usage: build/ex2 <filename> run build/ex2 --help for more info");

    useBaseToml();
    return 1;
  }

  // check for help options
  if (!strcmp(argv[1], "--help")) {
    if (file_exists("./run_config/base_config.toml")) {
      lg.print("See run_config/base_config.toml for configuration options");
    } else {
      lg.print("Creating run_config/base_config.toml...");
      generateNewConfig();
      lg.print("See run_config/base_config.toml for configuration options");
    }
    return 0;
  }

  const char *path = "./run_config/%s";
  int length = snprintf(nullptr, 0, path, argv[1]);
  char *formatted_file_path = new char[length + 1];
  snprintf(formatted_file_path, length + 1, path, argv[1]);

  if (!file_exists(formatted_file_path)) {
    lg.string(ERROR, "File %s does not exists", formatted_file_path);
    return 0;
  }

  CONFIG_FILE = formatted_file_path;
  setOptions();
  delete[] formatted_file_path;

  return 1;
}

/**
 * @brief Checks various start conditions.
 *
 * The following conditions are checked
 *    - NUMBER_INPUT_NEURONS >= NUMBER_NEURONS
 *    - NUMBER_INPUT_NEURONS % NUMBER_GROUPS == 0
 *    - NUMBER_NEURONS & NUMBER_GROUPS == 0
 *    - NUMBER_EDGES <= SNN::maximum_edges()
 */
void RuntimConfig::checkStartCond() {
  bool error = false;
  if (NUMBER_INPUT_NEURONS >= NUMBER_NEURONS) {
    lg.log(ERROR, "NUMBER_INPUT_NEURONS greater than or equal to "
                  "NUMBER_NEURONS... quitting");
    error = true;
  } else if (NUMBER_NEURONS % NUMBER_GROUPS != 0) {
    lg.log(ERROR, "NUMBER_NEURONS not divisible by NUMBER_GROUPS... quitting");
    error = true;
  } else if (NUMBER_INPUT_NEURONS % NUMBER_GROUPS != 0) {
    lg.log(ERROR,
           "NUMBER_INPUT_NEURONS not divisible by NUMBER_GROUPS... quitting");
    error = true;
  } else if (NUMBER_EDGES > SNN::maximum_edges()) {
    lg.string(ERROR,
              "Maximum number of possible edges (%s) exceeded .. quitting",
              std::to_string(SNN::maximum_edges()).c_str());
    error = true;
  }
  if (error) {
    mx.destroy_mutexes();
    exit(0);
  } else {
    return;
  }
}
