
#include "functions.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include "neuron_group.hpp"
#include <pthread.h>
#include <unistd.h>

#define RAND_SEED time(0)

#define NUMBER_NODES 6
#define NUMBER_EDGES 5
#define NUMBER_GROUPS 2

using std::cin;
using std::cout;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;
volatile double value = 0;
bool finish = false;
Log lg;

/*
  1 - ERROR,
  2 - WARNING,
  3 - INFO,
  4 - DEBUG,
*/
LogLevel level = DEBUG;

int main() {
  srand(RAND_SEED);

  vector<NeuronGroup *> neuron_groups(NUMBER_GROUPS);

  // will potentially leave a remainder
  int neuron_per_group = NUMBER_NODES / NUMBER_GROUPS;

  // reamainder is guearenteed to be less than the number of groups
  int remainder = NUMBER_NODES % NUMBER_GROUPS;

  cout << "\nAdding Neurons\n";
  cout << "----------------\n\n";

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

  for (auto group : neuron_groups) {
    delete group;
  }

  return 0;
}
