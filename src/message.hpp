#ifndef MESSAGE
#define MESSAGE

class NeuronGroup;
class Neuron;

typedef struct {
  double message;
  Neuron *target_neuron;
  NeuronGroup *target_neuron_group;
  double timestamp;
} Message;

#endif // !MESSAGE
