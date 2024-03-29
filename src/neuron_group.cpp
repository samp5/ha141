#include "functions.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include <pthread.h>
#include <unistd.h>

// Constructor
NeuronGroup::NeuronGroup(int _id, int number_neurons) {
  lg.log_group_state(DEBUG, "Adding Group %d", _id);

  this->id = _id;

  // add neurons;
  lg.log_group_state(INFO, "Group %d", this->id);

  for (int i = 0; i < number_neurons; i++) {
    Neuron *neuron = new Neuron(i + 1, get_inhibitory_status(), this);
    this->neurons.push_back(neuron);
  }
}

// Destructor
NeuronGroup::~NeuronGroup() {
  for (auto neuron : this->neurons) {

    lg.log_group_neuron_state(DEBUG, "Deleteing Group %d Neuron %d", this->id,
                              neuron->get_id());

    delete neuron;
  }
}

// Run group
//
// runs through all neurons and checks their activation status
// every `WAIT_LOOPS` * `WAIT_TIME`
//
void *NeuronGroup::group_run() {

  // Log running status
  lg.log_group_state(INFO, "Group %d running", this->get_id());

  // While the network is running...
  while (::active) {

    for (Neuron *neuron : this->neurons) {

      lg.log_group_neuron_type(
          DEBUG2, "Checking activation:(%d) Neuron %d is %s", this->get_id(),
          neuron->get_id(), get_active_status_string(neuron->is_activated()));

      if (neuron->is_activated()) {

        lg.log_group_neuron_state(DEBUG2, "Running (%d) Neuron (%d)",
                                  this->get_id(), neuron->get_id());

        // Run neuron
        neuron->run_in_group();
      }
    }

    lg.log_group_state(DEBUG2, "Group %d pausing", this->id);

    for (int i = 1; i <= WAIT_LOOPS; i++) {
      lg.log_group_value(DEBUG3, "Group %d waiting: %d", this->get_id(), i);
      usleep(WAIT_TIME);
    }

    lg.log_group_state(DEBUG2, "Group %d resuming", this->id);
  }
  return NULL;
}

int NeuronGroup::neuron_count() { return (int)this->neurons.size(); }

const vector<Neuron *> &NeuronGroup::get_neruon_vector() {
  return this->neurons;
}

void NeuronGroup::print_group() {
  lg.print("\n", false);
  lg.log_group_value(DEBUG, "Neuron Group %d (%d neurons)", this->id,
                     this->neurons.size());
  lg.print("========================================================");

  for (Neuron *neuron : this->neurons) {
    lg.log_group_neuron_state(DEBUG, "   (%d) Neuron %d", this->id,
                              neuron->get_id());
    print_group_maps(neuron);
  }
  lg.print("\n", false);
}

// each of these proceses should maybe be spawned in their own threads ?
void NeuronGroup::process_intragroup_queue() {

  for (Message *message : this->intragroup_messages) {
    // mutex lock happens in the neuron add_message function
    message->post_synaptic_neuron->add_message(message);
  }
  this->intragroup_messages.clear();
}

void NeuronGroup::process_intergroup_queue() {
  for (Message *message : this->intergroup_messages) {

    if (message->target_neuron_group != this) {
      lg.log_group_neuron_state(ERROR,
                                "process_intergroup_queue: Message meant for "
                                "Group %d in Group %d intergroup_messages",
                                this->get_id(),
                                message->target_neuron_group->get_id());
    }
    message->post_synaptic_neuron->add_message(message);
  }
  this->intergroup_messages.clear();
}
void NeuronGroup::add_to_intragroup(Message *message) {
  pthread_mutex_lock(&potential_mutex);
  this->intragroup_messages.push_back(message);
  pthread_mutex_unlock(&potential_mutex);
}
void NeuronGroup::add_to_intergroup(Message *message) {
  pthread_mutex_lock(&potential_mutex);
  this->intergroup_messages.push_back(message);
  pthread_mutex_unlock(&potential_mutex);
}
