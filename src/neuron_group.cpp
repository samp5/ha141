#include "input_neuron.hpp"
#include "log.hpp"
#include "network.hpp"
#include "neuron.hpp"
#include "runtime.hpp"
#include <algorithm>
#include <cstdlib>
#include <pthread.h>
#include <random>
#include <unistd.h>
#include <utility>

/**
 * @brief NeuronGroup constructor.
 *
 * Construct a NeuronGroup. Allocates all `Neuron`s and `InputNeuron`s.
 *
 * @param _id NeuronGroup ID
 */
NeuronGroup::NeuronGroup(int _id, int number_neurons, int number_input_neurons,
                         SNN *network)
    : id(_id), most_recent_timestamp(0), finished(false), network(network) {
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
    pthread_mutex_destroy(&finised_tex);
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

  // std::cout << id << " locked message_q_tex\n";
  pthread_mutex_lock(&message_q_tex);
  bool empty = message_q.empty();
  pthread_mutex_unlock(&message_q_tex);
  // std::cout << id << " unlocked message_q_tex\n";

  int last_timestamp = 0;
  while (!empty) {
    Message *message = getMessage();
    updateTimestamp(message->timestamp);

    // Check to see if we are done with this stimulus
    if (message->timestamp > network->getConfig()->time_per_stimulus) {
      delete message;
      // std::cout << id << " locked message_q_tex\n";
      pthread_mutex_lock(&message_q_tex);
      while (!message_q.empty()) {
        delete *message_q.begin();
        message_q.erase(message_q.begin());
      }
      message_q.clear();
      pthread_mutex_unlock(&message_q_tex);
      // std::cout << id << " unlocked message_q_tex\n";
      break;
    }

    // check for out of place messages
    if (message->timestamp < getTimestamp()) {
      switch (message->message_type) {
      case Message_t::From_Neighbor: {
        network->lg->message(
            ERROR,
            "\n\tGroup %d\n\tLast timestamp: %d \n\tMessage_t: "
            "%s \n\tFrom Group: %d \n\tTimestamp: %d",
            id, last_timestamp, message->message_type,
            message->presynaptic_neuron->getGroup()->getID(),
            message->timestamp);
        break;
      }
      case Message_t::Stimulus: {
        network->lg->message(
            ERROR,
            "\n\tGroup %d\n\tLast timestamp: %d \n\tMessage_t: "
            "%s \n\tTimestamp: %d",
            id, last_timestamp, message->message_type, message->timestamp);
        break;
      }
      case Message_t::Refractory:
      case Message_t::Decay:
        break;
      }
    }

    // if we have an incoming intergroup connection
    if (!interGroupConnections.empty()) {
      IGlimit limiter = findLimitingGroup();
      while (limiter.timestamp < message->timestamp && limiter.limitingGroup) {
        // std::cout << id << " locked limit_tex for Group "
        //<< limiter.limitingGroup->getID() << "\n";

        if (limiter.limitingGroup->isFinished()) {
          break;
        }

        pthread_mutex_lock(&limiter.limitingGroup->getLimitTex());
        while (limiter.timestamp < message->timestamp) {
          // std::cout << id << " wating on limit_cond for Group "
          // << limiter.limitingGroup->getID()
          //<< "; limiting time: " << limiter.timestamp
          // << " message time: " << message->timestamp << "\n";
          if (limiter.limitingGroup->isFinished()) {
            break;
          }
          pthread_cond_wait(&limiter.getLimitCond(), &limiter.getLimitTex());
          limiter.updateTimestamp();
        }
        // std::cout << id << " finished waiting on limit_cond for Group "
        // << limiter.limitingGroup->getID() << "\n";
        pthread_mutex_unlock(&limiter.limitingGroup->getLimitTex());
        // std::cout << id << " unlocked limit_tex for Group "
        //<< limiter.limitingGroup->getID() << "\n";

        limiter = findLimitingGroup();
        if (!limiter.limitingGroup) {
          break;
        }
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

    // std::cout << id << " locked message_q_tex\n";
    pthread_mutex_lock(&message_q_tex);
    empty = message_q.empty();
    pthread_mutex_unlock(&message_q_tex);
    // std::cout << id << " unlocked message_q_tex\n";

    // std::cout << id << " broadcasting its limit_cond\n";
    pthread_cond_broadcast(&limit_cond);
  }
  // std::cout << id << " finished its message q\n";
  finished = true;
  updateTimestamp(network->getConfig()->time_per_stimulus + 1);
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
    // std::cout << id << " locked message_q_tex\n";
    pthread_mutex_lock(&message_q_tex);
    bool empty = message_q.empty();
    pthread_mutex_unlock(&message_q_tex);
    // std::cout << id << " unlocked message_q_tex\n";
    if (!empty) {
      network->lg->log(ERROR, "Message queue not empty at time of reset");
      for (auto m : message_q) {
        std::cout << "from " << m->presynaptic_neuron->getGroup()->getID()
                  << " \n";
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
  // std::cout << id << " locked its time_stamp_tex\n";
  most_recent_timestamp = mr;
  pthread_mutex_unlock(&time_stamp_tex);
  // std::cout << id << " unlocked its time_stamp_tex\n";
}
int NeuronGroup::getTimestamp() {
  pthread_mutex_lock(&time_stamp_tex);
  // std::cout << id << " locked its time_stamp_tex\n";
  int mr = most_recent_timestamp;
  pthread_mutex_unlock(&time_stamp_tex);
  // std::cout << id << " unlocked its time_stamp_tex\n";

  return mr;
}
IGlimit NeuronGroup::findLimitingGroup() {
  int min = INT_MAX;
  NeuronGroup *limiter = nullptr;
  IGlimit ret(limiter, min);
  for (auto g : interGroupConnections) {
    if (g->isFinished()) {
      continue;
    }
    int t = g->getTimestamp();
    if (t < ret.timestamp) {
      ret.timestamp = t;
      ret.limitingGroup = g;
    }
  }
  return ret;
}
