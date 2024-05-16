#include "neuron.hpp"
#include "log.hpp"
#include "message.hpp"
#include "runtime.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>

extern RuntimConfig cf;
extern Mutex mx;

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

Neuron::Neuron(int _id, NeuronGroup *group, Neuron_t type) {
  this->type = type;
  this->id = _id;
  this->group = group;
  this->membrane_potential = cf.INITIAL_MEMBRANE_POTENTIAL;
  this->excit_inhib_value = this->generateInhibitoryStatus();
  this->last_decay = -1;

  const char *inhib =
      this->excit_inhib_value == -1 ? "excitatory\0" : "inhibitory\0";

  lg.neuronType(INFO, "(%d) Neuron %d added: %s", this->group->getID(), _id,
                inhib);
}

Neuron::~Neuron() {
  std::for_each(this->PostSynapticConnnections.begin(),
                this->PostSynapticConnnections.end(),
                [](Synapse *syn) { delete syn; });
  std::for_each(this->PreSynapticConnections.begin(),
                this->PreSynapticConnections.end(),
                [](Synapse *syn) { delete syn; });
}

/**
 * @brief Transfer data to Log.
 *
 * Transfers data from thread local Neuron::log_data
 * to global Log::log_data
 */
void Neuron::transferData() {
  for (auto data : this->log_data) {
    lg.addData(data);
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
    lg.log(ERROR, "Connection to Input type Neuron... quitting");
    exit(1);
  }

  Synapse *new_connection = new Synapse(this, neighbor, weight);
  Synapse *return_record = new Synapse(neighbor, this, weight);

  this->addPostSynapticConnection(new_connection);
  this->addPreSynapticConnection(return_record);

  lg.neuronInteraction(INFO, "Edge from (%d) Neuron %d to (%d) Neuron %d added",
                       this->getGroup()->getID(), this->getID(),
                       neighbor->getGroup()->getID(), neighbor->getID());
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
  Message *incoming_message = this->retrieveMessage();

  if (incoming_message == NULL) {
    double time = lg.time();
    this->retroactiveDecay(this->last_decay, time);
    return 0;
  }

  this->retroactiveDecay(this->last_decay, incoming_message->timestamp);

  if (incoming_message->timestamp <
      this->refractory_start + cf.REFRACTORY_DURATION) {

    delete incoming_message;
    incoming_message = nullptr;

    lg.groupNeuronState(
        INFO, "(%d) Neuron %d is still in refractory period, ignoring message",
        this->getGroup()->getID(), this->getID());

    return 1;
  }

  this->accumulatePotential(incoming_message->message);

  lg.neuronValue(INFO,
                 "(%d) Neuron %d recieved message, accumulated equal to %f",
                 this->group->getID(), this->id, this->membrane_potential);

  this->addData(incoming_message->timestamp, incoming_message->message_type);

  // Deallocate this message
  delete incoming_message;
  incoming_message = nullptr;
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

  lg.groupNeuronState(
      INFO,
      "(%d) Neuron %d reached activation threshold, entering refractory phase",
      this->group->getID(), this->id);

  this->refractory();
}

/**
 * @brief Main run cycle for a Neuron.
 *
 * Checks Neuron::membrane_potential and calls Neuron::sendMessages if over the
 * RuntimConfig::ACTIVATION_THRESHOLD
 *
 */
void Neuron::run() {
  while (this->recieveMessage()) {
    if (this->membrane_potential >= cf.ACTIVATION_THRESHOLD) {
      this->sendMessages();
    }
  }
  this->deactivate();
}

/**
 * @brief Starts the refractory period for a Neuron.
 *
 * Sets the Neuron::membrane_potential to
 * RuntimConfig::REFRACTORY_MEMBRANE_POTENTIAL and logs a refractory stage
 *
 */
void Neuron::refractory() {
  this->refractory_start = lg.time();

  pthread_mutex_lock(&mx.potential);
  this->membrane_potential = cf.REFRACTORY_MEMBRANE_POTENTIAL;
  pthread_mutex_unlock(&mx.potential);

  lg.neuronValue(INFO,
                 "(%d) Neuron %d in refractory state: potential set to %f",
                 group->getID(), id, membrane_potential);

  this->addData(refractory_start, Message_t::Refractory);
}

/**
 * @brief Checks activation status of a Neuron.
 *
 * @return bool of activation status
 */
bool Neuron::isActivated() const { return this->active; }

/**
 * @brief Return a pointer to the owning group.
 *
 * @return Neuron::group
 */
