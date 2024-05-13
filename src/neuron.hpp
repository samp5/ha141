#ifndef NEURON
#define NEURON

#include "log.hpp"
#include "message.hpp"
#include "neuron_group.hpp"
#include "synapse.hpp"
#include <iostream>
#include <list>
#include <map>
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

  // timestamp data
  double last_decay;
  double refractory_start = 0.0;

  // Edge values
  vector<Synapse *> PostSynapticConnnections;
  vector<Synapse *> PreSynapticConnections;
  typedef std::map<Neuron *, double> weight_map;
  weight_map _postsynaptic;
  weight_map _presynaptic;

  // pthread values
  pthread_t thread;
  pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

  // Conditional execution values
  bool active = false;
  bool recieved = false;

  // message list
  list<Message *> messages;

public:
  Neuron(int _id, NeuronGroup *group, Neuron_t type);
  virtual ~Neuron();

  void addNeighbor(Neuron *neighbor, double weight);
  void addPostSynapticConnection(Synapse *synapse);
  void addPreSynapticConnection(Synapse *synapse);

  // Running and messaging
  void *run();
  virtual void run_in_group();
  int recieve_in_group();
  void add_message(Message *);
  Message *get_message();
  virtual void send_messages_in_group();
  vector<Synapse *> &getSynapses() { return this->PostSynapticConnnections; }

  // State operations
  virtual void reset();
  void refractory();
  void set_type(Neuron_t type);
  void activate();
  void deactivate();
  double decay(double timestamp);
  void retroactive_decay(double from, double to);
  void update_potential(double value);
  int generateInhibitoryStatus();

  // Thread operations
  void start_thread();
  void join_thread();

  // GETTERS
  pthread_t getThreadID() { return thread; }
  pthread_cond_t *getPthreadCond() { return &cond; }
  double getLastDecay() { return this->last_decay; }
  const vector<Synapse *> &getPostSynaptic() const {
    return this->PostSynapticConnnections;
  }
  const vector<Synapse *> &getPresynaptic() const {
    return this->PreSynapticConnections;
  }
  int getID() { return id; }
  Neuron_t getType() { return this->type; }
  double getPotential();
  const list<Message *> &getMessageVector();
  NeuronGroup *getGroup();
  bool isActivated() const;
  int getBias() { return this->excit_inhib_value; }

  // log operations
  void push_back_data(LogData *data) { this->log_data.push_back(data); }
  void transfer_data();
};

#endif // !NEURON
