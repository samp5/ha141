#include "functions.hpp"
#include "input_neuron.hpp"

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

  int i = 0;
  while (i < number_neighbors) {

    // Get random neurons
    Neuron *from = get_random_neuron(groups);
    Neuron *to = get_random_neuron(groups, false);

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

Neuron *get_random_neuron(const vector<NeuronGroup *> &groups,
                          bool input_type_allowed) {

  typedef vector<Neuron *>::size_type vec_sz_t;

  vec_sz_t group_number = rand() % groups.size();

  if (input_type_allowed) {

    // If input type is allowed, normal thing
    int neuron_number = rand() % groups.at(group_number)->neuron_count();

    return groups.at(group_number)->get_neruon_vector().at(neuron_number);

  } else {

    lg.log(DEBUG4, "Finding non-input neuron...");

    bool try_again;
    unsigned long tries = 0;
    Neuron *ret_neuron;

    do {
      // make a vector to hold all the candiate neurons (non input)
      vector<Neuron *> group_neurons(
          groups[group_number]->get_neruon_vector().size());

      // copy all non-input neurons to new vector
      // fancy lamda function
      auto end = copy_if(groups.at(group_number)->get_neruon_vector().begin(),
                         groups.at(group_number)->get_neruon_vector().end(),
                         group_neurons.begin(), [](Neuron *neuron) {
                           return neuron->get_type() != Input;
                         });

      // shrink to fit
      group_neurons.resize(distance(group_neurons.begin(), end));

      lg.log_value(DEBUG4, "Group %d has %d non-input neuron(s)",
                   groups.at(group_number)->get_id(), group_neurons.size());

      // If there are no input neurons
      if (group_neurons.empty()) {
        lg.log_group_state(
            WARNING,
            "There are no non-input neurons in Group %d: trying a "
            "different group...",
            groups[group_number]->get_id());

        group_number = group_number == groups.size() ? 0 : group_number + 1;
        try_again = true;
        tries++;

      } else {
        try_again = false;
        ret_neuron = group_neurons.at(rand() % group_neurons.size());
      }
    } while (try_again && tries < groups.size());

    if (tries == groups.size()) {
      lg.log(ERROR, "Unable to find non-input neuron, exitting");
      exit(1);
    }

    return ret_neuron;
  }
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

    usleep(WAIT_TIME);

    for (auto neuron : neuron_vec) {
      double time_rn = lg.get_time_stamp();
      neuron->decay(time_rn);
    }
  }
}

void set_message_values_for_input_neurons(vector<NeuronGroup *> groups,
                                          std::string file_name) {
  vector<Neuron *> neuron_vec;

  for (const auto &group : groups) {
    for (const auto &neuron : group->get_neruon_vector()) {
      if (neuron->get_type() == Input) {
        neuron_vec.push_back(neuron);
      }
    }
  }

  std::ifstream file(file_name);

  if (!file.is_open()) {
    lg.log(ERROR, "set_message_values_for_input_neurons: Unable to open file");
    return;
  }

  int number_input_neurons = neuron_vec.size();
  int data_read = 0;
  double value;

  while (!file.eof() && data_read < number_input_neurons) {
    file >> value;

    InputNeuron *input_neuron =
        dynamic_cast<InputNeuron *>(neuron_vec.at(data_read));

    input_neuron->set_input_value(value);
    data_read++;
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
      if (neuron->get_type() == Input) {
        neuron_vec.push_back(neuron);
      }
    }
  }

  std::ifstream file(file_name);

  if (!file.is_open()) {
    lg.log(ERROR, "construct_message_vector_from_file: Unable to open file");
    return message_vector;
  }

  int number_input_neurons = neuron_vec.size();
  int data_read = 0;
  double value;

  while (!file.eof() && data_read < number_input_neurons) {
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
  message->post_synaptic_neuron = target;
  message->target_neuron_group = target->get_group();

  return message;
}

void print_message(Message *message) {
  lg.log_message(DEBUG3,
                 "Message: \n\tdummy_time:\t%f \n\ttarget neuron:\t%d "
                 "\n\ttarget_group:\t%d \n\tvalue:\t%f",
                 message->timestamp, message->target_neuron_group->get_id(),
                 message->post_synaptic_neuron->get_id(), message->message);
}

