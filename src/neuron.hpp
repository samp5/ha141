#ifndef NEURON
#define NEURON

#include "log.hpp"
#include "message.hpp"
#include "neuron_group.hpp"
#include "synapse.hpp"
#include <iostream>
#include <list>
#include <pthread.h>
#include <unistd.h>

using std::cout;
using std::list;

class NeuronGroup;

enum Neuron_t { None = 0, Input = 1, Hidden = 2, Output = 3 };

class Neuron {
protected:
  vector<LogData *> log_data;

  // Neuron vaules
  double membrane_potential; /**< Membrane potential of a Neuron */
  int excit_inhib_value;
  int id;
  Neuron_t type;
  NeuronGroup *group;
  bool active = false;

  // timestamp data
  double last_decay; /**< The timestamp of the most recent decay */
  double refractory_start =
      0.0; /**< The timestamp of most recent refractory period start */

  // Edge values
  vector<Synapse *>
      PostSynapticConnnections; /**< vector of Synapse pointers to which this
                                   Neuron is connected */
  vector<Synapse *>
      PreSynapticConnections; /**< vector of Synapse pointers from which this
                                 Neuron has Connections */

  // message list
  list<Message *> messages; /**< list of Message pointers  */

public:
  Neuron(int _id, NeuronGroup *group, Neuron_t type);
  virtual ~Neuron();

  void addNeighbor(Neuron *neighbor, double weight);
  void addPostSynapticConnection(Synapse *synapse);
  void addPreSynapticConnection(Synapse *synapse);

  // Running and messaging
  virtual void run();
  virtual void sendMessages();

  int recieveMessage();
  void addMessage(Message *);
  Message *retrieveMessage();
  const vector<Synapse *> &getSynapses() const;

  // State operations
  virtual void reset();
  void refractory();
  void setType(Neuron_t type);
  void activate();
  void deactivate();
  void retroactiveDecay(double from, double to);
  void accumulatePotential(double value);
  int generateInhibitoryStatus();

  // GETTERS
  bool isActivated() const;

  double getPotential() const;
  NeuronGroup *getGroup() const;

  const list<Message *> &getMessageVector() const;
  const vector<Synapse *> &getPostSynaptic() const;
  const vector<Synapse *> &getPresynaptic() const;

  double getLastDecay() const;
  int getBias() const;
  Neuron_t getType() const;
  int getID() const;

  // log operations
  void addData(double time, Message_t message_type);
  void transferData();
};

#endif // !NEURON
