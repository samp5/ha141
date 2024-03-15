#include "functions.hpp"
#include "log.hpp"
#include "message.hpp"

void print_group_maps(Neuron *neuron) {
  const weight_map *p_postsyntapic = neuron->get_postsynaptic();
  const weight_map *p_presyntapic = neuron->get_presynaptic();

  if (!p_postsyntapic->empty()) {
    // print post synaptic
    weight_map::const_iterator post_it = p_postsyntapic->begin();

    lg.log_group_neuron_state(DEBUG2, "      (%d) Neuron %d is connected to:",
                              neuron->get_group()->get_id(), neuron->get_id());

    while (post_it != p_postsyntapic->end()) {
      lg.log_group_neuron_state(DEBUG2, "         (%d) Neuron %d",
                                post_it->first->get_group()->get_id(),
                                post_it->first->get_id());
      ++post_it;
    }
  } else {
    lg.log_group_neuron_state(
        DEBUG3, "         (%d) Neuron %d has no outgoing connections",
        neuron->get_group()->get_id(), neuron->get_id());
  }

  if (!p_presyntapic->empty()) {
    // print pre synaptic
    weight_map::const_iterator pre_it = p_presyntapic->begin();
    lg.log_group_neuron_state(DEBUG2,
                              "      (%d) Neuron %d has connections from:",
                              neuron->get_group()->get_id(), neuron->get_id());
    while (pre_it != p_presyntapic->end()) {
      lg.log_group_neuron_state(DEBUG2, "        (%d) Neuron %d",
                                pre_it->first->get_group()->get_id(),
                                pre_it->first->get_id());
      ++pre_it;
    }
  } else {
    lg.log_group_neuron_state(
        DEBUG3, "         (%d) Neuron %d has no incoming connections",
        neuron->get_group()->get_id(), neuron->get_id());
  }
}

void print_maps(Neuron *neuron) {
  const weight_map *p_postsyntapic = neuron->get_postsynaptic();
  const weight_map *p_presyntapic = neuron->get_presynaptic();

  if (!p_postsyntapic->empty()) {
    // print post synaptic
    weight_map::const_iterator post_it = p_postsyntapic->begin();

    lg.log_neuron_state(DEBUG, "Neuron %d is connected to:", neuron->get_id());

    while (post_it != p_postsyntapic->end()) {
      lg.log_neuron_state(DEBUG, "    - Neuron %d", post_it->first->get_id());
      ++post_it;
    }
  }

  if (!p_presyntapic->empty()) {
    // print pre synaptic
    weight_map::const_iterator pre_it = p_presyntapic->begin();
    lg.log_neuron_state(DEBUG,
                        "Neuron %d has connections from:", neuron->get_id());
    while (pre_it != p_presyntapic->end()) {
      lg.log_neuron_state(DEBUG, "    - Neuron %d", pre_it->first->get_id());
      ++pre_it;
    }
  }
}

bool has_neighbor(Neuron *from_neuron, Neuron *to_neuron) {
  bool ret;
  const weight_map *p_postsyntapic = from_neuron->get_postsynaptic();
  const weight_map *p_presyntapic = from_neuron->get_presynaptic();

  // print maps as debugging measure
  print_maps(from_neuron);
  print_maps(to_neuron);

  // check for connection FROM from_neuron TO to_neuron
  if (p_postsyntapic->find(to_neuron) != p_postsyntapic->end()) {

    lg.log_neuron_interaction(
        DEBUG, "has_neighbor: Neuron %d is already connected to Neuron %d",
        from_neuron->get_id(), to_neuron->get_id());

    // if the to neuron is not already in the postsynaptic map
    ret = true;
  }
  // check for connections FROM to_neuron TO from_neuron
  else if (p_presyntapic->find(to_neuron) != p_presyntapic->end()) {
    lg.log_neuron_interaction(
        DEBUG, "has_neighbor: Neuron %d already has connection from Neuron %d",
        to_neuron->get_id(), from_neuron->get_id());

    // if the to_neuron is in the presynaptic list
    ret = true;
  } else {
    ret = false;
  }
  return ret;
}

