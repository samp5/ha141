#include "functions.hpp"
#include "neuron.hpp"

NeuronGroup::NeuronGroup(int _id, int number_neurons) {
  this->id = _id;

  // make space for number of neurons
  this->neurons.reserve(number_neurons);

  // add neurons;
  for (int i = 0; i < number_neurons; i++) {
    Neuron *neuron = new Neuron(i + 1, get_inhibitory_status());
    neurons[i] = neuron;
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
