#ifndef NEURON_GROUP
#define NEURON_GROUP

#include "log.hpp"
#include <pthread.h>

class Neuron;

extern Log lg;

// This class is so that many neurons can run on one thread
// All neurons will still be allocated on the heap

class NeuronGroup {
  // NOT COMPLETE
private:
  vector<Neuron *> neurons;
  int id;
  pthread_t thread;

  // this variable should be read only from outside the class
  // Analagous to "value" in previous version
  // Message buffer
  // Queue data structure
  // should be a struct
  int message;
  // separate message buffer for inter group messages

public:
  NeuronGroup(int _id, int number_neurons);
  // might move neuron memory responsibilities to this class
  ~NeuronGroup();

  void *group_run();
  void start_thread() { pthread_create(&thread, NULL, thread_helper, this); }
  double get_message() const { return message; }
  double get_id() const { return id; }

  void set_message(double message);
  void print_group();
  int neuron_count();
  const vector<Neuron *> &get_neruon_vector();

  /*--------------------------------------------------------------*\
   *                  Thread helper:
   *    POSIX needs a void* (*)(void*) function signature
   *    This function allows us to use the run() member funciton
  \--------------------------------------------------------------*/
  static void *thread_helper(void *instance) {
    return ((NeuronGroup *)instance)->group_run();
  }
};

#endif // !NEURON_GROUP
#define NEURON_GROUP
