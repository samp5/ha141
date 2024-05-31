/** @file */
#ifndef MESSAGE
#define MESSAGE
class NeuronGroup;
class Neuron;

/**
 * \enum Message_t
 * Message type.
 */
enum Message_t { Decay, Stimulus, From_Neighbor, Refractory };

/**
 *
 * \struct Message
 * Datastructure for messages.
 *
 */
struct Message {
public:
  Message(double value, Neuron *target, Message_t type, double timestamp);
  double message;
  Neuron *presynaptic_neuron;
  Neuron *post_synaptic_neuron;
  NeuronGroup *target_neuron_group;
  int timestamp;
  Message_t message_type;
  bool operator>(const Message &other) const {
    return timestamp > other.timestamp;
  }
  bool operator<(const Message &other) const {
    return timestamp < other.timestamp;
  }
};

struct MessageComp {
  bool operator()(const Message *lhs, const Message *rhs) const {
    return lhs->timestamp < rhs->timestamp;
  }
};

#endif // !MESSAGE
