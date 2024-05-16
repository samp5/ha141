#include "message.hpp"
#include "log.hpp"
#include "network.hpp"
#include "neuron.hpp"

Message::Message(double value, Neuron *target, Message_t type)
    : message(value), post_synaptic_neuron(target),
      target_neuron_group(target->getGroup()),
      timestamp(target->getGroup()->getNetwork()->lg->time()),
      message_type(type) {}
