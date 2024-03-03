#ifndef NEURON
#define NEURON

#include <iostream>
#include <map>
#include <pthread.h>
#include <unistd.h>

#define INITIAL_MEMBRANE_POTENTIAL -55
#define ACTIVATION_THRESHOLD -55
#define REFRACTORY_MEMBRANE_POTENTIAL -70

using std::cout;
extern pthread_mutex_t mutex;
extern volatile double value;
extern bool finish;
extern pthread_barrier_t barrier;

enum Neuron_t { Input, Hidden };

class Neuron {
private:
  // Neuron vaules
  double membrane_potential = INITIAL_MEMBRANE_POTENTIAL;
  int excit_inhib_value;
  int id;
  Neuron_t type;

  // Edge values
  typedef std::map<Neuron *, double> weight_map;
  weight_map _postsynaptic;
  weight_map _presynaptic;

  // pthread values
  pthread_t thread;
  pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

  // Conditional execution values
  bool active = false;
  bool recieved = false;

public:
  Neuron(int _id, int inhibitory);
  ~Neuron();
  void add_neighbor(Neuron *neighbor, double weight);
  void add_next(Neuron *neighbor, double weight);
  void add_previous(Neuron *neighbor, double weight);
  void *run();
  void start_thread();
  void join_thread();
  void refractory();
  void activate() { active = true; }
  void deactivate() { active = false; }

  //>>>>>>>>>>>>>> Access to private variables <<<<<<<<<<<
  pthread_t get_thread_id() { return thread; }
  pthread_cond_t *get_cond() { return &cond; }
  int get_id() { return id; }
  double get_potential() { return membrane_potential; }
  const weight_map *get_presynaptic() {
    const weight_map *p_presynaptic = &_presynaptic;
    return p_presynaptic;
  }

  const weight_map *get_postsynaptic() {
    const weight_map *p_postsynaptic = &_postsynaptic;
    return p_postsynaptic;
  }

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
