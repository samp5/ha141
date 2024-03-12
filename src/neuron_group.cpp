#include "functions.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include <pthread.h>
#include <unistd.h>

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

NeuronGroup::~NeuronGroup() {
  for (auto neuron : this->neurons) {

    lg.log_group_neuron_state(DEBUG, "Deleteing Group %d Neuron %d", this->id,
                              neuron->get_id());

    delete neuron;
  }
}

void *NeuronGroup::group_run() {
  lg.log_group_state(INFO, "Group %d running", this->get_id());

  int i = 0;
  while (i < 3) {
    for (Neuron *neuron : this->neurons) {
      lg.log_group_neuron_type(
          DEBUG2, "Checking activation:(%d) Neuron %d is %s", this->get_id(),
          neuron->get_id(), get_active_status_string(neuron->is_activated()));
      if (neuron->is_activated()) {
        lg.log_group_neuron_state(DEBUG2, "Running (%d) Neuron (%d)",
                                  this->get_id(), neuron->get_id());
        neuron->run_in_group();
      }
    }
    this->process_intragroup_queue();
    this->process_intergroup_queue();
    i++;
  }
  pthread_exit(NULL);
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
    message->target_neuron->add_message(message);
  }
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
    message->target_neuron->add_message(message);
  }
}
void NeuronGroup::add_to_intragroup(Message *message) {
  pthread_mutex_lock(&mutex);
  this->intragroup_messages.push_back(message);
  pthread_mutex_unlock(&mutex);
}
void NeuronGroup::add_to_intergroup(Message *message) {
  pthread_mutex_lock(&mutex);
  this->intergroup_messages.push_back(message);
  pthread_mutex_unlock(&mutex);
}
