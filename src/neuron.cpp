#include "neuron.hpp"
#include "functions.hpp"
#include "log.hpp"
#include "message.hpp"

#include <cmath>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>

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
  this->membrane_potential = INITIAL_MEMBRANE_POTENTIAL;
  this->excit_inhib_value = inhibitory;
  this->last_decay = -1;

  const char *inhib = inhibitory == -1 ? "excitatory\0" : "inhibitory\0";

  lg.log_group_neuron_type(INFO, "(%d) Neuron %d added: %s",
                           this->group->get_id(), _id, inhib);
}

// Constructor for Neuron class for Neurons in Groups
//
// Sets ID, inhibitory status, group pointer
// and prints out a log message
//
// @param1: Neuron ID
// @param2: excitatory/inhibitory value (1 or -1)
// @param3: Pointer to the parent group
Neuron::Neuron(int _id, int inhibitory, NeuronGroup *group) {

  this->type = None;
  this->id = _id;
  this->excit_inhib_value = inhibitory;
  this->group = group;
  this->membrane_potential = INITIAL_MEMBRANE_POTENTIAL;
  this->last_decay = lg.get_time_stamp();

  const char *inhib = inhibitory == -1 ? "excitatory\0" : "inhibitory\0";

  lg.log_group_neuron_type(INFO, "(%d) Neuron %d added: %s",
                           this->group->get_id(), _id, inhib);
}

// Constructor for Neuron class
//
// Sets ID, inhibitory status, and prints out a log message
//
// @param1: Neuron ID
// @param2: excitatory/inhibitory value (1 or -1)
Neuron::Neuron(int _id, int _excit_inhib_value) {
  this->type = None;
  this->id = _id;
  this->excit_inhib_value = _excit_inhib_value;
  this->membrane_potential = INITIAL_MEMBRANE_POTENTIAL;
  this->last_decay = lg.get_time_stamp();

  const char *inhib =
      _excit_inhib_value == -1 ? "excitatory\0" : "inhibitory\0";

  lg.log_neuron_type(INFO, "Neuron %d added: %s", _id, inhib);
}

// Destructor for the Neuron class
//
// Destroys pthread conditional
//
Neuron::~Neuron() { pthread_cond_destroy(&cond); }

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
void Neuron::add_neighbor(Neuron *neighbor, double weight) {

  if (neighbor->get_type() == Input) {
    lg.log(ERROR, "Connection to Input type Neuron... quitting");
    exit(1);
  }

  if (!neighbor->get_group()) {
    lg.log_neuron_interaction(INFO, "Edge from Neuron %d to Neuron %d added.",
                              id, neighbor->get_id());
  } else {
    lg.log_group_neuron_interaction(
        INFO, "Edge from (%d) Neuron %d to (%d) Neuron %d added",
        this->get_group()->get_id(), this->get_id(),
        neighbor->get_group()->get_id(), neighbor->get_id());
  }

  _postsynaptic[neighbor] = weight;

  // get pointer to this instance
  Neuron *this_neuron = this;

  // add to neighbor _presynaptic
  neighbor->add_previous(this_neuron, weight);
}

