#include "neuron.hpp"
#include "log.hpp"
#include "message.hpp"
#include "network.hpp"
#include "runtime.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>

/**
 * @brief Construct a Neuron.
 *
 * Neuron is created with a random inhibitory status based on
 * Neuron::generateInhibitoryStatus
 *
 *
 * @param _id Neuron ID
 * @param group pointer to owning NeuronGroup
 * @param type Neuron_t type
 */

Neuron::Neuron(int _id, NeuronGroup *_g, Neuron_t _t) {
  type = _t;
  id = _id;
  group = _g;
  membrane_potential =
      _g->getNetwork()->getConfig()->INITIAL_MEMBRANE_POTENTIAL;
  excit_inhib_value = generateInhibitoryStatus();
  last_decay = -1;

  const char *inhib = excit_inhib_value == -1 ? "excitatory\0" : "inhibitory\0";

  _g->getNetwork()->lg->neuronType(INFO, "(%d) Neuron %d added: %s",
                                   group->getID(), _id, inhib);
}

Neuron::~Neuron() {
  std::for_each(PostSynapticConnnections.begin(),
                PostSynapticConnnections.end(),
                [](Synapse *syn) { delete syn; });
  std::for_each(PreSynapticConnections.begin(), PreSynapticConnections.end(),
                [](Synapse *syn) { delete syn; });
}

/**
 * @brief Transfer data to Log.
 *
 * Transfers data from thread local Neuron::log_data
 * to global Log::log_data
 */
void Neuron::transferData() {
  for (auto data : log_data) {
    group->getNetwork()->lg->addData(data);
  }
}

/**
 * @brief adds a Synapse to Neuron::PostSynapticConnnections.
 * Adds a Synapse to both the Presynaptic and Postsynapic Neuron respective
 * Synapse vectors. Either Neuron::PostSynapticConnnections or
 * Neuron::PreSynapticConnections
 *
 * @param neighbor Target connection
 * @param weight Weight for this edge
 */
void Neuron::addNeighbor(Neuron *neighbor, double weight) {

  if (neighbor->getType() == Input) {
    group->getNetwork()->lg->log(ERROR,
                                 "Connection to Input type Neuron... quitting");
    exit(1);
  }

  Synapse *new_connection = new Synapse(this, neighbor, weight);
  Synapse *return_record = new Synapse(neighbor, this, weight);

  addPostSynapticConnection(new_connection);
  addPreSynapticConnection(return_record);

  group->getNetwork()->lg->neuronInteraction(
      INFO, "Edge from (%d) Neuron %d to (%d) Neuron %d added",
      getGroup()->getID(), getID(), neighbor->getGroup()->getID(),
      neighbor->getID());
}

/**
 * @brief Retrieve and process all messages in the queue.
 *
 * For all the messages in Neuron::messages, process the message contents,
 * check membrane_potential status, and retroactively decay via
 * Neuron::retroactiveDecay
 *
 */
int Neuron::recieveMessage() {

  // Get message
  Message *incoming_message = retrieveMessage();

  if (incoming_message == NULL) {
    double time = group->getNetwork()->lg->time();
    retroactiveDecay(last_decay, time);
    return 0;
  }

  retroactiveDecay(last_decay, incoming_message->timestamp);

  if (incoming_message->timestamp <
      refractory_start +
          group->getNetwork()->getConfig()->REFRACTORY_DURATION) {

    // Deallocate this message
    if (incoming_message) {
      delete incoming_message;
      incoming_message = nullptr;
    }

    group->getNetwork()->lg->groupNeuronState(
        INFO, "(%d) Neuron %d is still in refractory period, ignoring message",
        getGroup()->getID(), getID());

    return 1;
  }

  accumulatePotential(incoming_message->message);

  group->getNetwork()->lg->neuronValue(
      INFO, "(%d) Neuron %d recieved message, accumulated equal to %f",
      group->getID(), id, membrane_potential);

  addData(incoming_message->timestamp, incoming_message->message_type);

  // Deallocate this message
  if (incoming_message) {
    delete incoming_message;
    incoming_message = nullptr;
  }
  return 1;
}

/**
 * @brief send Messages to all elements of Neuron::PostSynapticConnnections.
 *
 * Enters a refractory phase after sending all messages
 */
