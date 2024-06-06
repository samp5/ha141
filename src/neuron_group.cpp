#include "input_neuron.hpp"
#include "log.hpp"
#include "network.hpp"
#include "neuron.hpp"
#include "runtime.hpp"
#include <cstdlib>
#include <pthread.h>
#include <random>
#include <unistd.h>

/**
 * @brief NeuronGroup constructor.
 *
 * Construct a NeuronGroup. Allocates all `Neuron`s and `InputNeuron`s.
 *
 * @param _id NeuronGroup ID
 */
NeuronGroup::NeuronGroup(int _id, int number_neurons, int number_input_neurons,
                         SNN *network)
    : id(_id), most_recent_timestamp(0), network(network) {
  getNetwork()->lg->state(DEBUG, "Adding Group %d", _id);
  getNetwork()->lg->state(INFO, "Group %d", id);

  number_neurons -= number_input_neurons;
  int id = 1;
  while (number_neurons || number_input_neurons) {

    // 1 is regular, 0 is input
    int roll = rand() % 2;

    if (roll && number_neurons) {

      Neuron *neuron = new Neuron(id, this, Neuron_t::None);
      all_neurons.push_back(neuron);
      nI_neurons.push_back(neuron);
      number_neurons--;
      id++;

    } else if (!roll && number_input_neurons) {
      InputNeuron *neuron = new InputNeuron(id, this, 0);
      all_neurons.push_back(neuron);
      input_neurons.push_back(neuron);
      number_input_neurons--;
      id++;
    }
  }
}

/**
 * @brief Destructs NeuronGroup.
 *
 * NeuronGroup holds memory responsibility for `Neuron`s and deallocates them in
 * its destructor
 *
 */
NeuronGroup::~NeuronGroup() {
  for (auto neuron : all_neurons) {

    getNetwork()->lg->groupNeuronState(DEBUG, "Deleteing Group %d Neuron %d",
                                       id, neuron->getID());

    if (neuron) {
      delete neuron;
      neuron = nullptr;
    }

    pthread_mutex_destroy(&limit_tex);
    pthread_mutex_destroy(&time_stamp_tex);
    pthread_cond_destroy(&limit_cond);
  }
}

/**
 * @brief Main run cycle for a NeuronGroup.
 *
 * Checks the global bool ::active each cycle. Runs active neurons.
 * Before joining the main thread, transfers data from Neuron::log_data to
 * Log::log_data
 *
 */
void *NeuronGroup::run() {

  // Log running status
  getNetwork()->lg->state(INFO, "Group %d running", getID());

  bool hasInterGroup = !interGroupConnections.empty();

  pthread_mutex_lock(&message_q_tex);
  bool empty = message_q.empty();
  pthread_mutex_unlock(&message_q_tex);

  while (!empty) {

    Message *message = getMessage();

    if (message->timestamp < getTimestamp()) {
      logUnseqMessage(message, getTimestamp());
    }

    updateTimestamp(message->timestamp);

    if (hasInterGroup) {

      IGlimit limiter = findLimitingGroup();

      while (limiter.timestamp < message->timestamp) {
        pthread_mutex_lock(&limiter.limitingGroup->getLimitTex());
        while (limiter.timestamp < message->timestamp) {
          // network->lg->neuronInteraction(
          //     ESSENTIAL, "%d @ t-%d waiting on %d @ t-%d, ", id,
          //     message->timestamp, limiter.limitingGroup->getID(),
          //     limiter.timestamp);
          pthread_cond_broadcast(&limit_cond);
          pthread_cond_wait(&limiter.getLimitCond(), &limiter.getLimitTex());
          limiter.updateTimestamp();
        }
        pthread_mutex_unlock(&limiter.limitingGroup->getLimitTex());
        limiter = findLimitingGroup();
      }
    }

    // run neuron on message
    switch (message->post_synaptic_neuron->getType()) {
    case Neuron_t::Input: {
      InputNeuron *in =
          dynamic_cast<InputNeuron *>(message->post_synaptic_neuron);
      in->run(message);
      break;
    }
    case Neuron_t::None: {
      Neuron *n = message->post_synaptic_neuron;
      n->run(message);
      break;
    }
    }

    pthread_mutex_lock(&message_q_tex);
    empty = message_q.empty();
    pthread_mutex_unlock(&message_q_tex);

    pthread_cond_broadcast(&limit_cond);
  }
  updateTimestamp(network->getConfig()->time_per_stimulus +
                  network->getConfig()->max_synapse_delay);
  pthread_cond_broadcast(&limit_cond);

  return nullptr;
}