bool has_neighbor_group(Neuron *from_neuron, Neuron *to_neuron) {
  bool ret;
  const weight_map *p_postsyntapic = from_neuron->get_postsynaptic();
  const weight_map *p_presyntapic = from_neuron->get_presynaptic();

  // print maps as debugging measure
  print_group_maps(from_neuron);
  print_group_maps(to_neuron);

  // check for connection FROM from_neuron TO to_neuron
  if (p_postsyntapic->find(to_neuron) != p_postsyntapic->end()) {

    lg.log_neuron_interaction(
        DEBUG, "has_neighbor: Neuron %d is already connected to Neuron %d",
        from_neuron->get_id(), to_neuron->get_id());

    // if the to neuron is not already in the postsynaptic map
    ret = true;
  }
  // check for connections FROM to_neuron TO from_neuron
  else if (p_presyntapic->find(to_neuron) != p_presyntapic->end()) {
    lg.log_neuron_interaction(
        DEBUG, "has_neighbor: Neuron %d already has connection from Neuron %d",
        to_neuron->get_id(), from_neuron->get_id());

    // if the to_neuron is in the presynaptic list
    ret = true;
  } else {
    ret = false;
  }
  return ret;
}

void random_neighbors(vector<Neuron *> nodes, int number_neighbors) {

  lg.print("\nAdding Random Edges");
  lg.print("======================\n\n");
  int size = nodes.size();
  int i = 0;
  if (number_neighbors > size) {
    number_neighbors = size;
    lg.log(WARNING, "random_neighbors: Number of neighbors exceeds size, "
                    "setting number of neighbors to size");
  }

  while (i < number_neighbors) {

    // Get random neurons
    int from = rand() % size;
    int to = rand() % size;

    // check for self connections
    if (from == to) {
      continue;
    }
    if (has_neighbor(nodes[from], nodes[to])) {
      continue;
    }

    nodes[from]->add_neighbor(nodes[to], weight_function());
    i++;
  }
  cout << '\n';
}

void random_group_neighbors(vector<NeuronGroup *> groups,
                            int number_neighbors) {
  lg.print("\nAdding Random Edges");
  lg.print("======================\n");

  int neuron_count = get_neuron_count(groups);
  int i = 0;

  if (number_neighbors > neuron_count) {
    number_neighbors = neuron_count;
    lg.log(WARNING, "random_neighbors: Number of neighbors exceeds size, "
                    "setting number of neighbors to size");
  }

  while (i < number_neighbors) {

    // Get random neurons
    Neuron *from = get_random_neuron(groups);
    Neuron *to = get_random_neuron(groups);

    // check for self connections
    if (from == to) {
      continue;
    }
    if (has_neighbor_group(from, to)) {
      continue;
    }

    from->add_neighbor(to, weight_function());
    i++;
  }
}

int get_neuron_count(const vector<NeuronGroup *> &groups) {
  int size = 0;
  for (auto group : groups) {
    size += group->neuron_count();
  }
  return size;
}

Neuron *get_random_neuron(const vector<NeuronGroup *> &groups) {
  int group_number = rand() % groups.size();
  int neuron_number = rand() % groups[group_number]->neuron_count();
  return groups[group_number]->get_neruon_vector()[neuron_number];
}

void print_node_values(vector<Neuron *> nodes) {
  cout << "\nFinal Neuron Values\n";
  cout << "-------------------\n\n";
  for (Neuron *node : nodes) {
    lg.log_neuron_value(INFO, "Neuron %d : %f", node->get_id(),
                        node->get_potential());
  }
}

double weight_function() { return (double)rand() / RAND_MAX; }

