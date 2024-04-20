#include "functions.hpp"
#include "input_neuron.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>

// Constructor
NeuronGroup::NeuronGroup(int _id, int number_neurons,
                         int number_input_neurons) {
  lg.log_group_state(DEBUG, "Adding Group %d", _id);

  this->id = _id;

  lg.log_group_state(INFO, "Group %d", this->id);

  // we only need this many of "regular neurons"
  number_neurons -= number_input_neurons;

  int id = 1;

  while (number_neurons || number_input_neurons) {

    // 1 is regular, 0 is input
    int roll = rand() % 2;

    if (roll && number_neurons) {

      Neuron *neuron = new Neuron(id, get_inhibitory_status(), this);
      this->neurons.push_back(neuron);
      number_neurons--;
      id++;

    } else if (!roll && number_input_neurons) {

      InputNeuron *neuron = new InputNeuron(id, this);
      this->neurons.push_back(neuron);
      number_input_neurons--;
      id++;
    }
  }
}

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
void *NeuronGroup::group_run() {

  // Log running status
  lg.log_group_state(INFO, "Group %d running", this->get_id());

  // While the network is running...
  while (::active) {

    for (Neuron *neuron : this->neurons) {

      lg.log_group_neuron_type(
          DEBUG4, "Checking activation:(%d) Neuron %d is %s", this->get_id(),
          neuron->get_id(), get_active_status_string(neuron->is_activated()));

      if (neuron->is_activated()) {

        lg.log_group_neuron_state(DEBUG2, "Running (%d) Neuron (%d)",
                                  this->get_id(), neuron->get_id());
        neuron = neuron->get_type() == Input
                     ? dynamic_cast<InputNeuron *>(neuron)
                     : neuron;
        neuron->run_in_group();
      } else {
        double time = lg.get_time_stamp();
        neuron->retroactive_decay(neuron->get_last_decay(), time);
      }
    }
  }

  for (auto neuron : this->neurons) {
    neuron->transfer_data();
  }

  return NULL;
}

int NeuronGroup::neuron_count() { return (int)this->neurons.size(); }

const vector<Neuron *> &NeuronGroup::get_neruon_vector() {
  return this->neurons;
}

void NeuronGroup::print_group() {
  cout << "NeuronGroup::print_group() not implemented\n";
}

void NeuronGroup::reset() {
  for (auto neuron : this->neurons) {
    if (neuron->get_type() == Input) {
      neuron = dynamic_cast<InputNeuron *>(neuron);
    }
    neuron->reset();
  }
}