NeuronGroup *Neuron::getGroup() { return this->group; }

/**
 * @brief Adds a message to the queue.
 *
 * Messages are added in a sorted order in order to ensure a FIFO model
 *
 * @param message pointer to a Message struct
 */
void Neuron::addMessage(Message *message) {

  if (this->messages.empty()) {
    pthread_mutex_lock(&mx.message);
    this->messages.push_back(message);
    pthread_mutex_unlock(&mx.message);

  } else {

    pthread_mutex_lock(&mx.message);
    list<Message *>::const_iterator it = this->messages.begin();

    while (it != this->messages.end() &&
           message->timestamp > (*it)->timestamp) {
      it++;
    }

    this->messages.insert(it, message);
    pthread_mutex_unlock(&mx.message);
  }
}

/**
 * @brief retrive the oldest Message from Neuron::messages.
 *
 * @return pointer to the Message struct
 */
Message *Neuron::retrieveMessage() {

  pthread_mutex_lock(&mx.message);
  // Return if messages is empty
  if (this->messages.empty()) {
    pthread_mutex_unlock(&mx.message);
    lg.groupNeuronState(DEBUG, "No additional messages for (%d) Neuron %d",
                        this->getGroup()->getID(), this->getID());
    return NULL;
  }
  pthread_mutex_unlock(&mx.message);

  // Get least recent message and remove it from the queue
  pthread_mutex_lock(&mx.message);
  Message *last = this->messages.front();
  this->messages.pop_front();
  pthread_mutex_unlock(&mx.message);

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
  double tau = cf.TAU;
  double v_rest = cf.REFRACTORY_MEMBRANE_POTENTIAL;

  if (from < 0) {
    this->last_decay = to;
    return;
  }

  double decay_time_step = 2e-3;

  Message_t message_decay_type = Decay;

  double first_decay = from;
  double i;

  for (i = first_decay; i < to; i += decay_time_step) {

    double decay_value = (this->membrane_potential - v_rest) / tau;

    if (decay_value < 0 || decay_value < 0.0001) {
      continue;
    }

    this->accumulatePotential(-decay_value);
    this->addData(i, message_decay_type);
  }
  this->last_decay = i;
}

void Neuron::activate() {
  pthread_mutex_lock(&mx.activation);
  this->active = true;
  pthread_mutex_unlock(&mx.activation);
}

void Neuron::deactivate() {
  pthread_mutex_lock(&mx.activation);
  this->active = false;
  pthread_mutex_unlock(&mx.activation);
}

/**
 * @brief Sets the type of a Neuron.
 *
 *
 * @param type Neuron_t type
 */
void Neuron::setType(Neuron_t type) { this->type = type; }

/**
 * @brief adds to the Neuron::membrane_potential.
 *
 * @param value value to be added
 */
void Neuron::accumulatePotential(double value) {
  pthread_mutex_lock(&mx.potential);
  this->membrane_potential += value;
  pthread_mutex_unlock(&mx.potential);
}

const list<Message *> &Neuron::getMessageVector() { return this->messages; }

double Neuron::getPotential() {
  pthread_mutex_lock(&mx.potential);
  double potential = membrane_potential;
  pthread_mutex_unlock(&mx.potential);

  return potential;
}

void Neuron::addPostSynapticConnection(Synapse *synapse) {
  this->PostSynapticConnnections.push_back(synapse);
}
void Neuron::addPreSynapticConnection(Synapse *synapse) {
  this->PreSynapticConnections.push_back(synapse);
}

void Neuron::reset() {
  this->membrane_potential = cf.INITIAL_MEMBRANE_POTENTIAL;
  for (auto message : this->messages) {
    delete message;
  }
  this->last_decay = lg.time();
  this->refractory_start = 0;
  this->messages.clear();
  this->deactivate();
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
  return this->PostSynapticConnnections;
}
const vector<Synapse *> &Neuron::getPresynaptic() const {
  return this->PreSynapticConnections;
}

void Neuron::addData(double time, Message_t message_type) {
  if (cf.LIMIT_LOG_OUTPUT && message_type == Message_t::Refractory) {
    LogData *d = new LogData(id, group->getID(), time, membrane_potential, type,
                             message_type, *cf.STIMULUS);
    log_data.push_back(d);
  } else if (!cf.LIMIT_LOG_OUTPUT) {
    LogData *d = new LogData(id, group->getID(), time, membrane_potential, type,
                             message_type, *cf.STIMULUS);
    log_data.push_back(d);
  }
}
