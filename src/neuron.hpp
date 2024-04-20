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

extern pthread_mutex_t potential_mutex;
extern pthread_mutex_t message_mutex;
extern pthread_mutex_t activation_mutex;

extern volatile double value;
extern bool finish;
extern pthread_barrier_t barrier;
extern Log lg;

extern double DECAY_VALUE;
extern double TAU;
extern int INITIAL_MEMBRANE_POTENTIAL;
extern int ACTIVATION_THRESHOLD;
extern int REFRACTORY_MEMBRANE_POTENTIAL;
extern double REFRACTORY_DURATION;

class NeuronGroup;

enum Neuron_t { None = 0, Input = 1, Hidden = 2, Output = 3 };

class Neuron {
protected:
  vector<LogData *> log_data;

  // Neuron vaules
  double membrane_potential = INITIAL_MEMBRANE_POTENTIAL;
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
  Neuron(int _id, int inhibitory);
  Neuron(int _id, int inhibitory, NeuronGroup *group, Neuron_t type);
  Neuron(int _id, int inhibitory, NeuronGroup *group);
  virtual ~Neuron();

  void add_neighbor(Neuron *neighbor, double weight);
  void add_next(Neuron *neighbor, double weight);

  void addPostSynapticConnection(Synapse *synapse);
  void addPreSynapticConnnection(Synapse *synapse);

  void add_previous(Neuron *neighbor, double weight);

  // Running and messaging
  void *run();
  virtual void run_in_group();
  int recieve_in_group();
  int check_run_conditions();
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
  double decay(double timestamp, double tau = TAU,
               double v_rest = REFRACTORY_MEMBRANE_POTENTIAL);
  void retroactive_decay(double from, double to, double tau = TAU,
                         double v_rest = REFRACTORY_MEMBRANE_POTENTIAL);
  void update_potential(double value);

  // Thread operations
  void start_thread();
  void join_thread();

  //>>>>>>>>>>>>>> Access to private variables <<<<<<<<<<<
  pthread_t get_thread_id() { return thread; }
  pthread_cond_t *get_cond() { return &cond; }
  double get_last_decay() { return this->last_decay; }
  const vector<Synapse *> &getPostSynaptic() const {
    return this->PostSynapticConnnections;
  }
  const vector<Synapse *> &getPresynaptic() const {
    return this->PreSynapticConnections;
  }

  // returns the IO type of the neuron
  Neuron_t get_type() { return this->type; }
  int get_id() { return id; }
  double get_potential();
  const list<Message *> &get_message_vector();
  const weight_map *get_presynaptic() const;
  const weight_map *get_postsynaptic() const;
  NeuronGroup *get_group();
  bool is_activated() const;
  int getBias() { return this->excit_inhib_value; }

  // log operations
  void push_back_data(LogData *data) { this->log_data.push_back(data); }
  void transfer_data();

  /*--------------------------------------------------------------*\
   *                  Thread helper:
   *    POSIX needs a void* (*)(void*) function signature
   *    This function allows us to use the run() member funciton
  \--------------------------------------------------------------*/
  static void *thread_helper(void *instance) {
    return ((Neuron *)instance)->run();
  }
};

#endif // !NEURON