int get_inhibitory_status() {
  int ret;
  double x = (double)rand() / RAND_MAX;
  if (x >= 0.2) {
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

void *decay_helper(void *groups) {
  decay_neurons((vector<NeuronGroup *> *)groups);
  return NULL;
}

void decay_neurons(vector<NeuronGroup *> *groups) {
  vector<Neuron *> neuron_vec;

  // make a vector of all available neurons
  for (const auto &group : *groups) {
    for (const auto &neuron : group->get_neruon_vector()) {
      neuron_vec.push_back(neuron);
    }
  }

  while (::active) {

    for (int i = 1; i <= WAIT_LOOPS; i++) {
      lg.log_value(DEBUG3, "decay_neurons waiting: %d", i);
      usleep(WAIT_TIME);
    }

    for (auto neuron : neuron_vec) {
      neuron->decay();
    }
  }
}

vector<Message *>
construct_message_vector_from_file(vector<NeuronGroup *> groups,
                                   std::string file_name) {
  vector<Neuron *> neuron_vec;
  vector<Message *> message_vector;

  // make a vector of all available neurons
  for (const auto &group : groups) {
    for (const auto &neuron : group->get_neruon_vector()) {
      neuron_vec.push_back(neuron);
    }
  }

  std::ifstream file(file_name);

  if (!file.is_open()) {
    lg.log(ERROR, "construct_message_vector_from_file: Unable to open file");
    return message_vector;
  }

  int number_neurons = neuron_vec.size();
  int data_read = 0;
  double value;

  // should make this a funciton parameter
  while (!file.eof() && data_read < number_neurons) {
    file >> value;
    message_vector.push_back(construct_message(value, neuron_vec[data_read]));
    data_read++;
  }

  return message_vector;
}

Message *construct_message(double value, Neuron *target) {
  Message *message = new Message;
  message->message = value;
  message->timestamp = 0;
  message->target_neuron = target;
  message->target_neuron_group = target->get_group();

  return message;
}
void print_message(Message *message) {
  lg.log_message(DEBUG3, "Message: %f %d %d %f", message->timestamp,
                 message->target_neuron_group->get_id(),
                 message->target_neuron->get_id(), message->message);
}

void *send_message_helper(void *messages) {
  send_messages((const vector<Message *> *)messages);
  return NULL;
}

void send_messages(const vector<Message *> *messages) {

  while (::active) {
    for (int i = 1; i <= WAIT_LOOPS; i++) {
      lg.log_value(DEBUG3, "send_messages waiting: %d", i);
      usleep(WAIT_TIME);
    }

    for (auto message : *messages) {

      lg.log_message(DEBUG2, "Adding Message: %f %d %d %f", message->timestamp,
                     message->target_neuron_group->get_id(),
                     message->target_neuron->get_id(), message->message);

      Message *message_copy =
          construct_message(message->message, message->target_neuron);
      message_copy->timestamp = lg.get_time_stamp();

      message->target_neuron->add_message(message_copy);
      message->target_neuron->activate();
    }
  }
  pthread_exit(NULL);
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
  file << '\n';
  file << "# number of neurons" << '\n';
  file << "neuron_count = 6" << '\n';
  file << '\n';
  file << "# number of groups" << '\n';
  file << "group_count = 2" << '\n';
  file << '\n';
  file << "# number of connections" << '\n';
  file << "edge_count = 4" << '\n';
  file << '\n';
  file << "# in milliseconds" << '\n';
  file << "wait_time = 100" << '\n';
  file << '\n';
  file << "# integer" << '\n';
  file << "wait_loops = 5" << '\n';
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
  file << "[debug]" << '\n';
  file << "# Options are" << '\n';
  file << "# INFO" << '\n';
  file << "# DEBUG" << '\n';
  file << "# DEBUG2" << '\n';
  file << "# DEBUG3" << '\n';
  file << "# DEBUG4" << '\n';
  file << "level = \"INFO\"" << '\n';
  file << '\n';
  file << "[decay]" << '\n';
  file << "# Value that each neuron decays every wait_time * wait_loops"
       << '\n';
  file << "value = 1.0" << '\n';
  file << '\n';
  file << "[random]" << '\n';
  file << "# options are 'time' for using the current time, or an integer (as "
          "a string e.g. \"1\")"
       << '\n';
  file << "seed = \"time\"" << '\n';
  file << '\n';
  file << "[runtime_vars]" << '\n';
  file << "# in seconds" << '\n';
  file << "runtime = 20" << '\n';

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
    lg.log(INFO,
           "No command line arguements detected: Using base_config.toml:");
    lg.log(INFO,
           "Usage: build/ex2 <filename> run build/ex2 --help for file format");
    use_base_toml();
    return 1;

  } else if (argc > 2) {
    lg.log(
        INFO,
        "Too many command line arguements detected: Using base_config.toml:");
    lg.log(INFO,
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

  toml::table tbl;

  try {
    tbl = toml::parse_file(file_name);
  } catch (const toml::parse_error &err) {
    lg.log_string(ERROR, "Parsing failed:", err.what());
    return 0;
  }

  if (tbl["runtime_vars"]["runtime"].as_integer()) {
    RUN_TIME = 1e6 * tbl["runtime_vars"]["runtime"].as_integer()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "runtime");
  }

  if (tbl["random"]["seed"].as_string()) {
    std::string seed = tbl["random"]["seed"].as_string()->get();
    if (seed == "time") {
      RAND_SEED = time(0);
    } else if (tbl["random"]["seed"].as_integer()) {
      RAND_SEED = tbl["random"]["seed"].as_integer()->get();
    }
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "random seed");
  }

  if (tbl["decay"]["value"].as_floating_point()) {
    DECAY_VALUE = tbl["decay"]["value"].as_floating_point()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "decay value");
  }

  if (tbl["runtime_vars"]["input_file"].as_string()) {
    std::string file = tbl["runtime_vars"]["input_file"].as_string()->get();
    INPUT_FILE = file;
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "input_file");
  }

  if (tbl["debug"]["level"].as_string()) {
    std::string level = tbl["debug"]["level"].as_string()->get();
    ::DEBUG_LEVEL = get_level_from_string(level);
  } else {
    lg.log_string(ERROR, "Failed to parse: %s",
                  "refractory_membrane_potential");
  }

  if (tbl["neuron"]["refractory_membrane_potential"].as_floating_point()) {
    REFRACTORY_MEMBRANE_POTENTIAL =
        tbl["neuron"]["refractory_membrane_potential"]
            .as_floating_point()
            ->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s",
                  "refractory_membrane_potential");
  }

  if (tbl["neuron"]["activation_threshold"].as_floating_point()) {
    ACTIVATION_THRESHOLD =
        tbl["neuron"]["activation_threshold"].as_floating_point()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "activation_threshold");
  }

  if (tbl["neuron"]["initial_membrane_potential"].as_floating_point()) {
    INITIAL_MEMBRANE_POTENTIAL =
        tbl["neuron"]["initial_membrane_potential"].as_floating_point()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "initial_membrane_potential");
  }

  if (tbl["neuron"]["wait_loops"].as_integer()) {
    WAIT_LOOPS = tbl["neuron"]["wait_loops"].as_integer()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "wait_loops");
  }

  if (tbl["neuron"]["wait_time"].as_integer()) {
    WAIT_TIME = 1000 * tbl["neuron"]["wait_time"].as_integer()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "wait_time");
  }

  if (tbl["neuron"]["edge_count"].as_integer()) {
    NUMBER_EDGES = tbl["neuron"]["edge_count"].as_integer()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "edge_count");
  }

  if (tbl["neuron"]["group_count"].as_integer()) {
    NUMBER_GROUPS = tbl["neuron"]["group_count"].as_integer()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "group_count");
  }

  if (tbl["neuron"]["neuron_count"].as_integer()) {
    NUMBER_NODES = tbl["neuron"]["neuron_count"].as_integer()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "neuron_count");
  }

  return 1;
}

LogLevel get_level_from_string(std::string level) {
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
