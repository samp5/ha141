#include "synapse.hpp"
#include "message.hpp"
#include "network.hpp"
#include "neuron.hpp"
#include "runtime.hpp"
#include <cstdlib>

Synapse::Synapse(Neuron *from, Neuron *to, double w, double delay)
    : _origin(from), _destination(to),
      network(_origin->getGroup()->getNetwork()),
      _weight(w == -1 ? randomWeight() : delay),
      delay(delay == -1 ? randomDelay() : delay){};

/**
 * @brief Propagates a message.
 *
 * Activates the recieving neuron and calls Neuron::addMessage
 *
 */
void Synapse::propagate() {

  int preGroupID = getPreSynaptic()->getGroup()->getID();
  int postGroupID = getPostSynaptic()->getGroup()->getID();
  int preID = getPreSynaptic()->getID();
  int postID = getPostSynaptic()->getID();

  double message_value = getPreSynaptic()->getPotential() * getWeight() *
                         getPreSynaptic()->getBias();

  _origin->getGroup()->getNetwork()->lg->neuronInteraction(
      INFO, "Group %d: Neuron %d is sending a mesage to Group %d: Neuron %d",
      preGroupID, preID, postGroupID, postID);

  Message *message_to_send =
      new Message(message_value, _origin, _destination, From_Neighbor,
                  _origin->getLastFire() + delay);

  getPostSynaptic()->activate();
  getPostSynaptic()->getGroup()->addToMessageQ(message_to_send);
}

int Synapse::randomDelay() {
  int delay = rand() % network->getConfig()->max_synapse_delay +
              network->getConfig()->min_synapse_delay;
  return delay;
}
double Synapse::randomWeight() {
  double weight =
      (static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) *
      network->getConfig()->max_weight;
  return weight;
}
