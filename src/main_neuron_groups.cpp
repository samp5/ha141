#include "runtime.hpp"
RuntimConfig cf;
Mutex mx;

#include "log.hpp"
#include "network.hpp"
#include "neuron.hpp"
#include "neuron_group.hpp"
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
  if (!cf.parseArgs(argv, argc)) {
    mx.destroy_mutexes();
    return 0;
  }
  // Check the start conditions
  cf.checkStartCond();

  // Set seed
  srand(cf.RAND_SEED);

  lg.log(ESSENTIAL, "Assigning neuron groups...");

  SNN snn = SNN(&cf);

  // Add random edges between neurons
  lg.log(ESSENTIAL, "Adding synapses...");
  snn.generateRandomSynapses();

  lg.start_clock();
  snn.start();

  lg.log(ESSENTIAL, "Transfering data from Neurons to Log...");
  snn.join();

  lg.log(ESSENTIAL, "Writing data to file...");
  lg.write_data();
  lg.log(ESSENTIAL, "Done writing, exiting");

  // Destroy mutexes
  mx.destroy_mutexes();
  pthread_cond_destroy(&stimulus_switch_cond);
  return 0;
}
