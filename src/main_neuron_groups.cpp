#include "globals.hpp"
RuntimConfig cf;
Mutex mx;

#include "functions.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include "neuron_group.hpp"
#include <chrono>
#include <pthread.h>
#include <string>
#include <unistd.h>

using std::string;

Log lg;

pthread_cond_t stimulus_switch_cond = PTHREAD_COND_INITIALIZER;

// This is for group runs
bool active = true;
bool switching_stimulus = false;

int main(int argc, char **argv) {

  // If we are unable to parse, just quit
  if (!parse_command_line_args(argv, argc)) {
    mx.destroy_mutexes();
    return 0;
  }

  // Check the start conditions
  check_start_conditions();

  // Set seed
  srand(cf.RAND_SEED);

  // Reserve the vector memory
  std::vector<NeuronGroup *> neuron_groups(cf.NUMBER_GROUPS);

  lg.log(ESSENTIAL, "Assigning neuron groups...");
  // Assign neurons to groups
  assign_groups(neuron_groups);

  lg.log(ESSENTIAL, "Adding synapses...");
  auto start = lg.get_time_stamp();
  // Add random edges between neurons
  random_synapses(neuron_groups);
  auto end = lg.get_time_stamp();
  std::string msg = "Adding random synapses done: took " +
                    std::to_string(end - start) + " seconds";
  lg.log(ESSENTIAL, msg.c_str());

  vector<InputNeuron *> input_neurons;
  construct_input_neuron_vector(neuron_groups, input_neurons);

  cf.STIMULUS = cf.STIMULUS_VEC.begin();

  // set the input to a specific line
  set_line_x(input_neurons, *cf.STIMULUS);
  lg.log_value(ESSENTIAL, "Set stimulus to line %d", *cf.STIMULUS);

  // start all group threads
  lg.start_clock();
  for (auto group : neuron_groups) {
    group->start_thread();
  }

  int num_stim = cf.STIMULUS_VEC.size();
  int time_per_stim = (double)cf.RUN_TIME / num_stim;

  for (int i = 1; i < num_stim + 1; i++) {

    usleep(time_per_stim);

    if (i < num_stim) {
      auto start = std::chrono::high_resolution_clock::now();
      switching_stimulus = true;

      cf.STIMULUS++;
      set_next_line(input_neurons);
      lg.log_value(ESSENTIAL, "Set stimulus to line %d", *cf.STIMULUS);

      for (auto group : neuron_groups) {
        group->reset();
      }

      pthread_mutex_lock(&mx.stimulus);
      switching_stimulus = false;
      pthread_cond_broadcast(&stimulus_switch_cond);
      auto end = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(
          end - start);
      lg.set_offset(duration.count());
      pthread_mutex_unlock(&mx.stimulus);
    }
  }

  active = false;

  lg.log(ESSENTIAL, "Transfering data from Neurons to Log...");
  for (auto group : neuron_groups) {
    pthread_join(group->get_thread_id(), NULL);
  }

  // Deallocate groups
  for (auto group : neuron_groups) {
    delete group;
  }

  lg.log(ESSENTIAL, "Writing data to file...");
  lg.write_data();
  lg.log(ESSENTIAL, "Done writing, exiting");

  // Destroy mutexes
  mx.destroy_mutexes();
  pthread_cond_destroy(&stimulus_switch_cond);
  return 0;
}