void Neuron::sendMessages() {

  for (const auto synapse : getPostSynaptic()) {
    synapse->propagate();
  }

  group->getNetwork()->lg->groupNeuronState(
      INFO,
      "(%d) Neuron %d reached activation threshold, entering refractory phase",
      group->getID(), id);

  refractory();
}

/**
 * @brief Main run cycle for a Neuron.
 *
 * Checks Neuron::membrane_potential and calls Neuron::sendMessages if over the
 * RuntimConfig::ACTIVATION_THRESHOLD
 *
 */
void Neuron::run() {
  while (recieveMessage()) {
    if (membrane_potential >=
        group->getNetwork()->getConfig()->ACTIVATION_THRESHOLD) {
      sendMessages();
    }
  }
  deactivate();
}

/**
 * @brief Starts the refractory period for a Neuron.
 *
 * Sets the Neuron::membrane_potential to
 * RuntimConfig::REFRACTORY_MEMBRANE_POTENTIAL and logs a refractory stage
 *
 */
void Neuron::refractory() {
  refractory_start = group->getNetwork()->lg->time();

  pthread_mutex_lock(&group->getNetwork()->getMutex()->potential);
  membrane_potential =
      group->getNetwork()->getConfig()->REFRACTORY_MEMBRANE_POTENTIAL;
  pthread_mutex_unlock(&group->getNetwork()->getMutex()->potential);

  group->getNetwork()->lg->neuronValue(
      INFO, "(%d) Neuron %d in refractory state: potential set to %f",
      group->getID(), id, membrane_potential);

  addData(refractory_start, Message_t::Refractory);
}

/**
 * @brief Checks activation status of a Neuron.
 *
 * @return bool of activation status
 */
bool Neuron::isActivated() const { return active; }

/**
 * @brief Return a pointer to the owning group.
 *
 * @return Neuron::group
 */
NeuronGroup *Neuron::getGroup() const { return group; }

/**
 * @brief Adds a message to the queue.
 *
 * Messages are added in a sorted order in order to ensure a FIFO model
 *
 * @param message pointer to a Message struct
 */
void Neuron::addMessage(Message *message) {

  if (messages.empty()) {
    pthread_mutex_lock(&group->getNetwork()->getMutex()->message);
    messages.push_back(message);
    pthread_mutex_unlock(&group->getNetwork()->getMutex()->message);

  } else {

    pthread_mutex_lock(&group->getNetwork()->getMutex()->message);
    list<Message *>::const_iterator it = messages.begin();

    while (it != messages.end() && message->timestamp > (*it)->timestamp) {
      it++;
    }

    messages.insert(it, message);
    pthread_mutex_unlock(&group->getNetwork()->getMutex()->message);
  }
}

/**
 * @brief retrive the oldest Message from Neuron::messages.
 *
 * @return pointer to the Message struct
 */
Message *Neuron::retrieveMessage() {

  pthread_mutex_lock(&group->getNetwork()->getMutex()->message);
  // Return if messages is empty
  if (messages.empty()) {
    pthread_mutex_unlock(&group->getNetwork()->getMutex()->message);
    group->getNetwork()->lg->groupNeuronState(
        DEBUG, "No additional messages for (%d) Neuron %d", getGroup()->getID(),
        getID());
    return NULL;
  }
  pthread_mutex_unlock(&group->getNetwork()->getMutex()->message);

  // Get least recent message and remove it from the queue
  pthread_mutex_lock(&group->getNetwork()->getMutex()->message);
  Message *last = messages.front();
  messages.pop_front();
  pthread_mutex_unlock(&group->getNetwork()->getMutex()->message);

  return last;
}

/**
 * @brief retroactively decays a Neuron::membrane_potential.
 *
 * retroactively decays a Neuron with a timestep of 2e-3 seconds
 *
 * @param from The starting timestamp from which to decay
 * @param to The ending timestamp to decay to
 */
