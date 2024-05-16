#include "network.hpp"
#include <pthread.h>
#include <string>
#include <unistd.h>

using std::string;

pthread_cond_t stimulus_switch_cond = PTHREAD_COND_INITIALIZER;

// This is for group runs
bool active = true;
bool switching_stimulus = false;

int main(int argc, char **argv) {
  SNN snn = SNN(argc, argv);
  snn.lg->log(ESSENTIAL, "Assigning neuron groups...");

  snn.lg->log(ESSENTIAL, "Adding synapses...");
  snn.generateRandomSynapses();

  snn.lg->startClock();
  snn.start();

  snn.lg->log(ESSENTIAL, "Transfering data from Neurons to Log...");
  snn.join();

  snn.lg->log(ESSENTIAL, "Writing data to file...");
  snn.lg->writeData();

  snn.lg->log(ESSENTIAL, "Done writing, exiting");
  return 0;
}
