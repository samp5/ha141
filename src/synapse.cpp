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

  int preGroupID = this->getPreSynaptic()->getGroup()->getID();
  int postGroupID = this->getPostSynaptic()->getGroup()->getID();
  int preID = this->getPreSynaptic()->getID();
  int postID = this->getPostSynaptic()->getID();

  double message_value = this->getPreSynaptic()->getPotential() *
                         this->getWeight() * this->getPreSynaptic()->getBias();

  _origin->getGroup()->getNetwork()->lg->neuronInteraction(
      INFO, "Group %d: Neuron %d is sending a mesage to Group %d: Neuron %d",
      preGroupID, preID, postGroupID, postID);

  // TODO:
  // add delay to synapse (some random value around 1 millisecond)
  Message *message_to_send =
      new Message(message_value, this->getPostSynaptic(), From_Neighbor);

  this->getPostSynaptic()->activate();
  this->getPostSynaptic()->addMessage(message_to_send);
}

double Synapse::randomDelay() { return (double)rand() / RAND_MAX * 0.01; }
