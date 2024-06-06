#ifndef NEURON_GROUP
#define NEURON_GROUP

#include "log.hpp"
#include "message.hpp"
#include <list>
#include <pthread.h>
#include <set>

class Neuron;
class InputNeuron;
class SNN;
struct IGlimit;

using std::list;

class NeuronGroup {
private:
  vector<Neuron *> all_neurons;
  vector<Neuron *> nI_neurons;
  vector<InputNeuron *> input_neurons;
  int id;

  int most_recent_timestamp;
  pthread_mutex_t time_stamp_tex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_t limit_tex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t limit_cond = PTHREAD_COND_INITIALIZER;

  pthread_t thread;
  SNN *network;
  std::multiset<Message *, MessageComp> message_q;
  pthread_mutex_t message_q_tex = PTHREAD_MUTEX_INITIALIZER;
  std::vector<NeuronGroup *> interGroupConnections;

public:
  NeuronGroup(int _id, int number_neurons, int number_input_neurons,
              SNN *network);
  ~NeuronGroup();

  void *run();

  void startThread() {
    pthread_create(&thread, NULL, NeuronGroup::thread_helper, this);
  }

  int getID() const { return id; }

  pthread_t getThreadID() const { return thread; }
  SNN *getNetwork() const { return network; }
  int neuronCount() const;
  Message *getMessage();
  void addToMessageQ(Message *message);
  int generateRandomSynapses(int n_edges);
  void addInterGroupConnections(NeuronGroup *group);
  pthread_mutex_t &getMessageQtex() { return message_q_tex; }
  void reset();
  vector<Neuron *> &getMutNeuronVec();
  Neuron *getNonInputNeuron() const;
  Neuron *getRandNeuron() const;
  const vector<Neuron *> &getNeuronVec() const;
  void updateTimestamp(int mr);
  int getTimestamp();
  IGlimit findLimitingGroup();
  pthread_cond_t &getLimitCond() { return limit_cond; }
  pthread_mutex_t &getLimitTex() { return limit_tex; }
  void logUnseqMessage(Message *message, int last_timestamp);

  /*--------------------------------------------------------------*\
   *                  Thread helper:
   *    POSIX needs a void* (*)(void*) function signature
   *    This function allows us to use the run() member funciton
  \--------------------------------------------------------------*/
  static void *thread_helper(void *instance) {
    return ((NeuronGroup *)instance)->run();
  }
};
struct IGlimit {
  NeuronGroup *limitingGroup;
  int timestamp;
  IGlimit(NeuronGroup *_g, int _t) : limitingGroup(_g), timestamp(_t){};
  pthread_cond_t &getLimitCond() { return limitingGroup->getLimitCond(); }
  pthread_mutex_t &getLimitTex() { return limitingGroup->getLimitTex(); }
  void updateTimestamp() { timestamp = limitingGroup->getTimestamp(); }
};

#endif // !NEURON_GROUP
