#include "functions.hpp"
#include "../run_config/toml.hpp"
#include "globals.hpp"
#include "input_neuron.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include <cctype>
#include <cstring>
#include <fstream>
#include <iterator>
#include <sstream>
#include <unordered_map>

extern RuntimConfig cf;
extern Mutex mx;

vector<int> parse_line_range(const std::string &in) {
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

bool has_synaptic_connection(Neuron *from_neuron, Neuron *to_neuron) {
  auto pPostsynaptic = from_neuron->getPostSynaptic();
  auto pPresynaptic = from_neuron->getPresynaptic();

  if (find_if(pPostsynaptic.begin(), pPostsynaptic.end(),
              [to_neuron](Synapse *syn) {
                return syn->getPostSynaptic() == to_neuron;
              }) == pPostsynaptic.end()) {

    return false;
  } else if (find_if(pPresynaptic.begin(), pPresynaptic.end(),
                     [to_neuron](Synapse *syn) {
                       return syn->getPreSynaptic() == to_neuron;
                     }) == pPresynaptic.end()) {
    return false;
  } else {
    return true;
  }
}

vector<Neuron *> construct_neuron_vector(const vector<NeuronGroup *> &groups) {
  vector<Neuron *> ret;
  for (auto group : groups) {
    for (auto neuron : group->get_neruon_vector()) {
      ret.push_back(neuron);
    }
  }
  return ret;
}

std::unordered_map<Neuron *, std::list<Neuron *>>
construct_neighbor_options(const vector<Neuron *> &neurons) {
  std::unordered_map<Neuron *, std::list<Neuron *>> map;
  for (auto neuron_origin : neurons) {
    std::list<Neuron *> origin_list;
    for (auto neuron_destination : neurons) {
      if (neuron_origin == neuron_destination) {
        continue;
      }
      if (neuron_destination->getType() == Input) {
        continue;
      }
      origin_list.push_back(neuron_destination);
    }
    map.insert({neuron_origin, origin_list});
  }
  return map;
}

std::list<Neuron *>::const_iterator find_in_list(std::list<Neuron *> &n_list,
                                                 Neuron *neuron) {
  return std::find(n_list.begin(), n_list.end(), neuron);
}

void random_synapses(vector<NeuronGroup *> &groups) {

  int synapses_formed = 0;
  auto neuron_vector = construct_neuron_vector(groups);
  auto map = construct_neighbor_options(neuron_vector);

  for (auto neuron : neuron_vector) {

    // get the neighbor_options for this neuron from the map
    std::list<Neuron *> &neighbor_options = map.at(neuron);
    if (neighbor_options.empty()) {
      continue;
    }

    // get a random neuron in this list
    auto target = neighbor_options.begin();
    std::advance(target, rand() % neighbor_options.size());

    // get the neighbor_options for the postsynaptic neuron and an iter to
    // this neuron
    std::list<Neuron *> &post_synaptic_neuron_options = map.at(*target);
    auto this_neuron = find_in_list(post_synaptic_neuron_options, neuron);

    // add postsynaptic neuron as a neighbor
    neuron->addNeighbor(*target, weight_function());

    // erase the postsynaptic neuron from the list and this neuron from the
    // postsynaptic list
    neighbor_options.erase(target);
    if (this_neuron != post_synaptic_neuron_options.end()) {
      post_synaptic_neuron_options.erase(this_neuron);
    }

    // increment the synapses
    synapses_formed += 1;

    // check to make sure we still need edges
    if (synapses_formed >= cf.NUMBER_EDGES) {
      break;
    }
  }
}

int get_neuron_count(const vector<NeuronGroup *> &groups) {
  int size = 0;
  for (auto group : groups) {
    size += group->neuron_count();
  }
  return size;
}

void print_node_values(vector<Neuron *> nodes) {
  cout << "\nFinal Neuron Values\n";
  cout << "-------------------\n\n";
  for (Neuron *node : nodes) {
    lg.log_neuron_value(INFO, "Neuron %d : %f", node->getID(),
                        node->getPotential());
  }
}

double weight_function() { return ((double)rand() / RAND_MAX) * 0.1; }

int get_inhibitory_value() {
  int ret;
  double x = (double)rand() / RAND_MAX;
  if (x >= 0.1) {
    ret = -1;
  } else {
    ret = 1;
  }
  return ret;
}

const char *get_active_status_string(bool active) {
  if (active) {
    const char *active = "active"; // NOLINT
    return active;
  } else {
    const char *inactive = "inactive"; // NOLINT
    return inactive;
  }
}

void construct_input_neuron_vector(const vector<NeuronGroup *> &groups,
                                   vector<InputNeuron *> &input_neurons) {
  if (!input_neurons.empty()) {
    lg.log(ERROR,
           "get_input_neuron_vector: this constructs a new vector! "
           "emptying vector that was passed...\n If Neurons in this vector "
           "were dynamically allocated, that memory has NOT been freed");
    input_neurons.clear();
  }

  for (const auto &group : groups) {
    for (auto neuron : group->get_neruon_vector()) {
      if (neuron->getType() != Input) {
        continue;
      }
      input_neurons.push_back(dynamic_cast<InputNeuron *>(neuron));
    }
  }
}

void get_line_x(std::string &line, int target) {
  static std::string file_name = cf.INPUT_FILE;
  static std::ifstream file(file_name);
  std::string temp;
  if (!file.is_open()) {
    lg.log(ERROR, "get_next_line: Unable to open file");
    return;
  }

  file.seekg(std::ios::beg);
  for (int i = 1; i < target; i++) {
    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }

  std::getline(file, temp);
  for (char ch : temp) {
    if (ch == ',') {
      continue;
    }
    line += ' ';
    line += ch;
  }
}

void get_next_line(std::string &line) {
  static std::string file_name = cf.INPUT_FILE;
  static std::ifstream file(file_name);
  std::string temp;

  if (!file.is_open()) {
    lg.log(ERROR, "get_next_line: Unable to open file");
    return;
  }

  // Any previous contents of @a \_\_str are erased.
  std::getline(file, temp);

  for (char ch : temp) {
    if (ch == ',') {
      continue;
    }
    line += ' ';
    line += ch;
  }
}

void set_line_x(const vector<InputNeuron *> &input_neurons, int target) {
  if (input_neurons.empty()) {
    lg.log(ESSENTIAL, "set_line_x: passed empty input neuron vector?");
    return;
  }

  std::string line;
  get_line_x(line, target);

  std::stringstream s(line);
  double value;

  for (InputNeuron *input_neuron : input_neurons) {
    s >> value;
    input_neuron->set_input_value(value);
  }
}

void set_next_line(const vector<InputNeuron *> &input_neurons) {
  if (input_neurons.empty()) {
    lg.log(ESSENTIAL, "set_next_line: passed empty input neuron vector?");
    return;
  }

  std::string line;
  get_next_line(line);

  std::stringstream s(line);
  double value;

  for (InputNeuron *input_neuron : input_neurons) {
    s >> value;
    input_neuron->set_input_value(value);
  }
}

void deallocate_message_vector(const vector<Message *> *messages) {
  for (auto message : *messages) {
    delete message;
  }
}

bool file_exists(const char *file_name) {
  std::ifstream f(file_name);
  return f.good();
}

void create_base_toml() {
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

void use_base_toml() {
  if (file_exists("./run_config/base_config.toml")) {
    lg.log(DEBUG, "base_config.toml exists, using base_config.toml");
    set_options("./run_config/base_config.toml");
    return;
  }
  create_base_toml();
  set_options("./run_config/base_config.toml");
}

int parse_command_line_args(char **argv, int argc) {
  if (argc == 1) {
    lg.log(ESSENTIAL,
           "No command line arguements detected: Using base_config.toml:");
    lg.log(ESSENTIAL,
           "Usage: build/ex2 <filename> run build/ex2 --help for file format");
    use_base_toml();
    return 1;

  } else if (argc > 2) {
    lg.log(
        ESSENTIAL,
        "Too many command line arguements detected: Using base_config.toml:");
    lg.log(ESSENTIAL,
           "Usage: build/ex2 <filename> run build/ex2 --help for more info");

    use_base_toml();
    return 1;
  }

  // check for help options
  if (!strcmp(argv[1], "--help")) {
    if (file_exists("./run_config/base_config.toml")) {
      lg.print("See run_config/base_config.toml for configuration options");
    } else {
      lg.print("Creating run_config/base_config.toml...");
      create_base_toml();
      lg.print("See run_config/base_config.toml for configuration options");
    }
    return 0;
  }

  // check for file input
  const char *path = "./run_config/%s";
  int length = snprintf(nullptr, 0, path, argv[1]);
  char *formatted_file_path = new char[length + 1];
  snprintf(formatted_file_path, length + 1, path, argv[1]);

  if (!file_exists(formatted_file_path)) {
    lg.log_string(ERROR, "File %s does not exists", formatted_file_path);
    return 0;
  }

  set_options(formatted_file_path);
  delete[] formatted_file_path;

  return 1;
}

int set_options(const char *file_name) {

  cf.CONFIG_FILE = file_name;
  toml::table tbl;

  try {
    tbl = toml::parse_file(file_name);
  } catch (const toml::parse_error &err) {
    lg.log_string(ERROR, "Parsing failed:", err.what());
    return 0;
  }

  if (tbl["neuron"]["poisson_prob_of_success"].as_floating_point()) {
    cf.INPUT_PROB_SUCCESS =
        tbl["neuron"]["poisson_prob_of_success"].as_floating_point()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "poisson_prob_of_success");
  }

  if (tbl["neuron"]["refractory_duration"].as_floating_point()) {
    // convert to seconds
    cf.REFRACTORY_DURATION =
        tbl["neuron"]["refractory_duration"].as_floating_point()->get() /
        1000.0f;
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "refractory_duration");
  }

  if (tbl["neuron"]["tau"].as_floating_point()) {
    cf.TAU = tbl["neuron"]["tau"].as_floating_point()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "tau");
  }

  if (tbl["neuron"]["input_neuron_count"].as_integer()) {
    cf.NUMBER_INPUT_NEURONS =
        tbl["neuron"]["input_neuron_count"].as_integer()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "input_neuron_count");
  }

  if (tbl["runtime_vars"]["line_range"].as_string()) {
    cf.STIMULUS_VEC =
        parse_line_range(tbl["runtime_vars"]["line_range"].as_string()->get());
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "line_range");
  }

  if (tbl["runtime_vars"]["runtime"].as_integer()) {
    cf.RUN_TIME = 1e6 * tbl["runtime_vars"]["runtime"].as_integer()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "runtime");
  }

  if (tbl["random"]["seed"].as_string()) {
    std::string seed = tbl["random"]["seed"].as_string()->get();
    if (seed == "time") {
      cf.RAND_SEED = time(0);
    } else if (tbl["random"]["seed"].as_integer()) {
      cf.RAND_SEED = tbl["random"]["seed"].as_integer()->get();
    }
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "random seed");
  }

  if (tbl["runtime_vars"]["input_file"].as_string()) {
    std::string file = tbl["runtime_vars"]["input_file"].as_string()->get();
    cf.INPUT_FILE = file;
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "input_file");
  }

  if (tbl["debug"]["level"].as_string()) {
    std::string level = tbl["debug"]["level"].as_string()->get();
    cf.DEBUG_LEVEL = get_level_from_string(level);
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "Debug level");
  }

  if (tbl["neuron"]["refractory_membrane_potential"].as_floating_point()) {
    cf.REFRACTORY_MEMBRANE_POTENTIAL =
        tbl["neuron"]["refractory_membrane_potential"]
            .as_floating_point()
            ->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s",
                  "refractory_membrane_potential");
  }

  if (tbl["neuron"]["activation_threshold"].as_floating_point()) {
    cf.ACTIVATION_THRESHOLD =
        tbl["neuron"]["activation_threshold"].as_floating_point()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "activation_threshold");
  }

  if (tbl["neuron"]["initial_membrane_potential"].as_floating_point()) {
    cf.INITIAL_MEMBRANE_POTENTIAL =
        tbl["neuron"]["initial_membrane_potential"].as_floating_point()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "initial_membrane_potential");
  }

  if (tbl["neuron"]["group_count"].as_integer()) {
    cf.NUMBER_GROUPS = tbl["neuron"]["group_count"].as_integer()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "group_count");
  }

  if (tbl["neuron"]["neuron_count"].as_integer()) {
    cf.NUMBER_NEURONS = tbl["neuron"]["neuron_count"].as_integer()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "neuron_count");
  }

  if (tbl["neuron"]["edge_count"].as_string()) {
    std::string seed = tbl["neuron"]["edge_count"].as_string()->get();
    if (seed == "MAX") {
      cf.NUMBER_EDGES = maximum_edges();
      lg.log_string(INFO, "Maximum edges selected, setting to %s",
                    std::to_string(cf.NUMBER_EDGES).c_str());
    }
  } else if (tbl["neuron"]["edge_count"].as_integer()) {
    cf.NUMBER_EDGES = tbl["neuron"]["edge_count"].as_integer()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "edge_count");
  }

  return 1;
}