// Adds a neuron to the _presynaptic map with weight
//
// Adds the neighbor neuron to the _presynaptic map of the calling
// instance. This function should only be called via
// Neuron::add_neighbor(Neuron*, double)
//
// @param1: Pointer to neighbor neuron
// @param2: Weight for that edge
void Neuron::add_previous(Neuron *neighbor, double weight) {
  _presynaptic[neighbor] = weight;
  lg.log_neuron_interaction(
      DEBUG2, "Neuron %d added to the _presynaptic map of Neuron ",
      neighbor->get_id(), this->id);
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

  // Sanity check active status
  if (!this->active) {
    lg.log_group_neuron_state(
        ERROR,
        "run_in_group: Group %d: Neuron %d tried to run when it was not active",
        this->group->get_id(), this->id);
    return 0;
  }

  // Get message
  Message *incoming_message = this->get_message();

  if (incoming_message == NULL) {
    double time = lg.get_time_stamp();
    this->retroactive_decay(this->last_decay, time);
    return 0;
  }

  this->retroactive_decay(this->last_decay, incoming_message->timestamp);

  // Check message validity
  if (incoming_message->timestamp <
      this->refractory_start + REFRACTORY_DURATION) {

    if (!incoming_message) {
      lg.log_group_neuron_state(
          ERROR, "Tried to deallocate incoming message when message was NULL",
          this->get_group()->get_id(), this->get_id());
    }

    delete incoming_message;
    incoming_message = nullptr;

    lg.log_group_neuron_state(
        INFO, "(%d) Neuron %d is still in refractory period, ignoring message",
        this->get_group()->get_id(), this->get_id());

    return 1;
  }

  this->update_potential(incoming_message->message);

  lg.log_group_neuron_value(
      INFO, "(%d) Neuron %d recieved message, accumulated equal to %f",
      this->group->get_id(), this->id, this->membrane_potential);

  // use message timestamp not current time
  lg.add_data(this->group->get_id(), this->id, this->membrane_potential,
              incoming_message->timestamp, this->get_type(),
              incoming_message->message_type, this);

  // Deallocate this message
  if (!incoming_message) {
    lg.log_group_neuron_state(
        ERROR, "Tried to dealocate incoming message when message was NULL",
        this->get_group()->get_id(), this->get_id());
  }

  delete incoming_message;
  incoming_message = nullptr;
  return 1;
}

void Neuron::send_messages_in_group() {

  // loop through all neighbors
  for (const auto &pair : this->_postsynaptic) {

    lg.log_group_neuron_interaction(
        INFO, "Group %d: Neuron %d is sending a mesage to Group %d: Neuron %d",
        this->group->get_id(), this->id, pair.first->group->get_id(),
        pair.first->id);

    // construct message
    Message *message = new Message;
    message->target_neuron_group = pair.first->get_group();
    message->post_synaptic_neuron = pair.first;
    message->timestamp = lg.get_time_stamp();
    message->message_type = From_Neighbor;

    // calculate message
    pthread_mutex_lock(&potential_mutex);
    message->message = this->membrane_potential *
                       this->_postsynaptic[pair.first] *
                       this->excit_inhib_value;
    pthread_mutex_unlock(&potential_mutex);

    lg.log_group_neuron_value(
        DEBUG2, "Accumulated for Group %d: Neuron %d is %f",
        this->group->get_id(), this->id, this->membrane_potential);

    lg.log_group_neuron_interaction(
        DEBUG2, "Weight for Group %d: Neuron %d to Group %d: Neuron %d is %f",
        this->group->get_id(), this->id, pair.first->group->get_id(),
        pair.first->id, pair.second);

    lg.log_group_neuron_value(DEBUG2, "Group %d: Neuron %d modifier is %d",
                              this->group->get_id(), this->id,
                              this->excit_inhib_value);

    lg.log_group_neuron_interaction(
        INFO, "Message from  (%d) Neuron %d to (%d) Neuron %d is %f",
        this->group->get_id(), this->id, pair.first->group->get_id(),
        pair.first->id, message->message);

    // activate neighbor
    pair.first->activate();

    // add message to target
    message->post_synaptic_neuron->add_message(message);
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
    if (this->membrane_potential >= ACTIVATION_THRESHOLD) {
      this->send_messages_in_group();
    }
  }

  this->deactivate();
}

