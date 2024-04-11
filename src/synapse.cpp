#include "synapse.hpp"
#include "neuron.hpp"

void Synapse::propagate(Message *message) {
  this->getPostSynaptic()->add_message(message);
}