LogLevel get_level_from_string(std::string level) {
  if (level == "NONE")
    return NONE;
  if (level == "INFO")
    return INFO;
  if (level == "DEBUG")
    return DEBUG;
  if (level == "DEBUG2")
    return DEBUG2;
  if (level == "DEBUG3")
    return DEBUG3;
  if (level == "DEBUG4")
    return DEBUG4;
  lg.log(WARNING, "\"level\" does not match any available options: setting "
                  "LogLevel to INFO");
  return INFO;
}

void assign_groups(vector<NeuronGroup *> &neuron_groups) {

  int neuron_per_group = cf.NUMBER_NEURONS / cf.NUMBER_GROUPS;
  int input_neurons_per_group = cf.NUMBER_INPUT_NEURONS / cf.NUMBER_GROUPS;

  for (int i = 0; i < cf.NUMBER_GROUPS; i++) {

    // allocate for this group
    NeuronGroup *this_group =
        new NeuronGroup(i + 1, neuron_per_group, input_neurons_per_group);

    // add to vector
    neuron_groups.at(i) = this_group;
  }
}

void assign_neuron_types(vector<NeuronGroup *> &groups) {
  typedef vector<Neuron *>::size_type vec_sz_t;
  vector<Neuron *> neuron_vec;

  lg.log(INFO, "Assiging Input neurons");

  // make a vector of all available neurons
  for (const auto &group : groups) {
    for (const auto &neuron : group->get_neruon_vector()) {
      neuron_vec.push_back(neuron);
    }
  }

  vec_sz_t vecsize = neuron_vec.size();

  int input_count = cf.NUMBER_INPUT_NEURONS;

  while (input_count) {

    vec_sz_t rand_index = rand() % vecsize;

    if (neuron_vec.at(rand_index)->getType() == None) {

      neuron_vec.at(rand_index)->set_type(Input);
      lg.log_group_neuron_type(DEBUG, "(%d) Neuron %d is a %s type",
                               neuron_vec.at(rand_index)->getGroup()->get_id(),
                               neuron_vec.at(rand_index)->getID(), "Input");

      input_count--;
    }
  }
}

