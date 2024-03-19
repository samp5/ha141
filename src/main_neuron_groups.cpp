
#include "functions.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include "neuron_group.hpp"
#include <pthread.h>
#include <string>
#include <unistd.h>

using std::cout;
using std::string;

int INITIAL_MEMBRANE_POTENTIAL;
int ACTIVATION_THRESHOLD;
int REFRACTORY_MEMBRANE_POTENTIAL;
int RAND_SEED;
int NUMBER_EDGES;
int NUMBER_GROUPS;
int NUMBER_NEURONS;
int NUMBER_INPUT_NEURONS;
int WAIT_LOOPS;
int WAIT_TIME; // in microseconds
double TAU;
unsigned long RUN_TIME; // in microseconds
double REFRACTORY_DURATION;
double DECAY_VALUE;
LogLevel DEBUG_LEVEL;
std::string INPUT_FILE;
std::string CONFIG_FILE;
ostream &STREAM = cout;

Log lg;

// Protects the membrane_potential
pthread_mutex_t potential_mutex = PTHREAD_MUTEX_INITIALIZER;

// Protects output stream
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

// Protects activation
pthread_mutex_t activation_mutex = PTHREAD_MUTEX_INITIALIZER;

// Protects messaging
pthread_mutex_t message_mutex = PTHREAD_MUTEX_INITIALIZER;

// This is only for non-group runs
volatile double value = 0;

// This is only for non-group runs
bool finish = false;

// This is for group runs
bool active = true;

int main(int argc, char **argv) {

  // If we are unable to parse, just quit
  if (!parse_command_line_args(argv, argc)) {
    pthread_mutex_destroy(&potential_mutex);
    pthread_mutex_destroy(&log_mutex);
    pthread_mutex_destroy(&message_mutex);
    pthread_mutex_destroy(&activation_mutex);
    return 0;
  }

  // If the number of input neurons exceeds the number of neurons quite
  if (NUMBER_INPUT_NEURONS >= NUMBER_NEURONS) {
    lg.log(ERROR, "NUMBER_INPUT_NEURONS greater than or equal to "
                  "NUMBER_NEURONS... quitting");
    pthread_mutex_destroy(&potential_mutex);
    pthread_mutex_destroy(&log_mutex);
    pthread_mutex_destroy(&message_mutex);
    pthread_mutex_destroy(&activation_mutex);
    return 0;
  }

  // Set seed
  srand(RAND_SEED);

  // Reserve the vector memory
  vector<NeuronGroup *> neuron_groups(NUMBER_GROUPS);

  lg.print("\nAdding Neurons\n");
  lg.print("==================");

  // Assign neurons to groups
  assign_groups(neuron_groups);

  // assign neurons types
  // this just assigns input neurons for now
  assign_neuron_types(neuron_groups);

  // Add random edges between neurons
  random_group_neighbors(neuron_groups, NUMBER_EDGES);

  // Get message vector from file
  vector<Message *> messages =
      construct_message_vector_from_file(neuron_groups, INPUT_FILE);

  // Print out groups
  for (auto group : neuron_groups) {
    group->print_group();
  }

  // Print out messages
  for (auto message : messages) {
    print_message(message);
  }

  // thread ids
  pthread_t messaging_thread;
  pthread_t decay_thread;

  // create messager thread and pass pointer to message array
  pthread_create(&messaging_thread, NULL, send_message_helper,
                 (void *)&messages);

  // create decay thread and pass pointer to neuron groups
  pthread_create(&decay_thread, NULL, decay_helper, (void *)&neuron_groups);

  // start all group threads
  for (auto group : neuron_groups) {
    group->start_thread();
  }

  // sleep for runtime
  usleep(RUN_TIME);
  active = false;

  lg.log(INFO, "Writing data to file...");
  lg.write_data();

  deallocate_message_vector(&messages);

  // Deallocate groups
  for (auto group : neuron_groups) {
    delete group;
  }

  // Destroy mutexes
  pthread_mutex_destroy(&potential_mutex);
  pthread_mutex_destroy(&log_mutex);
  pthread_mutex_destroy(&message_mutex);
  pthread_mutex_destroy(&activation_mutex);
  return 0;
}
