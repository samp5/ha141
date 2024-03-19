#ifndef MESSAGE
#define MESSAGE

class NeuronGroup;
class Neuron;

typedef struct {
  double message;
  Neuron *presynaptic_neuron;
  Neuron *post_synaptic_neuron;
  NeuronGroup *target_neuron_group;
  double timestamp;
} Message;

#endif // !MESSAGE
