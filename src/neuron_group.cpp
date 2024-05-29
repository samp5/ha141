#include "input_neuron.hpp"
#include "log.hpp"
#include "network.hpp"
#include "neuron.hpp"
#include "runtime.hpp"
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>

/**
 * @brief NeuronGroup constructor.
 *
 * Construct a NeuronGroup. Allocates all `Neuron`s and `InputNeuron`s.
 *
 * @param _id NeuronGroup ID
 */
NeuronGroup::NeuronGroup(int _id, int number_neurons, int number_input_neurons,
                         SNN *network)
    : network(network) {
  getNetwork()->lg->state(DEBUG, "Adding Group %d", _id);

  id = _id;

  getNetwork()->lg->state(INFO, "Group %d", id);

  // we only need this many of "regular neurons"
  number_neurons -= number_input_neurons;

  int id = 1;

  while (number_neurons || number_input_neurons) {

    // 1 is regular, 0 is input
    int roll = rand() % 2;

    if (roll && number_neurons) {

      Neuron *neuron = new Neuron(id, this, Neuron_t::None);
      neurons.push_back(neuron);
      number_neurons--;
      id++;

    } else if (!roll && number_input_neurons) {
      InputNeuron *neuron = new InputNeuron(id, this, 0.0);
      neurons.push_back(neuron);
      number_input_neurons--;
      id++;
    }
  }
}

/**
 * @brief Destructs NeuronGroup.
 *
 * NeuronGroup holds memory responsibility for `Neuron`s and deallocates them in
 * its destructor
 *
 */
NeuronGroup::~NeuronGroup() {
  for (auto neuron : neurons) {

    getNetwork()->lg->groupNeuronState(DEBUG, "Deleteing Group %d Neuron %d",
                                       id, neuron->getID());

    if (neuron) {
      delete neuron;
      neuron = nullptr;
    }
  }
}

/**
 * @brief Main run cycle for a NeuronGroup.
 *
 * Checks the global bool ::active each cycle. Runs active neurons.
 * Before joining the main thread, transfers data from Neuron::log_data to
 * Log::log_data
 *
 */
void *NeuronGroup::run() {

  // Log running status
  getNetwork()->lg->state(INFO, "Group %d running", getID());

  // While the network is running...
  while (getNetwork()->isActive()) {

    for (Neuron *neuron : neurons) {
      if (!getNetwork()->isActive()) {
        break;
      }

      if (getNetwork()->switchingStimulus()) {
        pthread_barrier_wait(&(network->getBarrier()->barrier));

        pthread_mutex_lock(&getNetwork()->getMutex()->stimulus);
        while (getNetwork()->switchingStimulus()) {
          pthread_cond_wait(getNetwork()->switchCond(),
                            &getNetwork()->getMutex()->stimulus);
        }
        pthread_mutex_unlock(&getNetwork()->getMutex()->stimulus);
      }

      getNetwork()->lg->neuronType(
          DEBUG4, "Checking activation:(%d) Neuron %d is %s", getID(),
          neuron->getID(),
          getNetwork()->lg->activeStatusString(neuron->isActivated()));

      if (neuron->isActivated()) {

        getNetwork()->lg->groupNeuronState(DEBUG2, "Running (%d) Neuron (%d)",
                                           getID(), neuron->getID());
        neuron = neuron->getType() == Input
                     ? dynamic_cast<InputNeuron *>(neuron)
                     : neuron;
        neuron->run();
      } else {
        double time = getNetwork()->lg->time();
        neuron->retroactiveDecay(neuron->getLastDecay(), time);
      }
    }
  }

  for (auto neuron : neurons) {
    neuron->transferData();
  }

  return NULL;
}

/**
 * @brief Get Neuron count of the NeuronGroup.
 *
 * @return Neuron count
 */
int NeuronGroup::neuronCount() const { return (int)neurons.size(); }

/**
 * @brief Get a mutable reference to the Neuron vector.
 *
 */
const vector<Neuron *> &NeuronGroup::getMutNeuronVec() { return neurons; }

/**
 * @brief Reset NeuronGroup.
 *
 * Resets all `Neuron`s
 *
 */
void NeuronGroup::reset() {
  for (auto neuron : neurons) {
    if (neuron->getType() == Input) {
      neuron = dynamic_cast<InputNeuron *>(neuron);
    }
    neuron->reset();
  }
}
