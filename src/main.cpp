#include "network.hpp"
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <vector>

using std::string;

int main(int argc, char **argv) {
  std::vector<std::string> args(argv, argv + argc);
  SNN snn = SNN(args);
  snn.lg->log(ESSENTIAL, "Assigning neuron groups...");

  snn.lg->log(ESSENTIAL, "Adding synapses...");
  snn.generateRandomSynapses();

  snn.lg->startClock();
  snn.start();

  snn.lg->log(ESSENTIAL, "Transfering data from Neurons to Log...");
  snn.join();

  snn.lg->writeData();

  snn.lg->log(ESSENTIAL, "Done writing, exiting");
  return 0;
}
