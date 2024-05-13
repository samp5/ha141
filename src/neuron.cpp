#include "neuron.hpp"
#include "functions.hpp"
#include "globals.hpp"
#include "log.hpp"
#include "message.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>

extern RuntimConfig cf;
extern Mutex mx;

// Constructor for Neuron class for Neurons in Groups
//
// Sets ID, inhibitory status, group pointer
// and prints out a log message
//
// @param1: Neuron ID
// @param2: excitatory/inhibitory value (1 or -1)
// @param3: Pointer to the parent group
Neuron::Neuron(int _id, int inhibitory, NeuronGroup *group, Neuron_t type) {
  this->type = type;
  this->id = _id;
  this->excit_inhib_value = inhibitory;
  this->group = group;
  this->membrane_potential = cf.INITIAL_MEMBRANE_POTENTIAL;
  this->excit_inhib_value = inhibitory;
  this->last_decay = -1;

  const char *inhib = inhibitory == -1 ? "excitatory\0" : "inhibitory\0";

  lg.log_group_neuron_type(INFO, "(%d) Neuron %d added: %s",
                           this->group->get_id(), _id, inhib);
}

// Destructor for the Neuron class
//
// Destroys pthread conditional
//
Neuron::~Neuron() {
  pthread_cond_destroy(&cond);
  std::for_each(this->PostSynapticConnnections.begin(),
                this->PostSynapticConnnections.end(),
                [](Synapse *syn) { delete syn; });
  std::for_each(this->PreSynapticConnections.begin(),
                this->PreSynapticConnections.end(),
                [](Synapse *syn) { delete syn; });
}

void Neuron::transfer_data() {
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
int Neuron::recieve_in_group() {

  // Get message
  Message *incoming_message = this->get_message();

  if (incoming_message == NULL) {
    double time = lg.get_time_stamp();
    this->retroactive_decay(this->last_decay, time);
    return 0;
  }

  this->retroactive_decay(this->last_decay, incoming_message->timestamp);

  if (incoming_message->timestamp <
      this->refractory_start + cf.REFRACTORY_DURATION) {

    delete incoming_message;
    incoming_message = nullptr;

    lg.log_group_neuron_state(
        INFO, "(%d) Neuron %d is still in refractory period, ignoring message",
        this->getGroup()->get_id(), this->getID());

    return 1;
  }

  this->update_potential(incoming_message->message);

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

void Neuron::send_messages_in_group() {

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
void Neuron::run_in_group() {
  while (this->recieve_in_group()) {
    if (this->membrane_potential >= cf.ACTIVATION_THRESHOLD) {
      this->send_messages_in_group();
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
void Neuron::add_message(Message *message) {

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

Message *Neuron::get_message() {

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
void Neuron::retroactive_decay(double from, double to) {
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

    this->update_potential(-decay_value);

    lg.add_data(this->getGroup()->get_id(), this->getID(),
                this->membrane_potential, i, this->getType(),
                message_decay_type, this);
  }
  this->last_decay = i;
}

// Decays a neuron based on DECAY_VALUE
//
// Logs the updated membrane_potential
//
// @returns new membrane_potential
double Neuron::decay(double timestamp) {
  double tau = cf.TAU;
  double v_rest = cf.REFRACTORY_MEMBRANE_POTENTIAL;

  // for a membrane potential of  -55, tau = 10, decay value is 1.5
  double decay_value = (this->membrane_potential - v_rest) / tau;

  // if we sitting at -70 no need to decay
  if (std::abs(decay_value) < 0.01 || decay_value < 0) {
    return 0;
  }

  Message_t decay_type = Decay;

  this->update_potential(-decay_value);

  double potential = this->membrane_potential;

  lg.add_data(this->getGroup()->get_id(), this->getID(), potential, timestamp,
              this->getType(), decay_type, this);

  lg.log_group_neuron_value(
      DEBUG2, "(%d) Neuron %d is decaying. Decay value is %f",
      this->getGroup()->get_id(), this->getID(), decay_value);

  lg.log_group_neuron_value(
      DEBUG2, "(%d) Neuron %d decayed. Membrane potential now %f",
      this->getGroup()->get_id(), this->getID(), potential);

  return potential;
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
void Neuron::set_type(Neuron_t type) { this->type = type; }

void Neuron::update_potential(double value) {
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
