#ifndef MESSAGE
#define MESSAGE

class NeuronGroup;
class Neuron;

enum Message_t { Decay, Stimulus, From_Neighbor, Refractory, Checked };

struct Message {
public:
  Message(double value, Neuron *target, Message_t type);
  double message;
  Neuron *presynaptic_neuron;
  Neuron *post_synaptic_neuron;
  NeuronGroup *target_neuron_group;
  double timestamp;
  Message_t message_type;
};

#endif // !MESSAGE
