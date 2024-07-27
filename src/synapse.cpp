#include "synapse.hpp"
#include "log.hpp"
#include "message.hpp"
#include "network.hpp"
#include "neuron.hpp"
#include "runtime.hpp"
#include <cstdlib>

Synapse::Synapse(Neuron *from, Neuron *to, double w, double delay)
    : _origin(from), _destination(to),
      network(_origin->getGroup()->getNetwork()),
      _weight(w == -1 ? randomWeight() : w),
      delay(delay == -1 ? randomDelay() : delay){};

/**
 * @brief Propagates a message.
 *
 * Activates the recieving neuron and calls Neuron::addMessage
 *
 */
void Synapse::propagate() {
  static RuntimConfig *config = _origin->getGroup()->getNetwork()->getConfig();

  if (_origin->getLastFire() + delay > config->time_per_stimulus) {
    return;
  }

  // int preGroupID = _origin->getGroup()->getID();
  // int postGroupID = _destination->getGroup()->getID();
  // int preID = _origin->getID();
  // int postID = _destination->getID();
  //
  double message_value =
      _origin->getPotential() * getWeight() * _origin->getBias();

  // _origin->getGroup()->getNetwork()->lg->neuronInteraction(
  //     DEBUG, "Group %d: Neuron %d is sending a mesage to Group %d: Neuron
  //     %d", preGroupID, preID, postGroupID, postID);

  Message *message_to_send =
      new Message(message_value, _origin, _destination, From_Neighbor,
                  _origin->getLastFire() + delay);

  _destination->activate();
  _destination->getGroup()->addToMessageQ(message_to_send);
}

int Synapse::randomDelay() {
  int delay = rand() % network->getConfig()->max_synapse_delay +
              network->getConfig()->min_synapse_delay;
  return delay;
}
double Synapse::randomWeight() {
  double weight = (std::abs(static_cast<double>(network->getRandom())) /
                   static_cast<double>(RAND_MAX)) *
                  network->getConfig()->max_weight;
  return weight;
}
void Synapse::updateWeight(double newWeight) {
  _lastWeight = _weight;
  _weight = newWeight;
}

void Synapse::updateDelay(int delay) { this->delay = delay; }
