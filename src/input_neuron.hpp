#ifndef INPUT_NEURON
#define INPUT_NEURON

#include "neuron.hpp"
#include "neuron_group.hpp"
#include <pthread.h>

extern bool
    switching_stimulus; /**< Global bool for when stimulus is switching */
extern pthread_cond_t stimulus_switch_cond; /**< pthread_cond_t for halting
                                               threads on stimulus switch */

class InputNeuron : public Neuron {
protected:
  double input_value;            /**< Stimulus value */
  double probalility_of_success; /**< Probability of poisson sucess */

public:
  InputNeuron(int _id, NeuronGroup *group);
  void reset();
  void run();
  bool poissonResult();
  void setInputValue(double value);
  bool inRefractory();
  void sendMessages();
  double getInputValue() const { return this->input_value; }
};

#endif // !INPUT_NEURON
