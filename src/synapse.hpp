#ifndef SYNAPSE
#define SYNAPSE
#include "message.hpp"
class Neuron;

/**
 * @brief Synapse class for managing connections.
 *
 * All `Neuron`s hold a vector of PostSynaptic and Presynaptic Synapse pointers
 * The Synapse manages edge weighs and propagation
 *
 */
class Synapse {
public:
  Synapse(Neuron *from, Neuron *to, double w, double delay = -1)
      : _origin(from), _destination(to), _weight(w),
        delay(delay == -1 ? randomDelay() : delay){};
  Neuron *getPostSynaptic() { return _destination; }
  Neuron *getPreSynaptic() { return _origin; }
  void propagate();
  double randomDelay();
  void alterWeight(double weight);
  double getWeight() { return _weight; }

private:
  Neuron *_origin = nullptr;
  Neuron *_destination = nullptr;
  double _weight = 0.0;
  int delay = -1;
};

#endif // !SYNAPSE