void Neuron::retroactiveDecay(double from, double to) {
  double tau = group->getNetwork()->getConfig()->TAU;
  double v_rest =
      group->getNetwork()->getConfig()->REFRACTORY_MEMBRANE_POTENTIAL;

  if (from < 0) {
    last_decay = to;
    return;
  }

  double decay_time_step = 2e-3;

  Message_t message_decay_type = Decay;

  double first_decay = from;
  double i;

  for (i = first_decay; i < to; i += decay_time_step) {

    double decay_value = (membrane_potential - v_rest) / tau;

    if (decay_value < 0 || decay_value < 0.0001) {
      continue;
    }

    accumulatePotential(-decay_value);
    addData(i, message_decay_type);
  }
  last_decay = i;
}

void Neuron::activate() {
  pthread_mutex_lock(&group->getNetwork()->getMutex()->activation);
  active = true;
  pthread_mutex_unlock(&group->getNetwork()->getMutex()->activation);
}

void Neuron::deactivate() {
  pthread_mutex_lock(&group->getNetwork()->getMutex()->activation);
  active = false;
  pthread_mutex_unlock(&group->getNetwork()->getMutex()->activation);
}

/**
 * @brief Sets the type of a Neuron.
 *
 *
 * @param type Neuron_t type
 */
void Neuron::setType(Neuron_t type) { type = type; }

/**
 * @brief adds to the Neuron::membrane_potential.
 *
 * @param value value to be added
 */
void Neuron::accumulatePotential(double value) {
  pthread_mutex_lock(&group->getNetwork()->getMutex()->potential);
  membrane_potential += value;
  pthread_mutex_unlock(&group->getNetwork()->getMutex()->potential);
}

const list<Message *> &Neuron::getMessageVector() const { return messages; }

double Neuron::getPotential() const {
  pthread_mutex_lock(&group->getNetwork()->getMutex()->potential);
  double potential = membrane_potential;
  pthread_mutex_unlock(&group->getNetwork()->getMutex()->potential);

  return potential;
}

void Neuron::addPostSynapticConnection(Synapse *synapse) {
  PostSynapticConnnections.push_back(synapse);
}
void Neuron::addPreSynapticConnection(Synapse *synapse) {
  PreSynapticConnections.push_back(synapse);
}

void Neuron::reset() {

  pthread_mutex_lock(&getGroup()->getNetwork()->getMutex()->potential);
  membrane_potential =
      group->getNetwork()->getConfig()->INITIAL_MEMBRANE_POTENTIAL;
  pthread_mutex_unlock(&getGroup()->getNetwork()->getMutex()->potential);

  pthread_mutex_lock(&getGroup()->getNetwork()->getMutex()->message);
  for (auto message : messages) {
    if (message) {
      delete message;
      message = nullptr;
    }
  }
  messages.clear();
  pthread_mutex_unlock(&getGroup()->getNetwork()->getMutex()->message);

  last_decay = group->getNetwork()->lg->time();
  refractory_start = 0;
  deactivate();
}

int Neuron::generateInhibitoryStatus() {
  int ret;
  double x = (double)rand() / RAND_MAX;
  if (x >= 0.1) {
    ret = -1;
  } else {
    ret = 1;
  }
  return ret;
}

const vector<Synapse *> &Neuron::getPostSynaptic() const {
  return PostSynapticConnnections;
}
const vector<Synapse *> &Neuron::getPresynaptic() const {
  return PreSynapticConnections;
}

void Neuron::addData(double time, Message_t message_type) {
  if (group->getNetwork()->getConfig()->LIMIT_LOG_OUTPUT &&
      message_type == Message_t::Refractory) {
    LogData *d =
        new LogData(id, group->getID(), time, membrane_potential, type,
                    message_type, *group->getNetwork()->getConfig()->STIMULUS);
    log_data.push_back(d);
  } else if (!group->getNetwork()->getConfig()->LIMIT_LOG_OUTPUT) {
    LogData *d =
        new LogData(id, group->getID(), time, membrane_potential, type,
                    message_type, *group->getNetwork()->getConfig()->STIMULUS);
    log_data.push_back(d);
  }
}

double Neuron::getLastDecay() const { return last_decay; }
int Neuron::getBias() const { return excit_inhib_value; }
Neuron_t Neuron::getType() const { return type; }
int Neuron::getID() const { return id; }
const vector<Synapse *> &Neuron::getSynapses() const {
  return PostSynapticConnnections;
}
