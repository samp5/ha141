#ifndef OUTPUT_NEURON
#define OUTPUT_NEURON

#include "neuron.hpp"
#include "neuron_group.hpp"

class OutputNeuron : public Neuron {

public:
  OutputNeuron(int _id, NeuronGroup *group) : Neuron(_id, -1, group, Output){};
  void run_in_group();
  bool check_refractory_period();
  void send_messages_in_group();
};

#endif // !OUTPUT_NEURON
