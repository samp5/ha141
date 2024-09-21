#include "network.hpp"
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <vector>

using std::string;

int main(int argc, char **argv) {
  std::vector<std::string> args(argv, argv + argc);
  // SNN snn = SNN(args);
  SNN snn = SNN();

  auto start = snn.lg->time();
  snn.lg->log(ESSENTIAL, "Assigning neuron groups...");

  snn.lg->log(ESSENTIAL, "Adding synapses...");
  snn.initializeFromSynapseFile(args, "./networkX/out.adj");

  snn.lg->startClock();
  snn.lg->log(ESSENTIAL, "Starting network...");
  snn.forkRun({{0, 1, 2, 3, 4, 5, 6}});

  snn.lg->log(ESSENTIAL, "Transfering data from Neurons to Log...");

  snn.generateCSV();

  snn.lg->log(ESSENTIAL, "Done writing, exiting");
  auto end = snn.lg->time();
  std::string msg = "Program took " + std::to_string(end - start) + "seconds ";
  snn.lg->log(ESSENTIAL, msg.c_str());
  snn.lg->printNetworkInfo();
  return 0;
}
