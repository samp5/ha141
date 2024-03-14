#ifndef NEURON_GROUP
#define NEURON_GROUP

#include "log.hpp"
#include "message.hpp"
#include <list>
#include <pthread.h>

class Neuron;

extern Log lg;
extern const int WAIT_TIME;
extern const int WAIT_INCREMENT;
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

  // separate message buffer for inter group messages
  list<Message *> intragroup_messages;
  list<Message *> intergroup_messages;
  // When a neuron sends a message within its own group:
  //    Add the message to the queue
  // process_intragroup_queue should:
  //    read from the start of the queue
  //    add the message the right neurons queue

public:
  NeuronGroup(int _id, int number_neurons);
  // might move neuron memory responsibilities to this class
  ~NeuronGroup();

  void process_intragroup_queue();
  void process_intergroup_queue();
  void *group_run();
  void start_thread() {
    pthread_create(&thread, NULL, NeuronGroup::thread_helper, this);
  }
  void add_to_intragroup(Message *message);
  void add_to_intergroup(Message *message);
  double get_id() const { return id; }

  pthread_t get_thread_id() { return thread; }
  void print_group();
  int neuron_count();
  double get_message();
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
