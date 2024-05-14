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

extern Log lg;

class NeuronGroup;

enum Neuron_t { None = 0, Input = 1, Hidden = 2, Output = 3 };

class Neuron {
protected:
  vector<LogData *> log_data;

  // Neuron vaules
  double membrane_potential;
  int excit_inhib_value;
  int id;
  Neuron_t type;
  NeuronGroup *group;
  bool active = false;

  // timestamp data
  double last_decay;
  double refractory_start = 0.0;

  // Edge values
  vector<Synapse *> PostSynapticConnnections;
  vector<Synapse *> PreSynapticConnections;

  // message list
  list<Message *> messages;

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
  vector<Synapse *> &getSynapses() { return this->PostSynapticConnnections; }

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

  double getPotential();
  NeuronGroup *getGroup();

  const list<Message *> &getMessageVector();
  const vector<Synapse *> &getPostSynaptic() const;
  const vector<Synapse *> &getPresynaptic() const;

  double getLastDecay() { return this->last_decay; }
  int getBias() { return this->excit_inhib_value; }
  Neuron_t getType() { return this->type; }
  int getID() { return id; }

  // log operations
  void addData(LogData *data) { this->log_data.push_back(data); }
  void transferData();
};

#endif // !NEURON
