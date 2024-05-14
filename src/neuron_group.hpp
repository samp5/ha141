#ifndef NEURON_GROUP
#define NEURON_GROUP

#include "log.hpp"
#include "message.hpp"
#include <list>
#include <pthread.h>

class Neuron;

extern Log lg;
extern bool active;
using std::list;

// This class is so that many neurons can run on one thread
// All neurons will still be allocated on the heap

class NeuronGroup {
  // NOT COMPLETE
private:
  vector<Neuron *> neurons;
  int id;
  pthread_t thread;

public:
  NeuronGroup(int _id, int number_neurons, int number_input_neurons);
  ~NeuronGroup();

  void *run();

  void startThread() {
    pthread_create(&thread, NULL, NeuronGroup::thread_helper, this);
  }
  double getID() const { return id; }

  pthread_t getThreadID() { return thread; }
  int neuronCount();
  void reset();
  const vector<Neuron *> &getNeuronVec();

  /*--------------------------------------------------------------*\
   *                  Thread helper:
   *    POSIX needs a void* (*)(void*) function signature
   *    This function allows us to use the run() member funciton
  \--------------------------------------------------------------*/
  static void *thread_helper(void *instance) {
    return ((NeuronGroup *)instance)->run();
  }
};

#endif // !NEURON_GROUP
