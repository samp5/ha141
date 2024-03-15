
#include "functions.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include "neuron_group.hpp"
#include <pthread.h>
#include <unistd.h>

using std::cout;

int INITIAL_MEMBRANE_POTENTIAL;
int ACTIVATION_THRESHOLD;
int REFRACTORY_MEMBRANE_POTENTIAL;
int RAND_SEED;
int NUMBER_EDGES;
int NUMBER_GROUPS;
int NUMBER_NODES;
int WAIT_LOOPS;
int WAIT_TIME;          // in microseconds
unsigned long RUN_TIME; // in microseconds
double DECAY_VALUE;

ostream &STREAM = cout;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t message_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;
volatile double value = 0;
bool finish = false;
Log lg;
bool active = true;
LogLevel DEBUG_LEVEL;

/*
  1 - ERROR,
  2 - WARNING,
  3 - INFO,
  4 - DEBUG,
  5 - DEBUG2,
  6 - DEBUG3,
  7 - DEBUG4,
*/

int main(int argc, char **argv) {

  if (!parse_command_line_args(argv, argc)) {
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&log_mutex);
    pthread_mutex_destroy(&message_mutex);
    return 0;
  }

  srand(RAND_SEED);

  vector<NeuronGroup *> neuron_groups(NUMBER_GROUPS);

  // will potentially leave a remainder
  int neuron_per_group = NUMBER_NODES / NUMBER_GROUPS;

  // reamainder is guearenteed to be less than the number of groups
  int remainder = NUMBER_NODES % NUMBER_GROUPS;

  cout << "\nAdding Neurons\n";
  cout << "=================";

  for (int i = 0; i < NUMBER_GROUPS; i++) {

    // check for remainder and the neurons in the current group accordingly
    int per_group = remainder ? neuron_per_group + 1 : neuron_per_group;

    if (remainder)
      remainder--;

    // allocate for this group
    NeuronGroup *this_group = new NeuronGroup(i + 1, per_group);

    // add to vector
    neuron_groups[i] = this_group;
  }

  random_group_neighbors(neuron_groups, NUMBER_EDGES);

  for (auto group : neuron_groups) {
    group->print_group();
  }

  vector<Message *> messages =
      construct_message_vector_from_file(neuron_groups, "./input_files/test");

  for (auto message : messages) {
    print_message(message);
  }

  // thread id
  pthread_t messaging_thread;
  pthread_t decay_thread;

  // create messager thread and pass pointer to message array
  pthread_create(&messaging_thread, NULL, send_message_helper,
                 (void *)&messages);
  pthread_create(&decay_thread, NULL, decay_helper, (void *)&neuron_groups);

  // start all group threads
  for (auto group : neuron_groups) {
    group->start_thread();
  }

  usleep(RUN_TIME);
  active = false;

  lg.log(INFO, "Writing data to file...");
  lg.write_data();

  deallocate_message_vector(&messages);

  for (auto group : neuron_groups) {
    group->print_group();
  }

  for (auto group : neuron_groups) {
    delete group;
  }

  pthread_mutex_destroy(&mutex);
  pthread_mutex_destroy(&log_mutex);
  pthread_mutex_destroy(&message_mutex);
  return 0;
}