void *send_message_helper(void *messages) {
  send_messages((const vector<Message *> *)messages);
  return NULL;
}

void send_messages(const vector<Message *> *messages) {

  while (::active) {

    for (int i = 1; i <= WAIT_LOOPS; i++) {
      lg.log_value(DEBUG3, "send_messages waiting: %d", i);
    }

    // random numbers for the time_stamp
    //
    //

    for (auto message : *messages) {
      lg.log_message(DEBUG2,
                     "Adding Message! time: %f group: %d neuron: %d value: %f",
                     message->timestamp, message->target_neuron_group->get_id(),
                     message->post_synaptic_neuron->get_id(), message->message);

      Message *message_copy =
          construct_message(message->message, message->post_synaptic_neuron);
      message_copy->timestamp = lg.get_time_stamp();

      message->post_synaptic_neuron->add_message(message_copy);
      message->post_synaptic_neuron->activate();
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
  file << "# number of input type neurons" << '\n';
  file << "input_neuron_count = 3" << '\n';
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
  file << "#  tau for the linearlization of the decay function";
  file << "tau = 1.0";
  file << '\n';
  file << "#  poisson_prob_of_success";
  file << "poisson_prob_of_success = 0.7";
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

  CONFIG_FILE = file_name;
  toml::table tbl;

  try {
    tbl = toml::parse_file(file_name);
  } catch (const toml::parse_error &err) {
    lg.log_string(ERROR, "Parsing failed:", err.what());
    return 0;
  }

  if (tbl["neuron"]["poisson_prob_of_success"].as_floating_point()) {
    INPUT_PROB_SUCCESS =
        tbl["neuron"]["poisson_prob_of_success"].as_floating_point()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "poisson_prob_of_success");
  }

  if (tbl["neuron"]["refractory_duration"].as_floating_point()) {
    // convert to seconds
    REFRACTORY_DURATION =
        tbl["neuron"]["refractory_duration"].as_floating_point()->get() /
        1000.0f;
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "refractory_duration");
  }

  if (tbl["neuron"]["tau"].as_floating_point()) {
    TAU = tbl["neuron"]["tau"].as_floating_point()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "tau");
  }

  if (tbl["neuron"]["input_neuron_count"].as_integer()) {
    NUMBER_INPUT_NEURONS =
        tbl["neuron"]["input_neuron_count"].as_integer()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "input_neuron_count");
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
    NUMBER_NEURONS = tbl["neuron"]["neuron_count"].as_integer()->get();
  } else {
    lg.log_string(ERROR, "Failed to parse: %s", "neuron_count");
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

  int neuron_per_group = NUMBER_NEURONS / NUMBER_GROUPS;
  int input_neurons_per_group = NUMBER_INPUT_NEURONS / NUMBER_GROUPS;

  for (int i = 0; i < NUMBER_GROUPS; i++) {

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

  int input_count = NUMBER_INPUT_NEURONS;

  while (input_count) {

    vec_sz_t rand_index = rand() % vecsize;

    if (neuron_vec.at(rand_index)->get_type() == None) {

      neuron_vec.at(rand_index)->set_type(Input);
      lg.log_group_neuron_type(DEBUG, "(%d) Neuron %d is a %s type",
                               neuron_vec.at(rand_index)->get_group()->get_id(),
                               neuron_vec.at(rand_index)->get_id(), "Input");

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
  } else if (NUMBER_EDGES > maximum_edges()) {
    lg.log(ERROR, "Maximum number of possible edges exceeded .. quitting");
    error = true;
  }
  if (error) {
    destroy_mutexes();
    exit(0);
  } else {
    return;
  }
}

void destroy_mutexes() {
  pthread_mutex_destroy(&potential_mutex);
  pthread_mutex_destroy(&log_mutex);
  pthread_mutex_destroy(&message_mutex);
  pthread_mutex_destroy(&activation_mutex);
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

  int n_i = NUMBER_INPUT_NEURONS;
  int n_t = NUMBER_NEURONS;

  // always even so division is fine
  int edges_lost_to_input = n_i * (n_i - 1) / 2;

  // maximum possible edges
  int max_edges = (n_t * (n_t - 1) / 2) - edges_lost_to_input;

  return max_edges;
}
