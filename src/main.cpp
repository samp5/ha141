#include "log.hpp"
#include "network.hpp"
#include "neuron.hpp"
#include "neuron_group.hpp"
#include "runtime.hpp"
#include <pthread.h>
#include <string>
#include <unistd.h>

using std::string;

Log lg;
RuntimConfig cf;
Mutex mx;

pthread_cond_t stimulus_switch_cond = PTHREAD_COND_INITIALIZER;

// This is for group runs
bool active = true;
bool switching_stimulus = false;

int main(int argc, char **argv) {
  if (!cf.parseArgs(argv, argc)) {
    mx.destroy_mutexes();
    return 0;
  }

  cf.checkStartCond();
  srand(cf.RAND_SEED);

  lg.log(ESSENTIAL, "Assigning neuron groups...");
  SNN snn = SNN(&cf);

  lg.log(ESSENTIAL, "Adding synapses...");
  snn.generateRandomSynapses();

  lg.startClock();
  snn.start();

  lg.log(ESSENTIAL, "Transfering data from Neurons to Log...");
  snn.join();

  lg.log(ESSENTIAL, "Writing data to file...");
  lg.writeData();

  lg.log(ESSENTIAL, "Done writing, exiting");
  mx.destroy_mutexes();

  pthread_cond_destroy(&stimulus_switch_cond);
  return 0;
}
