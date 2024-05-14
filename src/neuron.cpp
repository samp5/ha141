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

Neuron::Neuron(int _id, NeuronGroup *group, Neuron_t type) {
  this->type = type;
  this->id = _id;
  this->group = group;
  this->membrane_potential = cf.INITIAL_MEMBRANE_POTENTIAL;
  this->excit_inhib_value = this->generateInhibitoryStatus();
  this->last_decay = -1;

  const char *inhib =
      this->excit_inhib_value == -1 ? "excitatory\0" : "inhibitory\0";

  lg.log_group_neuron_type(INFO, "(%d) Neuron %d added: %s",
                           this->group->get_id(), _id, inhib);
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
    lg.add_data(*data);
    delete data;
  }
}

// Adds neuron to the _postsynaptic map with weight
//
// Adds a neighbor neuron to the _postsynaptic map of the calling
// instance. The calling instance is then added to the _presynaptic
// map of the neighbor neuron via Neuron::add_previous(Neuron*, double);
//
// Prints log of edge added
//
// @param1: Pointer to neighbor neuron
// @param2: Weight for that edge
void Neuron::addNeighbor(Neuron *neighbor, double weight) {

  if (neighbor->getType() == Input) {
    lg.log(ERROR, "Connection to Input type Neuron... quitting");
    exit(1);
  }

  Synapse *new_connection = new Synapse(this, neighbor, weight);
  Synapse *return_record = new Synapse(neighbor, this, weight);

  this->addPostSynapticConnection(new_connection);
  this->addPreSynapticConnection(return_record);

  if (!neighbor->getGroup()) {
    lg.log_neuron_interaction(INFO, "Edge from Neuron %d to Neuron %d added.",
                              id, neighbor->getID());
  } else {
    lg.log_group_neuron_interaction(
        INFO, "Edge from (%d) Neuron %d to (%d) Neuron %d added",
        this->getGroup()->get_id(), this->getID(),
        neighbor->getGroup()->get_id(), neighbor->getID());
  }
}

// Recivee all messages for this neuron
//
// Gets message and updates membrane_potential
// adds data to log and handles message memory
// deallocation
//
// should only be called in `run_in_group()`
//
// @returns 0 if all messages recieved, 1 otherwise
int Neuron::recieveMessage() {

  // Get message
  Message *incoming_message = this->retrieveMessage();

  if (incoming_message == NULL) {
    double time = lg.get_time_stamp();
    this->retroactiveDecay(this->last_decay, time);
    return 0;
  }

  this->retroactiveDecay(this->last_decay, incoming_message->timestamp);

  if (incoming_message->timestamp <
      this->refractory_start + cf.REFRACTORY_DURATION) {

    delete incoming_message;
    incoming_message = nullptr;

    lg.log_group_neuron_state(
        INFO, "(%d) Neuron %d is still in refractory period, ignoring message",
        this->getGroup()->get_id(), this->getID());

    return 1;
  }

  this->accumulatePotential(incoming_message->message);

  lg.log_group_neuron_value(
      INFO, "(%d) Neuron %d recieved message, accumulated equal to %f",
      this->group->get_id(), this->id, this->membrane_potential);

  // use message timestamp not current time
  lg.add_data(this->group->get_id(), this->id, this->membrane_potential,
              incoming_message->timestamp, this->getType(),
              incoming_message->message_type, this);

  // Deallocate this message
  delete incoming_message;
  incoming_message = nullptr;
  return 1;
}

void Neuron::sendMessages() {

  for (const auto synapse : getPostSynaptic()) {
    synapse->propagate();
  }

  lg.log_group_neuron_state(
      INFO,
      "(%d) Neuron %d reached activation threshold, entering refractory phase",
      this->group->get_id(), this->id);

  this->refractory();
}

// Run cycle for a neuron in a group
void Neuron::run() {
  while (this->recieveMessage()) {
    if (this->membrane_potential >= cf.ACTIVATION_THRESHOLD) {
      this->sendMessages();
    }
  }
  this->deactivate();
}
// Runs a refractory period for a Neuron
//
// Neuron sleeps for 2 milliseconds and potential is reset to
// value set by preprocessor directive REFRACTORY_MEMBRANE_POTENTIAL
void Neuron::refractory() {

  Message_t refractory_type = Refractory;

  this->refractory_start = lg.get_time_stamp();

  pthread_mutex_lock(&mx.potential);
  this->membrane_potential = cf.REFRACTORY_MEMBRANE_POTENTIAL;
  pthread_mutex_unlock(&mx.potential);

  lg.log_neuron_value(INFO,
                      "Neuron %d in refractory state: potential set to %f",
                      this->id, this->membrane_potential);

  lg.add_data(this->getGroup()->get_id(), this->getID(),
              cf.REFRACTORY_MEMBRANE_POTENTIAL, this->refractory_start,
              this->getType(), refractory_type, this);
}

bool Neuron::isActivated() const { return this->active; }
NeuronGroup *Neuron::getGroup() { return this->group; }

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

Message *Neuron::retrieveMessage() {

  pthread_mutex_lock(&mx.message);
  // Return if messages is empty
  if (this->messages.empty()) {
    pthread_mutex_unlock(&mx.message);
    lg.log_group_neuron_state(DEBUG,
                              "No additional messages for (%d) Neuron %d",
                              this->getGroup()->get_id(), this->getID());
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

// Decays a neuron based on DECAY_VALUE retroactively
//
// Adds data points at even intervals from `from` to `to`
//
// Logs the updated membrane_potential
//
// @returns new membrane_potential
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

    // #askpedram Decay happens regardles of refractory period
    // if (i < this->refractory_start + REFRACTORY_DURATION) {
    //   continue;
    // }

    double decay_value = (this->membrane_potential - v_rest) / tau;

    if (decay_value < 0 || decay_value < 0.0001) {
      continue;
    }

    this->accumulatePotential(-decay_value);

    lg.add_data(this->getGroup()->get_id(), this->getID(),
                this->membrane_potential, i, this->getType(),
                message_decay_type, this);
  }
  this->last_decay = i;
}

// Activates neuron
void Neuron::activate() {
  pthread_mutex_lock(&mx.activation);
  this->active = true;
  pthread_mutex_unlock(&mx.activation);
}

// deactivates neuron
void Neuron::deactivate() {
  pthread_mutex_lock(&mx.activation);
  this->active = false;
  pthread_mutex_unlock(&mx.activation);
}

// Set type of neuron
//
// @param1: Neuron_t
void Neuron::setType(Neuron_t type) { this->type = type; }

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
  this->last_decay = lg.get_time_stamp();
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
