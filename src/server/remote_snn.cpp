#include "remote_snn.hpp"
#include "../runtime.hpp"
#include <cstdlib>
#include <cstring>
#include <sstream>

using std::istringstream;
rpcSNN::rpcSNN(ConfigDict dict) : SNN(), configDict(dict) {
  lg->setNetwork(this); // log gets allocated in SNN()
  config = new RuntimConfig(this);
  if (configDict.empty()) {
    configDict = getDefaultConfig();
  }
  config->setOptions(configDict);
  gen = std::mt19937(rd());
  gen.seed(config->RAND_SEED);
  image = nullptr; // we have to wait to initalize the images until we know
}

ConfigDict rpcSNN::getDefaultConfig() {

  ConfigDict dict = {{"neuron_count", 0},
                     {"input_neuron_count", 0},
                     {"group_count", 1},
                     {"edge_count", 0},
                     {"refractory_duration", 5},
                     {"initial_membrane_potential", -6.0},
                     {"activation_threshold", -5.0},
                     {"refractory_membrane_potential", -7.0},
                     {"tau", 100.0},
                     {"max_latency", 10},
                     {"max_synapse_delay", 2},
                     {"min_synapse_delay", 1},
                     {"max_weight", 10.0},
                     {"poisson_prob_of_success", 0.8},
                     {"debug_level", LogLevel::NONE},
                     {"limit_log_size", true},
                     {"show_stimulus", false},
                     {"time_per_stimulus", 200},
                     {"seed", -1},
                     {"rpc", false}};
  return dict;
}

void rpcSNN::generateImage() {
  /*
   * The passed buffer (which is at this point stored in pySNN::data
   * Has dimensions m x n, where m is the number of stimulus in this batch
   * and n is the number of input neurons required to observe the stimulus
   *
   * Here, we update the number of inputNeurons (that was previously parsed
   * from the configuraiton file) to match the required number specifed by
   * the passed buffer
   *
   * Alternatively, NUMBER_INPUT_NEURONS can be set by an initization function,
   * at which point the flow trusts the input vaue and enters the first if block
   * below
   */
  if (data.empty()) {
    lg->log(
        LogLevel::INFO,
        "pySNN::data is empty, trusting "
        "RuntimConfig::NUMBER_INPUT_NEURONS is correct (this value is set by "
        "either configuration file, or overridden by pySNN::initialize)");
  } else if (static_cast<size_t>(config->NUMBER_INPUT_NEURONS) !=
             data.front().size()) {
    lg->value(WARNING,
              "Number of input neurons does not equal the number of "
              "elements per stimulus, setting number of input neurons to %d",
              (int)data.front().size());
    config->NUMBER_INPUT_NEURONS = data.front().size();
  }

  lg->value(LogLevel::INFO, "NUMBER_INPUT_NEURONS is %d",
            config->NUMBER_INPUT_NEURONS);

  /*
   * The number of input neurons affects the calulated latency
   * via Image::getLatency.
   *
   * Test if the number of input neurons is a perfect square
   * otherwise using a rectangle with the smallest possible perimeter
   */
  if (Image::isSquare(config->NUMBER_INPUT_NEURONS)) {
    lg->log(ESSENTIAL, "Assuming square input image");
    image = new Image(config->NUMBER_INPUT_NEURONS, config->max_latency);
  } else {
    lg->log(ESSENTIAL,
            "Input image not square, using smallest perimeter rectangle");

    auto dimensions = Image::bestRectangle(config->NUMBER_INPUT_NEURONS);
    int x = dimensions.first;
    int y = dimensions.second;
    image = new Image(x, y, config->max_latency);
  }
}

