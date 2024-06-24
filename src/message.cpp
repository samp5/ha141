#include "message.hpp"
#include "network.hpp"
#include "neuron.hpp"

Message::Message(double value, Neuron *origin, Neuron *target, Message_t type,
                 double timestamp)
    : message(value), presynaptic_neuron(origin), post_synaptic_neuron(target),
      target_neuron_group(target->getGroup()), timestamp(timestamp),
      message_type(type) {}

bool Message::isInterGroup() {
  // No stimulus types are intergroup
  if (message_type == Message_t::Stimulus) {
    return false;
  }
  if (presynaptic_neuron->getGroup() == post_synaptic_neuron->getGroup()) {
    return true;
  } else {
    return false;
  }
}
