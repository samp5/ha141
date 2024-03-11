#include "functions.hpp"
#include "log.hpp"
#include "neuron.hpp"

NeuronGroup::NeuronGroup(int _id, int number_neurons) {

  lg.log_group_state(DEBUG, "Adding Group %d", _id);

  this->id = _id;

  // add neurons;
  lg.log_group_state(INFO, "Group %d", this->id);
  for (int i = 0; i < number_neurons; i++) {
    Neuron *neuron = new Neuron(i + 1, get_inhibitory_status());
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
  for (Neuron *neuron : this->neurons) {
    if (neuron->is_activated()) {
      neuron->run_in_group();
    }
  }
  return NULL;
}

void NeuronGroup::set_message(double message) { this->message = message; }

void NeuronGroup::print_group() {
  lg.log_group_state(DEBUG, "Neuron Group %d", this->id);
  lg.print("==============");

  for (Neuron *neuron : this->neurons) {
    lg.log_neuron_state(DEBUG, "    Neuron %d", neuron->get_id());
  }
}