void rpcSNN::initialize(char *packed_adj_dict, int number_input_neurons) {
  config->NUMBER_INPUT_NEURONS = number_input_neurons;
  /*
   * Generate and images based on the passed number of input neurons
   */
  generateImage();

  int numberNonInput = getNumberNeuronFromPackedData(packed_adj_dict);
  config->NUMBER_NEURONS = config->NUMBER_INPUT_NEURONS + numberNonInput;

  // Neurons per group
  int neuronPerGroup = config->NUMBER_NEURONS / config->NUMBER_GROUPS;
  int inputNeuronPerGroup =
      config->NUMBER_INPUT_NEURONS / config->NUMBER_GROUPS;

  // Remainders
  int neuronPerGroupRe = config->NUMBER_NEURONS % config->NUMBER_GROUPS;
  int inputNeuronPerGroupRe =
      config->NUMBER_INPUT_NEURONS % config->NUMBER_GROUPS;

  lg->log(LogLevel::INFO, "Adding NeuronGroups");

  for (int i = 0; i < config->NUMBER_GROUPS; i++) {
    int npgRe = neuronPerGroupRe ? 1 : 0;       // apply a paritial remainder?
    int inpgRe = inputNeuronPerGroupRe ? 1 : 0; // apply a paritial remainder?

    NeuronGroup *this_group = new NeuronGroup(
        i + 1, neuronPerGroup + npgRe, inputNeuronPerGroup + inpgRe, this);

    groups.push_back(this_group);
  }

  generateAllNeuronVec();
  generateInputNeuronVec();
  setInputNeuronLatency();
  generateNonInputNeuronVec();

  int numEdges = addEdgesFromPackedData(packed_adj_dict);
  // add neurons from the packed data

  // add inputNeuron Connections
  size_t max_size = std::min(input_neurons.size(), nonInputNeurons.size());
  if (max_size == 0) {
    lg->log(LogLevel::ERROR,
            "pySNN::updateEdgeWeights: computed min size of inputNeuron vector "
            "and nonInputNeurons vector is 0?");
    return;
  }

  for (size_t i = 0; i < max_size; i++) {
    InputNeuron *origin = input_neurons.at(i);
    Neuron *destination = nonInputNeurons.at(i);
    origin->addNeighbor(destination, 1, 0);
    numEdges++;
  }

  config->NUMBER_EDGES = numEdges;
}

// so cursed
int rpcSNN::getNumberNeuronFromPackedData(char *packed_data) {
  int length = strlen(packed_data);
  if (length < 2) {
    lg->log(ERROR, "Passed empty packed_data to "
                   "rpcSNN::set_number_neuron_from_packed_data");
    exit(1);
  }
  size_t index = length - 2; // get rid of the last \n
  while (index > 0 && packed_data[index] != '\n') {
    index--;
  }
  if (index == 0) {
    return 1;
  }

  // packed_data + index + 1 is the character after the last \n
  // so for ....\n384,30:0.15:5,15:7.6:4,...,
  // we are just grabbing 384 which is the max index.
  // then, there are max_index + 1 neurons represented in the packed data

  std::istringstream s(packed_data + index + 1);
  int max_index;
  s >> max_index;
  return max_index + 1;
}

int rpcSNN::addEdgesFromPackedData(char *packed_adj_dict) {
  std::istringstream s_stream(packed_adj_dict);

  int numEdges = 0;

  int origin_index;      // current origin index
  int destination_index; // current destination index
  double weight;         // current weight
  double delay;          // current delay

  while (s_stream >> origin_index) {
    // std::cout << "origin index extracted: " << origin_index << "\n";

    if (isspace(s_stream.peek())) { // this will hit on nodes without neighbors
      // std::cout << "\n";
      continue;
    }
    s_stream.ignore(1); // ignore comma

    // sequence is guarenteed to start with a number
    while (s_stream >> destination_index) {
      weight = -1;
      s_stream.ignore(1); // ignore colon
      if (isdigit(s_stream.peek())) {
        s_stream >> weight;
      }
      delay = -1;
      s_stream.ignore(1); // ignore colon
      if (isdigit(s_stream.peek())) {
        s_stream >> delay;
      }
      // std::cout << "neighbor: " << destination_index << " weight: " << weight
      //           << " delay: " << delay << "\n";
      numEdges += 1;
      nonInputNeurons.at(origin_index)
          ->addNeighbor(nonInputNeurons.at(destination_index), weight, delay);

      if (isspace(s_stream.peek())) { // this will only happen with the last
                                      // neighbor
        break;
      }
      s_stream.ignore(1); // ignore comma
    }
    // std::cout << "\n";
  }
  return numEdges;
}
