#include "input_neuron.hpp"
#include "message.hpp"
#include "network.hpp"
#include "neuron.hpp"
#include "runtime.hpp"
#include <pthread.h>

/**
 * @brief Constructor for InputNeuron.
 *
 * Sets status as excitatory by default. Probability of success
 * is set in RuntimConfig
 *
 * @param _id Neuron ID
 * @param group NeuronGroup that this neuron belongs to
 */
InputNeuron::InputNeuron(int _id, NeuronGroup *group, double latency)
    : Neuron(_id, group, Input),
      probalility_of_success(
          group->getNetwork()->getConfig()->INPUT_PROB_SUCCESS),
      latency(latency) {
  this->excit_inhib_value = -1;
  this->activate();
}

/**
 * @brief Main run sequence for an InputNeuron.
 *
 * - Waits on stimulus switch halting thread.
 * - Ignores message during refractory period.
 * - Updates potential based on InputNeuron::input_value on
 * - Poisson success
 * - Sends messages through all synapses
 */
void InputNeuron::run() {

  // Check for refractory period
  if (this->inRefractory()) {
    group->getNetwork()->lg->groupNeuronState(
        INFO,
        "INPUT: (%d) Neuron %d is still in refractory period, ignoring input",
        this->getGroup()->getID(), this->getID());
    return;
  }

  // Grab the time and decay
  double time = group->getNetwork()->lg->time();
  this->retroactiveDecay(this->last_decay, time);

  // Check to see if we need to send messages
  if (this->membrane_potential >=
      group->getNetwork()->getConfig()->ACTIVATION_THRESHOLD) {
    this->sendMessages();
  }

  // Check latency
  if (time < group->getNetwork()->getStimulusStart() + this->latency) {
    return;
  }

  // Add stimulus for poisson succeses
  if (poissonResult()) {

    double time = group->getNetwork()->lg->time();

    this->accumulatePotential(this->input_value);
    this->addData(time, Message_t::Stimulus);

    group->getNetwork()->lg->neuronValue(
        INFO,
        "(Input) (%d) Neuron %d poisson success! Adding input value to "
        "membrane_potential. membrane_potential now %f",
        this->getGroup()->getID(), this->getID(), this->membrane_potential);
  }

  // Check to see if we need to send messages
  if (this->membrane_potential >=
      group->getNetwork()->getConfig()->ACTIVATION_THRESHOLD) {
    this->sendMessages();
  }
}

/**
 * @brief Calculates Poisson result.
 *
 * Based on InputNeuron::probability.
 * This value is set in RuntimConfig
 *
 * @return `true` for success `false` for failure
 */
bool InputNeuron::poissonResult() {

  double roll = (double)rand() / RAND_MAX;

  if (roll <= this->probalility_of_success) {
    return true;
  } else {
    return false;
  }
}

/**
 * @brief Checks refractory status of neuron.
 *
 *
 *
 * @return `true` for in refractory, `false` otherwise
 */
bool InputNeuron::inRefractory() {

  double timestamp = group->getNetwork()->lg->time();

  // #askpedram

  // first check refractory status
  if (timestamp < this->refractory_start +
                      group->getNetwork()->getConfig()->REFRACTORY_DURATION) {

    group->getNetwork()->lg->groupNeuronState(
        INFO, "(%d) Neuron %d is still in refractory period, ignoring message",
        this->getGroup()->getID(), this->getID());

    return true;
  }

  return false;
}

/**
 * @brief Sends messages to all connections.
 *
 * Propagates messages across all synapses then enters a
 * refractory phase
 *
 */
void InputNeuron::sendMessages() {

  for (const auto synapse : getSynapses()) {
    synapse->propagate();
  }

  group->getNetwork()->lg->groupNeuronState(
      INFO,
      "(%d) Neuron %d reached activation threshold, entering refractory phase",
      this->group->getID(), this->id);

  this->refractory();
}
/**
 * @brief Sets input value (stimulus).
 *
 * Set the input value of the neuron.
 * This value is added to the membrane potential of
 * the neuron on each Poisson success
 *
 * @param value The new value of the InputNeuron
 */
void InputNeuron::setInputValue(double value) {
  this->input_value = value;
  group->getNetwork()->lg->neuronValue(
      DEBUG3, "(Input) (%d) Neuron %d input value set to %f",
      this->getGroup()->getID(), this->getID(), value);
}

void InputNeuron::setLatency(double latency) { this->latency = latency; }

/**
 * @brief Resets the InputNeuron.
 *
 * Set membrane_potential to INITIAL_MEMBRANE_POTENTIAL
 * Set last_decay to now
 * Resets refractory_start
 *
 */
void InputNeuron::reset() {
  pthread_mutex_lock(&group->getNetwork()->getMutex()->potential);
  this->membrane_potential =
      group->getNetwork()->getConfig()->INITIAL_MEMBRANE_POTENTIAL;
  pthread_mutex_unlock(&group->getNetwork()->getMutex()->potential);
  this->last_decay = group->getNetwork()->lg->time();
  this->refractory_start = 0;
}
