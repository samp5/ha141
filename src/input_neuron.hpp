#ifndef INPUT_NEURON
#define INPUT_NEURON

#include "neuron.hpp"
#include "neuron_group.hpp"
#include <pthread.h>

extern double INPUT_PROB_SUCCESS;
extern pthread_mutex_t stimulus_switch_mutex;
extern pthread_cond_t stimulus_switch_cond;
extern bool switching_stimulus;

class InputNeuron : public Neuron {
private:
  double input_value;
  double probalility_of_success = INPUT_PROB_SUCCESS;

public:
  InputNeuron(int _id, NeuronGroup *group) : Neuron(_id, -1, group, Input) {
    this->activate();
  }

  void run_in_group();
  bool poisson_result();
  void set_input_value(double value);
  bool check_refractory_period();
  void send_messages_in_group();
};

#endif // !INPUT_NEURON
