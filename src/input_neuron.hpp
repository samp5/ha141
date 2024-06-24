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
  int latency;

public:
  InputNeuron(int _id, NeuronGroup *group, int latency);
  void reset();
  void run(Message *message);
  bool poissonResult() const;
  void setInputValue(double value);
  void setLatency(int latency);
  void generateEvents();
  void generateEvents(const std::vector<int> &timestamps);
  bool inRefractory() const;
  double getInputValue() const { return input_value; }
  int getLatency() const { return latency; }
};

#endif // !INPUT_NEURON