// Start the run cycle for a neuron
//
// Neuron waits on datamember cond. When activated, Neuron sends
// signal to all neighbors with a message based on weight, inhibitory status,
// and accumulated value. The data is passed to the recieving neuron via the
// global variable `value`.
//
void *Neuron::run() {

  pthread_mutex_lock(&potential_mutex);
  while (!active) {
    lg.log_neuron_state(INFO, "Neuron %d is waiting", this->id);
    pthread_cond_wait(&cond, &potential_mutex);
  }
  pthread_mutex_unlock(&potential_mutex);

  if (finish) {
    lg.log_neuron_state(INFO, "Neuron %d is exiting", this->id);
    pthread_exit(NULL);
  }

  pthread_mutex_lock(&potential_mutex);
  membrane_potential = ::value == INITIAL_MEMBRANE_POTENTIAL
                           ? membrane_potential
                           : membrane_potential + value;

  ::value = INITIAL_MEMBRANE_POTENTIAL;

  lg.log_neuron_value(INFO, "Neuron %d is activated, accumulated equal to %f",
                      this->id, this->membrane_potential);
  // lg.add_data(this->id, this->membrane_potential);
  recieved = true;

  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&potential_mutex);

  if (_postsynaptic.empty()) {
    lg.log_neuron_state(INFO, "Neuron %d does not have any neighbors",
                        this->id);
  } else if (membrane_potential < ACTIVATION_THRESHOLD) {
    lg.log_neuron_state(
        INFO,
        "Membrane potential for Neuron %d is below the threshold, not firing",
        this->id);
  } else {
    for (const auto &pair : _postsynaptic) {

      lg.log_neuron_interaction(INFO,
                                "Neuron %d is sending a mesage to Neuron %d",
                                this->id, pair.first->id);

      pthread_mutex_lock(&potential_mutex);

      double message =
          membrane_potential * _postsynaptic[pair.first] * excit_inhib_value;

      lg.log_neuron_value(DEBUG, "Accumulated for Neuron %d is %f", this->id,
                          this->membrane_potential);
      lg.log_neuron_interaction(DEBUG,
                                "Weight for Neuron %d to Neuron %d is %f",
                                this->id, pair.first->id, pair.second);
      lg.log_neuron_value(DEBUG, "Neuron %d modifier is %d", this->id,
                          this->excit_inhib_value);
      lg.log_neuron_interaction(INFO,
                                "Message from Neuron %d to Neuron %d is %f",
                                this->id, pair.first->id, message);

      ::value = message;

      // activate neighbor
      pair.first->activate();
      // get condition
      pthread_cond_t *neighbor_con = pair.first->get_cond();
      // signal start
      pthread_cond_signal(neighbor_con);
      pthread_mutex_unlock(&potential_mutex);

      pthread_mutex_lock(&potential_mutex);
      while (!pair.first->recieved) {
        pthread_cond_wait(neighbor_con, &potential_mutex);
      }
      pthread_mutex_unlock(&potential_mutex);
    }
    lg.log_neuron_state(INFO, "Neuron %d fired, entering refractory phase",
                        this->id);
    this->refractory();
    lg.log_neuron_state(INFO, "Neuron %d completed refractory phase, running",
                        this->id);
  }

  active = false;
  this->run();
  pthread_exit(NULL);
}

// Starts the thread for a neuron instance
//
void Neuron::start_thread() {
  pthread_create(&thread, NULL, Neuron::thread_helper, this);
}

// Not sure if this is the right way to do this actually.
//
void Neuron::join_thread() { pthread_join(thread, NULL); }

// Runs a refractory period for a Neuron
//
// Neuron sleeps for 2 milliseconds and potential is reset to
// value set by preprocessor directive REFRACTORY_MEMBRANE_POTENTIAL
void Neuron::refractory() {

  Message_t refractory_type = Refractory;

  this->refractory_start = lg.get_time_stamp();

  pthread_mutex_lock(&potential_mutex);
  this->membrane_potential = REFRACTORY_MEMBRANE_POTENTIAL;
  pthread_mutex_unlock(&potential_mutex);

  lg.log_neuron_value(INFO,
                      "Neuron %d in refractory state: potential set to %f",
                      this->id, this->membrane_potential);

  lg.add_data(this->get_group()->get_id(), this->get_id(),
              REFRACTORY_MEMBRANE_POTENTIAL, this->refractory_start,
              this->get_type(), refractory_type, this);
}

// Return presynaptic edges for a neuron
//
// Returns a constant pointer to the presynaptic weightmap
// for the calling instance
//
const weight_map *Neuron::get_presynaptic() const {
  const weight_map *p_presynaptic = &_presynaptic;
  return p_presynaptic;
}