/**
 * @brief Get Neuron count of the NeuronGroup.
 *
 * @return Neuron count
 */
int NeuronGroup::neuronCount() const { return (int)all_neurons.size(); }

/**
 * @brief Get a mutable reference to the Neuron vector.
 *
 */
vector<Neuron *> &NeuronGroup::getMutNeuronVec() { return all_neurons; }

/**
 * @brief Get a mutable reference to the Neuron vector.
 *
 */
const vector<Neuron *> &NeuronGroup::getNeuronVec() const {
  return all_neurons;
}

/**
 * @brief Reset NeuronGroup.
 *
 * Resets all `Neuron`s
 *
 */
void NeuronGroup::reset() {
  for (auto neuron : all_neurons) {
    if (neuron->getType() == Input) {
      neuron = dynamic_cast<InputNeuron *>(neuron);
    }
    pthread_mutex_lock(&message_q_tex);
    bool empty = message_q.empty();
    pthread_mutex_unlock(&message_q_tex);

    if (!empty) {
      network->lg->log(WARNING, "Message queue not empty at time of reset");
      network->lg->groupNeuronState(WARNING, "Group %d intergroup connections",
                                    id, 0);
      for (auto g : interGroupConnections) {
        network->lg->state(WARNING, "\tGroup %d", g->getID());
      }
      network->lg->groupNeuronState(WARNING, "Group %d remaining messages", id,
                                    0);
      for (auto m : message_q) {
        network->lg->groupNeuronState(
            WARNING, "\tFrom: %d Time: %d",
            m->presynaptic_neuron->getGroup()->getID(), m->timestamp);
      }
    }
    neuron->reset();
  }
  pthread_mutex_lock(&time_stamp_tex);
  most_recent_timestamp = 0;
  pthread_mutex_unlock(&time_stamp_tex);
}

Message *NeuronGroup::getMessage() {
  pthread_mutex_lock(&message_q_tex);
  // std::cout << id << " locked message_q_tex\n";
  Message *ret = *message_q.begin();
  message_q.erase(message_q.begin());
  pthread_mutex_unlock(&message_q_tex);
  // std::cout << id << " unlocked message_q_tex\n";
  return ret;
}

void NeuronGroup::addToMessageQ(Message *message) {
  pthread_mutex_lock(&message_q_tex);
  // std::cout << id << " locked message_q_tex\n";
  message_q.insert(message);
  pthread_mutex_unlock(&message_q_tex);
  // std::cout << id << " unlocked message_q_tex\n";
}

