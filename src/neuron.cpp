#include "neuron.hpp"
#include "functions.hpp"
#include "log.hpp"

#include <pthread.h>
#include <unistd.h>

// Constructor for Neuron class
//
// Sets ID, inhibitory status, and prints out a log message
//
// @param1: Neuron ID
// @param2: excitatory/inhibitory value (1 or -1)
Neuron::Neuron(int _id, int _excit_inhib_value) {
  id = _id;
  excit_inhib_value = _excit_inhib_value;

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

  lg.log_neuron_interaction(INFO, "Edge from Neuron %d to Neuron %d added.", id,
                            neighbor->get_id());

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
      DEBUG, "Neuron %d added to the _presynaptic map of Neuron ",
      neighbor->get_id(), this->id);
}

void Neuron::run_in_group() {
  if (!this->active) {
    lg.log_neuron_state(
        WARNING, "run_in_group: Neuron %d tried to run when it was not active",
        this->id);
    return;
  }

  pthread_mutex_lock(&mutex);
  this->membrane_potential =
      this->group->get_message() == INITIAL_MEMBRANE_POTENTIAL
          ? this->membrane_potential
          : this->membrane_potential + this->group->get_message();

  this->group->set_message(INITIAL_MEMBRANE_POTENTIAL);

  lg.log_group_neuron_value(
      INFO, "Neuron %d is activated, accumulated equal to %f",
      this->group->get_id(), this->id, this->membrane_potential);

  lg.add_data(this->id, this->membrane_potential);

  recieved = true;

  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mutex);

  if (this->_postsynaptic.empty()) {
    lg.log_group_neuron_state(INFO,
                              "Group %d: Neuron %d does not have any neighbors",
                              this->group->get_id(), this->id);
  } else if (membrane_potential < ACTIVATION_THRESHOLD) {
    lg.log_group_neuron_state(
        INFO,
        "Membrane potential for Neuron %d is below the threshold, not firing",
        this->group->get_id(), this->id);
  } else {
    for (const auto &pair : _postsynaptic) {

      lg.log_neuron_interaction(INFO,
                                "Neuron %d is sending a mesage to Neuron %d",
                                this->id, pair.first->id);

      pthread_mutex_lock(&mutex);

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
      pthread_mutex_unlock(&mutex);

      pthread_mutex_lock(&mutex);
      while (!pair.first->recieved) {
        pthread_cond_wait(neighbor_con, &mutex);
      }
      pthread_mutex_unlock(&mutex);
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
// Start the run cycle for a neuron
//
// Neuron waits on datamember cond. When activated, Neuron sends
// signal to all neighbors with a message based on weight, inhibitory status,
// and accumulated value. The data is passed to the recieving neuron via the
// global variable `value`.
//
void *Neuron::run() {

  pthread_mutex_lock(&mutex);
  while (!active) {
    lg.log_neuron_state(INFO, "Neuron %d is waiting", this->id);
    pthread_cond_wait(&cond, &mutex);
  }

  pthread_mutex_unlock(&mutex);
  if (finish) {
    lg.log_neuron_state(INFO, "Neuron %d is exiting", this->id);
    pthread_exit(NULL);
  }

  pthread_mutex_lock(&mutex);
  membrane_potential = ::value == INITIAL_MEMBRANE_POTENTIAL
                           ? membrane_potential
                           : membrane_potential + value;

  ::value = INITIAL_MEMBRANE_POTENTIAL;

  lg.log_neuron_value(INFO, "Neuron %d is activated, accumulated equal to %f",
                      this->id, this->membrane_potential);
  lg.add_data(this->id, this->membrane_potential);
  recieved = true;

  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mutex);

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

      pthread_mutex_lock(&mutex);

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
      pthread_mutex_unlock(&mutex);

      pthread_mutex_lock(&mutex);
      while (!pair.first->recieved) {
        pthread_cond_wait(neighbor_con, &mutex);
      }
      pthread_mutex_unlock(&mutex);
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
//
const weight_map *Neuron::get_postsynaptic() const {
  const weight_map *p_postsynaptic = &_postsynaptic;
  return p_postsynaptic;
}
bool Neuron::is_activated() const { return this->active; }
