#ifndef SYNAPSE
#define SYNAPSE

#include "message.hpp"
class Neuron;

class Synapse {
public:
  Synapse(Neuron *from, Neuron *to, double w)
      : _origin(from), _destination(to), _weight(w){};
  Neuron *getPostSynaptic() { return this->_destination; }
  Neuron *getPreSynaptic() { return this->_origin; }
  void propagate();
  void alterWeight(double weight);
  double getWeight() { return this->_weight; }

private:
  Neuron *_origin = nullptr;
  Neuron *_destination = nullptr;
  double _weight = 0.0;
  double _last_weight = -1.0;
};

#endif // !SYNAPSE
#define SYNAPSE