int NeuronGroup::generateRandomSynapses(int number_edges) {
  // find non input neurons
  std::vector<Neuron *> nI_neurons;
  std::vector<InputNeuron *> input_neurons;
  for (auto n : all_neurons) {
    if (n->getType() != Neuron_t::Input) {
      nI_neurons.push_back(n);
    } else {
      input_neurons.push_back(dynamic_cast<InputNeuron *>(n));
    }
  }

  auto n_neurons = all_neurons.size();
  auto n_non_input = nI_neurons.size();
  if (number_edges > SNN::maximum_edges(input_neurons.size(), n_neurons)) {
    network->lg->log(
        LogLevel::DEBUG2,
        "Edges are intragroup only, adjusting edge count accordingly");
    number_edges = SNN::maximum_edges(input_neurons.size(), n_neurons);
  }

  // Initialize adjacency matrix
  typedef std::vector<std::vector<int>> Matrix;
  typedef std::vector<std::vector<int>>::size_type MatrixSz;

  Matrix mat(n_neurons, std::vector<int>(n_non_input, 0));

  auto gen = network->getGen();
  std::uniform_int_distribution<> rows(0, n_neurons - 1);
  std::uniform_int_distribution<> cols(0, n_non_input - 1);

  int number_connections = 0;
  while (number_connections < number_edges) {
    auto row = static_cast<MatrixSz>(rows(gen));
    auto col = static_cast<MatrixSz>(cols(gen));

    if (mat[row][col] || row == col) {
      continue;
    } else {
      mat[row][col] = 1;
      if (row < n_non_input) {
        mat[col][row] = -1;
      }
      number_connections += 1;
    }
  }
  // Add normal neurons
  for (std::size_t r = 0; r < n_non_input; r++) {
    Neuron *origin = nI_neurons[r];
    for (std::size_t c = 0; c < n_non_input; c++) {
      if (mat[r][c] == 1) {
        origin->addNeighbor(nI_neurons[c]);
      }
    }
  }
  // Add connections for input neurons
  for (std::size_t r = n_non_input; r < n_neurons; r++) {
    InputNeuron *origin = input_neurons.at(r - n_non_input);
    for (std::size_t c = 0; c < n_non_input; c++) {
      if (mat.at(r - n_non_input).at(c) == 1) {
        origin->addNeighbor(nI_neurons.at(c));
      }
    }
  }
  return number_edges;
}
void NeuronGroup::addInterGroupConnections(NeuronGroup *group) {
  interGroupConnections.push_back(group);
}
Neuron *NeuronGroup::getNonInputNeuron() const {
  static std::uniform_int_distribution<> index(0, nI_neurons.size() - 1);
  return nI_neurons[index(network->getGen())];
}
Neuron *NeuronGroup::getRandNeuron() const {
  static std::uniform_int_distribution<> index(0, all_neurons.size() - 1);
  return all_neurons[index(network->getGen())];
}

void NeuronGroup::updateTimestamp(int mr) {
  pthread_mutex_lock(&time_stamp_tex);
  most_recent_timestamp = mr;
  pthread_mutex_unlock(&time_stamp_tex);
}
int NeuronGroup::getTimestamp() {
  pthread_mutex_lock(&time_stamp_tex);
  int mr = most_recent_timestamp;
  pthread_mutex_unlock(&time_stamp_tex);

  return mr;
}
IGlimit NeuronGroup::findLimitingGroup() {
  int min = getTimestamp();
  NeuronGroup *limiter = nullptr;
  IGlimit ret(limiter, min);
  for (auto g : interGroupConnections) {
    int t = g->getTimestamp();
    if (t < ret.timestamp) {
      ret.timestamp = t;
      ret.limitingGroup = g;
    }
  }
  return ret;
}

void NeuronGroup::logUnseqMessage(Message *message, int last_timestamp) {
  switch (message->message_type) {
  case Message_t::From_Neighbor: {
    network->lg->message(ERROR,
                         "\n\tGroup %d\n\tLast timestamp: %d \n\tMessage_t: "
                         "%s \n\tFrom Group: %d \n\tTimestamp: %d",
                         id, last_timestamp, message->message_type,
                         message->presynaptic_neuron->getGroup()->getID(),
                         message->timestamp);
    break;
  }
  case Message_t::Stimulus: {
    network->lg->message(ERROR,
                         "\n\tGroup %d\n\tLast timestamp: %d \n\tMessage_t: "
                         "%s \n\tTimestamp: %d",
                         id, last_timestamp, message->message_type,
                         message->timestamp);
    break;
  }
  case Message_t::Refractory:
  case Message_t::Decay:
    break;
  }
}
