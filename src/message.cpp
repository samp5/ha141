#include "message.hpp"
#include "log.hpp"
#include "neuron.hpp"

extern Log lg;
Message::Message(double value, Neuron *target, Message_t type)
    : message(value), post_synaptic_neuron(target),
      target_neuron_group(target->getGroup()), timestamp(lg.time()),
      message_type(type) {}
