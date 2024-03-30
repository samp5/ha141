#ifndef MESSAGE
#define MESSAGE

class NeuronGroup;
class Neuron;

enum Message_t { Decay, Stimulus, From_Neighbor, Refractory };

typedef struct {
  double message;
  Neuron *presynaptic_neuron;
  Neuron *post_synaptic_neuron;
  NeuronGroup *target_neuron_group;
  double timestamp;
  Message_t message_type;

} Message;

#endif // !MESSAGE