// Return _postsynaptic edges for a neuron
//
// Returns a constant pointer to the _postsynaptic weightmap
// for the calling instance
const weight_map *Neuron::get_postsynaptic() const {
  const weight_map *p_postsynaptic = &_postsynaptic;
  return p_postsynaptic;
}
bool Neuron::is_activated() const { return this->active; }

NeuronGroup *Neuron::get_group() { return this->group; }

void Neuron::add_message(Message *message) {
  if (this->messages.empty()) {

    pthread_mutex_lock(&message_mutex);
    this->messages.push_back(message);
    pthread_mutex_unlock(&message_mutex);

  } else {

    pthread_mutex_lock(&message_mutex);
    list<Message *>::const_iterator it = this->messages.begin();

    while (it != this->messages.end() &&
           message->timestamp > (*it)->timestamp) {
      it++;
    }

    this->messages.insert(it, message);
    pthread_mutex_unlock(&message_mutex);
  }
}

Message *Neuron::get_message() {

  pthread_mutex_lock(&message_mutex);
  // Return if messages is empty
  if (this->messages.empty()) {
    pthread_mutex_unlock(&message_mutex);
    lg.log_group_neuron_state(DEBUG,
                              "No additional messages for (%d) Neuron %d",
                              this->get_group()->get_id(), this->get_id());
    return NULL;
  }
  pthread_mutex_unlock(&message_mutex);

  // Get least recent message and remove it from the queue
  pthread_mutex_lock(&message_mutex);
  Message *last = this->messages.front();
  this->messages.pop_front();
  pthread_mutex_unlock(&message_mutex);

  return last;
}

// Decays a neuron based on DECAY_VALUE retroactively
//
// Adds data points at even intervals from `from` to `to`
//
// Logs the updated membrane_potential
//
// @returns new membrane_potential
void Neuron::retroactive_decay(double from, double to, double tau,
                               double v_rest) {

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

    lg.add_data(this->get_group()->get_id(), this->get_id(),
                this->membrane_potential, i, this->get_type(),
                message_decay_type, this);
  }
  this->last_decay = i;
}

// Decays a neuron based on DECAY_VALUE
//
// Logs the updated membrane_potential
//
// @returns new membrane_potential
double Neuron::decay(double timestamp, double tau, double v_rest) {

  // for a membrane potential of  -55, tau = 10, decay value is 1.5
  double decay_value = (this->membrane_potential - v_rest) / tau;

  // if we sitting at -70 no need to decay
  if (abs(decay_value) < 0.01 || decay_value < 0) {
    return 0;
  }

  Message_t decay_type = Decay;

  this->update_potential(-decay_value);

  double potential = this->membrane_potential;

  lg.add_data(this->get_group()->get_id(), this->get_id(), potential, timestamp,
              this->get_type(), decay_type, this);

  lg.log_group_neuron_value(
      DEBUG2, "(%d) Neuron %d is decaying. Decay value is %f",
      this->get_group()->get_id(), this->get_id(), decay_value);

  lg.log_group_neuron_value(
      DEBUG2, "(%d) Neuron %d decayed. Membrane potential now %f",
      this->get_group()->get_id(), this->get_id(), potential);

  return potential;
}

// Activates neuron
void Neuron::activate() {
  pthread_mutex_lock(&activation_mutex);
  this->active = true;
  pthread_mutex_unlock(&activation_mutex);
}

// deactivates neuron
void Neuron::deactivate() {
  pthread_mutex_lock(&activation_mutex);
  this->active = false;
  pthread_mutex_unlock(&activation_mutex);
}

// Set type of neuron
//
// @param1: Neuron_t
void Neuron::set_type(Neuron_t type) { this->type = type; }

void Neuron::update_potential(double value) {
  pthread_mutex_lock(&potential_mutex);
  this->membrane_potential += value;
  pthread_mutex_unlock(&potential_mutex);
}
const list<Message *> &Neuron::get_message_vector() { return this->messages; }
