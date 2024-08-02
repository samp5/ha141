#include "input_neuron.hpp"
#include "message.hpp"
#include "network.hpp"
#include "neuron.hpp"
#include "runtime.hpp"
#include <pthread.h>
#include <random>

/**
 * @brief Constructor for InputNeuron.
 *
 * Sets status as excitatory by default. Probability of success
 * is set in RuntimConfig
 *
 * @param _id Neuron ID
 * @param group NeuronGroup that this neuron belongs to
 */
InputNeuron::InputNeuron(int _id, NeuronGroup *group, int latency)
    : Neuron(_id, group, Input),
      probalility_of_success(
          group->getNetwork()->getConfig()->INPUT_PROB_SUCCESS),
      latency(latency) {
  excit_inhib_value = 1;
  const char *inhib = excit_inhib_value == 1 ? "excitatory\0" : "inhibitory\0";

  group->getNetwork()->lg->neuronType(DEBUG, "INPUT (%d) Neuron %d added: %s",
                                      group->getID(), _id, inhib);

  activate();
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
void InputNeuron::run(Message *message) {

  if (message->timestamp < refractory_start + refractory_duration) {
    // group->getNetwork()->lg->groupNeuronState(
    //     DEBUG,
    //     "INPUT: (%d) Neuron %d is still in refractory period, ignoring
    //     input", getGroup()->getID(), getID());
    return;
  }

  // Grab the time and decay
  retroactiveDecay(last_decay, message->timestamp);

  accumulatePotential(message->message);

  // Check to see if we need to send messages
  if (membrane_potential >= activationThreshold) {
    last_fire = message->timestamp;
    sendMessages();
  }

  if (message) {
    delete message;
    message = nullptr;
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
bool InputNeuron::poissonResult() const {

  double roll = (double)rand() / RAND_MAX;

  if (roll <= probalility_of_success) {
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
bool InputNeuron::inRefractory() const {

  double timestamp = group->getNetwork()->lg->time();

  // #askpedram

  // first check refractory status
  if (timestamp < refractory_start +
                      group->getNetwork()->getConfig()->REFRACTORY_DURATION) {

    group->getNetwork()->lg->groupNeuronState(
        DEBUG, "(%d) Neuron %d is still in refractory period, ignoring message",
        getGroup()->getID(), getID());

    return true;
  }

  return false;
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
void InputNeuron::setInputValue(long double value) {
  input_value = value;
  group->getNetwork()->lg->neuronValue(
      DEBUG3, "(Input) (%d) Neuron %d input value set to %lf",
      getGroup()->getID(), getID(), value);
}

void InputNeuron::setLatency(int _l) { latency = _l; }

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
  membrane_potential =
      group->getNetwork()->getConfig()->INITIAL_MEMBRANE_POTENTIAL;
  pthread_mutex_unlock(&group->getNetwork()->getMutex()->potential);
  last_decay = group->getNetwork()->lg->time();
  refractory_start = -INT_MAX;
}

/**
 * @brief Generate Neuron Events.
 *
 * Generate neuron events from a vector of success timestamps
 * passed from the calling SNN::generateInputNeuoneEvents
 *
 * @param timestamps vector of timestamps
 */
void InputNeuron::generateEvents(const std::vector<int> &timestamps) {
  for (auto i : timestamps) {

    if (i < latency) {
      continue;
    }

    Message *message =
        new Message(input_value, nullptr, this, Message_t::Stimulus, i);
    group->addToMessageQ(message);
  }
}

/**
 * @brief Generate Neuron Events bad way.
 *
 */
void InputNeuron::generateEvents() {
  SNN *network = group->getNetwork();

  static std::poisson_distribution<> d(
      probalility_of_success *
      group->getNetwork()->getConfig()->time_per_stimulus);

  int number_events = d(network->getGen());
  int created_events = 0;

  while (created_events < number_events) {
    int timestamp =
        network->getRandom() % (network->getConfig()->time_per_stimulus + 1);

    if (timestamp < latency) {
      created_events++;
      continue;
    }

    Message *message =
        new Message(input_value, nullptr, this, Message_t::Stimulus, timestamp);
    group->addToMessageQ(message);
    created_events++;
  }
}
