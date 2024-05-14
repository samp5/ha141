#ifndef INPUT_NEURON
#define INPUT_NEURON

#include "neuron.hpp"
#include "neuron_group.hpp"
#include <pthread.h>

extern bool switching_stimulus;
extern pthread_cond_t stimulus_switch_cond;

class InputNeuron : public Neuron {
protected:
  double input_value;
  double probalility_of_success;

public:
  InputNeuron(int _id, NeuronGroup *group);

  void reset();
  void run();
  bool poisson_result();
  void set_input_value(double value);
  bool check_refractory_period();
  void sendMessages();
  double getInputValue() const { return this->input_value; }
};

#endif // !INPUT_NEURON
