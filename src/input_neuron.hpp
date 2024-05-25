#ifndef INPUT_NEURON
#define INPUT_NEURON

#include "neuron.hpp"
#include "neuron_group.hpp"
#include <pthread.h>

/**
 * Neuron subclass for recieving external stimulus
 *
 */
class InputNeuron : public Neuron {
protected:
  double input_value;            /**< Stimulus value */
  double probalility_of_success; /**< Probability of poisson sucess */
  double latency;

public:
  InputNeuron(int _id, NeuronGroup *group, double latency);
  void reset();
  void run();
  bool poissonResult();
  void setInputValue(double value);
  void setLatency(double latency);
  bool inRefractory();
  void sendMessages();
  double getInputValue() const { return this->input_value; }
  double getLatency() const { return this->latency; }
};

#endif // !INPUT_NEURON
