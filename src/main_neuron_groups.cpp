
#include "functions.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include "neuron_group.hpp"
#include <chrono>
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
double INPUT_PROB_SUCCESS;
LogLevel DEBUG_LEVEL;
std::string INPUT_FILE;
std::string CONFIG_FILE;
ostream &STREAM = cout;

Log lg;

pthread_mutex_t stimulus_switch_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t stimulus_switch_cond = PTHREAD_COND_INITIALIZER;
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

// This is for group runs
bool active = true;
bool finish = false;
bool switching_stimulus = false;

int main(int argc, char **argv) {

  // If we are unable to parse, just quit
  if (!parse_command_line_args(argv, argc)) {
    destroy_mutexes();
    return 0;
  }

  // Check the start conditions
  check_start_conditions();

  // Set seed
  srand(RAND_SEED);

  // Reserve the vector memory
  vector<NeuronGroup *> neuron_groups(NUMBER_GROUPS);

  // Assign neurons to groups
  assign_groups(neuron_groups);

  // Add random edges between neurons
  random_synapses(neuron_groups, NUMBER_EDGES);

  // Get message vector from file

  vector<InputNeuron *> input_neurons;
  construct_input_neuron_vector(neuron_groups, input_neurons);

  // set the input to a specific line
  set_line_x(input_neurons, 4);

  // start all group threads
  lg.start_clock();
  for (auto group : neuron_groups) {
    group->start_thread();
  }

  int num_stim = 1;
  int time_per_stim = (double)RUN_TIME / num_stim;

  for (int i = 1; i < num_stim + 1; i++) {

    usleep(time_per_stim);

    if (i < num_stim) {
      auto start = std::chrono::high_resolution_clock::now();
      switching_stimulus = true;

      set_next_line(input_neurons);

      pthread_mutex_lock(&stimulus_switch_mutex);
      switching_stimulus = false;
      pthread_cond_broadcast(&stimulus_switch_cond);
      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(
          end - start);
      lg.set_offset(duration.count());
      pthread_mutex_unlock(&stimulus_switch_mutex);
    }
  }

  active = false;

  for (auto group : neuron_groups) {
    pthread_join(group->get_thread_id(), NULL);
  }

  // Deallocate groups
  for (auto group : neuron_groups) {
    delete group;
  }

  lg.log(INFO, "Writing data to file...");
  lg.write_data();

  // Destroy mutexes
  destroy_mutexes();
  pthread_cond_destroy(&stimulus_switch_cond);
  return 0;
}
