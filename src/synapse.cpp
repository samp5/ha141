#include "synapse.hpp"
#include "message.hpp"
#include "neuron.hpp"

void Synapse::propagate() {

  int preGroupID = this->getPreSynaptic()->getGroup()->getID();
  int postGroupID = this->getPostSynaptic()->getGroup()->getID();
  int preID = this->getPreSynaptic()->getID();
  int postID = this->getPostSynaptic()->getID();

  double message_value = this->getPreSynaptic()->getPotential() *
                         this->getWeight() * this->getPreSynaptic()->getBias();

  lg.log_group_neuron_interaction(
      INFO, "Group %d: Neuron %d is sending a mesage to Group %d: Neuron %d",
      preGroupID, preID, postGroupID, postID);

  // TODO:
  // add delay to synapse (some random value around 1 millisecond)
  Message *message_to_send =
      new Message(message_value, this->getPostSynaptic(), From_Neighbor);

  this->getPostSynaptic()->activate();
  this->getPostSynaptic()->addMessage(message_to_send);
}
