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

  double message_value = getPreSynaptic()->getPotential() *
                         getWeight() * getPreSynaptic()->getBias();

  _origin->getGroup()->getNetwork()->lg->neuronInteraction(
      INFO, "Group %d: Neuron %d is sending a mesage to Group %d: Neuron %d",
      preGroupID, preID, postGroupID, postID);

  // TODO:
  // add delay to synapse (some random value around 1 millisecond)
  Message *message_to_send =
      new Message(message_value, getPostSynaptic(), From_Neighbor);

  getPostSynaptic()->activate();
  getPostSynaptic()->addMessage(message_to_send);
}

double Synapse::randomDelay() { return (double)rand() / RAND_MAX * 0.01; }
