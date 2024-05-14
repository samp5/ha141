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

      Neuron *neuron = new Neuron(id, this, Neuron_t::None);
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

// Destructor
NeuronGroup::~NeuronGroup() {
  for (auto neuron : this->neurons) {

    lg.log_group_neuron_state(DEBUG, "Deleteing Group %d Neuron %d", this->id,
                              neuron->getID());

    delete neuron;
  }
}

// Run group
//
// runs through all neurons and checks their activation status
void *NeuronGroup::run() {

  // Log running status
  lg.log_group_state(INFO, "Group %d running", this->getID());

  // While the network is running...
  while (::active) {

    for (Neuron *neuron : this->neurons) {
      if (!::active) {
        break;
      }

      lg.log_group_neuron_type(
          DEBUG4, "Checking activation:(%d) Neuron %d is %s", this->getID(),
          neuron->getID(), lg.get_active_status_string(neuron->isActivated()));

      if (neuron->isActivated()) {

        lg.log_group_neuron_state(DEBUG2, "Running (%d) Neuron (%d)",
                                  this->getID(), neuron->getID());
        neuron = neuron->getType() == Input
                     ? dynamic_cast<InputNeuron *>(neuron)
                     : neuron;
        neuron->run();
      } else {
        double time = lg.get_time_stamp();
        neuron->retroactiveDecay(neuron->getLastDecay(), time);
      }
    }
  }

  for (auto neuron : this->neurons) {
    neuron->transferData();
  }

  return NULL;
}

int NeuronGroup::neuronCount() { return (int)this->neurons.size(); }

const vector<Neuron *> &NeuronGroup::getNeuronVec() { return this->neurons; }

void NeuronGroup::reset() {
  for (auto neuron : this->neurons) {
    if (neuron->getType() == Input) {
      neuron = dynamic_cast<InputNeuron *>(neuron);
    }
    neuron->reset();
  }
}