std::string message_type_to_string(Message_t type) {
  std::string ret;
  switch (type) {
  case Stimulus:
    ret = "S";
    break;
  case Refractory:
    ret = "R";
    break;
  case From_Neighbor:
    ret = "N";
    break;
  case Decay:
    ret = "D";
    break;
  case Checked:
    ret = "C";
    break;
  }
  return ret;
}
std::string io_type_to_string(Neuron_t type) {
  std::string ret;
  switch (type) {
  case None:
    ret = "None";
    break;
  case Input:
    ret = "Input";
    break;
  case Hidden:
    ret = "Hidden";
    break;
  case Output:
    ret = "Output";
    break;
  }
  return ret;
}
void check_start_conditions() {
  bool error = false;
  if (cf.NUMBER_INPUT_NEURONS >= cf.NUMBER_NEURONS) {
    lg.log(ERROR, "NUMBER_INPUT_NEURONS greater than or equal to "
                  "NUMBER_NEURONS... quitting");
    error = true;
  } else if (cf.NUMBER_NEURONS % cf.NUMBER_GROUPS != 0) {
    lg.log(ERROR, "NUMBER_NEURONS not divisible by NUMBER_GROUPS... quitting");
    error = true;
  } else if (cf.NUMBER_INPUT_NEURONS % cf.NUMBER_GROUPS != 0) {
    lg.log(ERROR,
           "NUMBER_INPUT_NEURONS not divisible by NUMBER_GROUPS... quitting");
    error = true;
  } else if (cf.NUMBER_EDGES > maximum_edges()) {
    lg.log_string(ERROR,
                  "Maximum number of possible edges (%s) exceeded .. quitting",
                  std::to_string(maximum_edges()).c_str());
    error = true;
  }
  if (error) {
    mx.destroy_mutexes();
    exit(0);
  } else {
    return;
  }
}

int maximum_edges() {
  // for an undirected graph there are n(n-1) / 2 edges
  //
  // Since our connections are only allowed to go one way, the network is
  // essentially an undirected graph.
  //
  // input neurons can only have outgoing edges is
  // the parameter NUMBER_NEURONS represents the total number of neurons
  // then n_t = NUMBER_NEURONS, n_r = NUMBER_NEURONS - NUMBER_INPUT_NEURONS
  // n_i = NUMBER_INPUT_NEURONS
  //
  // the max edges are the total number of maximum edges minus the number of
  // edges that would be possible in a undirected graph of only the input
  // neurons maximum_edges = n_t(n_t) / 2 - n_i(n_i-1) / 2

  int n_i = cf.NUMBER_INPUT_NEURONS;
  int n_t = cf.NUMBER_NEURONS;

  // always even so division is fine
  int edges_lost_to_input = n_i * (n_i - 1) / 2;

  // maximum possible edges
  int max_edges = (n_t * (n_t - 1) / 2) - edges_lost_to_input;

  return max_edges;
}
