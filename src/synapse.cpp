#include "synapse.hpp"
#include "functions.hpp"
#include "message.hpp"
#include "neuron.hpp"

void Synapse::propagate() {

  int preGroupID = this->getPreSynaptic()->get_group()->get_id();
  int postGroupID = this->getPostSynaptic()->get_group()->get_id();
  int preID = this->getPreSynaptic()->get_id();
  int postID = this->getPostSynaptic()->get_id();

  double message_value = this->getPreSynaptic()->get_potential() *
                         this->getWeight() * this->getPreSynaptic()->getBias();

  lg.log_group_neuron_interaction(
      INFO, "Group %d: Neuron %d is sending a mesage to Group %d: Neuron %d",
      preGroupID, preID, postGroupID, postID);

  Message *message_to_send =
      construct_message(message_value, this->getPostSynaptic(), From_Neighbor);

  this->getPostSynaptic()->activate();
  this->getPostSynaptic()->add_message(message_to_send);
}
