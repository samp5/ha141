#include "synapse.hpp"
#include "message.hpp"
#include "network.hpp"
#include "neuron.hpp"

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
      new Message(message_value, getPostSynaptic(), From_Neighbor,
                  getPreSynaptic()->getLastFire() + delay);

  getPostSynaptic()->activate();
  getPostSynaptic()->getGroup()->addToMessageQ(message_to_send);
}

double Synapse::randomDelay() { return rand() % 10; }
