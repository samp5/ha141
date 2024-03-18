#include "neuron.hpp"
#include "functions.hpp"
#include "log.hpp"

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

  this->type = None;
  this->id = _id;
  this->excit_inhib_value = inhibitory;
  this->group = group;
  this->membrane_potential = INITIAL_MEMBRANE_POTENTIAL;
  this->type = type;

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

  const char *inhib =
      _excit_inhib_value == -1 ? "excitatory\0" : "inhibitory\0";

  lg.log_neuron_type(INFO, "Neuron %d added: %s", _id, inhib);
}

// Destructor for the Neuron class
//
// Destroys pthread conditional
//
Neuron::~Neuron() { pthread_cond_destroy(&cond); }

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
        WARNING,
        "run_in_group: Group %d: Neuron %d tried to run when it was not active",
        this->group->get_id(), this->id);
    return 0;
  }

  // Get message
  Message *incoming_message = this->get_message();

  // Check message validity
  if (incoming_message == NULL) {
    return 0;
  }

  // Lock Mutex
  pthread_mutex_lock(&potential_mutex);
  // Update membrane_potential
  this->membrane_potential =
      this->membrane_potential + incoming_message->message;
  pthread_mutex_unlock(&potential_mutex);

  lg.log_group_neuron_value(
      INFO, "(%d) Neuron %d is activated, accumulated equal to %f",
      this->group->get_id(), this->id, this->membrane_potential);

  // use message timestamp not current time
  // #askpedram
  lg.add_data(this->group->get_id(), this->id, this->membrane_potential,
              incoming_message->timestamp, this->get_type());

  // Deallocate this message
  delete incoming_message;
  return 1;
}

// Checks if a neuron is able to run after recieving
// all its messages
//
// checks neighbors and activation threshold
//
// @returns: 1 if okay to run, 0 if not
int Neuron::check_run_conditions() {

  // Check for neighbors
  if (this->_postsynaptic.empty()) {
    lg.log_group_neuron_state(INFO,
                              "Group %d: Neuron %d does not have any neighbors",
                              this->group->get_id(), this->id);
    this->deactivate();
    return 0;
  }

  // Check for activation threshold
  if (membrane_potential < ACTIVATION_THRESHOLD) {

    lg.log_group_neuron_state(INFO,
                              "Membrane potential for Group %d: Neuron %d is "
                              "below the threshold, not firing",
                              this->group->get_id(), this->id);
    this->deactivate();
    return 0;
  }
  return 1;
}

// Run cycle for a neuron in a group
void Neuron::run_in_group() {

  // Get all the messsages for this neuron
  while (this->recieve_in_group()) {
  }

  // Check run conditions
  if (!this->check_run_conditions()) {
    return;
  }

  // loop through all neighbors
  for (const auto &pair : _postsynaptic) {

    lg.log_group_neuron_interaction(
        INFO, "Group %d: Neuron %d is sending a mesage to Group %d: Neuron %d",
        this->group->get_id(), this->id, pair.first->group->get_id(),
        pair.first->id);

    // construct message
    Message *message = new Message;
    message->target_neuron_group = pair.first->get_group();
    message->target_neuron = pair.first;
    message->timestamp = lg.get_time_stamp();

    // calculate message
    pthread_mutex_lock(&potential_mutex);
    message->message =
        membrane_potential * _postsynaptic[pair.first] * excit_inhib_value;
    pthread_mutex_unlock(&potential_mutex);

    lg.log_group_neuron_value(
        DEBUG, "Accumulated for Group %d: Neuron %d is %f",
        this->group->get_id(), this->id, this->membrane_potential);

    lg.log_group_neuron_interaction(
        DEBUG, "Weight for Group %d: Neuron %d to Group %d: Neuron %d is %f",
        this->group->get_id(), this->id, pair.first->group->get_id(),
        pair.first->id, pair.second);

    lg.log_group_neuron_value(DEBUG, "Group %d: Neuron %d modifier is %d",
                              this->group->get_id(), this->id,
                              this->excit_inhib_value);

    lg.log_group_neuron_interaction(
        INFO, "Message from Group %d: Neuron %d to Group %d: Neuron %d is %f",
        this->group->get_id(), this->id, pair.first->group->get_id(),
        pair.first->id, message->message);

    // activate neighbor
    pair.first->activate();

    // Check if it is intragroup
    if (this->group->get_id() == pair.first->group->get_id()) {

      // add message to target
      message->target_neuron->add_message(message);

    } else {

      // add message to neuron
      message->target_neuron->add_message(message);
    }
  }

  lg.log_group_neuron_state(INFO,
                            "(%d) Neuron %d fired, entering refractory phase",
                            this->group->get_id(), this->id);

  this->refractory();

  lg.log_group_neuron_state(
      INFO, "(%d) Neuron %d completed refractory phase, running",
      this->group->get_id(), this->id);

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
  lg.add_data(this->id, this->membrane_potential);
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
  membrane_potential = REFRACTORY_MEMBRANE_POTENTIAL;
  lg.log_neuron_value(INFO, "Neuron %d portential set to %f", this->id,
                      this->membrane_potential);
  usleep(2000);
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
  pthread_mutex_lock(&message_mutex);
  this->messages.push_back(message);
  pthread_mutex_unlock(&message_mutex);
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

  // Get least recent message
  pthread_mutex_lock(&message_mutex);
  Message *last = this->messages.front();
  pthread_mutex_unlock(&message_mutex);

  // Remove it from the queue
  pthread_mutex_lock(&message_mutex);
  this->messages.pop_front();
  pthread_mutex_unlock(&message_mutex);

  return last;
}

// Decays a neuron based on DECAY_VALUE
//
// Logs the updated membrane_potential
//
// @returns new membrane_potential
double Neuron::decay() {
  pthread_mutex_lock(&potential_mutex);
  this->membrane_potential -= DECAY_VALUE;

  lg.add_data(this->get_group()->get_id(), this->get_id(),
              this->membrane_potential);

  pthread_mutex_unlock(&potential_mutex);

  lg.log_group_neuron_value(
      DEBUG2, "(%d) Neuron %d decaying. Membrane potential now %f",
      this->get_group()->get_id(), this->get_id(), this->get_potential());
  return this->membrane_potential;
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
