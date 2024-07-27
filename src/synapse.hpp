#ifndef SYNAPSE
#define SYNAPSE
#include "message.hpp"
class Neuron;
class SNN;

/**
 * @brief Synapse class for managing connections.
 *
 * All `Neuron`s hold a vector of PostSynaptic and Presynaptic Synapse pointers
 * The Synapse manages edge weighs and propagation
 *
 */
class Synapse {
public:
  Synapse(Neuron *from, Neuron *to, double w = -1, double delay = -1);
  Neuron *getPostSynaptic() { return _destination; }
  Neuron *getPreSynaptic() { return _origin; }
  void propagate();
  int randomDelay();
  double randomWeight();
  void updateWeight(double newWeight);
  void updateDelay(int delay);
  double getWeight() { return _weight; }

private:
  Neuron *_origin = nullptr;
  Neuron *_destination = nullptr;
  SNN *network = nullptr;
  double _weight = 0.0;
  double _lastWeight = 0.0;
  int delay = -1;
};

#endif // !SYNAPSE
