
#include "functions.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include "neuron_group.hpp"
#include <pthread.h>
#include <unistd.h>

// #define RAND_SEED time(0)
#define RAND_SEED 1

#define NUMBER_NODES 8
#define NUMBER_EDGES 1
#define NUMBER_GROUPS 2

using std::cout;

ostream &STREAM = cout;
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
LogLevel level = DEBUG2;

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

  vector<Message *> messages =
      construct_message_vector_from_file(neuron_groups, "./input_files/test");

  for (auto message : messages) {
    print_message(message);
  }

  Message *test_message = new Message;
  test_message->message = 10;
  test_message->target_neuron_group = neuron_groups.front();
  test_message->timestamp = 1000;
  test_message->target_neuron =
      neuron_groups.front()->get_neruon_vector().front();

  Message *test_message2 = new Message;
  test_message2->message = 10;
  test_message2->target_neuron_group = neuron_groups.front();
  test_message2->timestamp = 1000;
  test_message2->target_neuron =
      neuron_groups.front()->get_neruon_vector().back();

  neuron_groups.front()->add_to_intragroup(test_message);
  neuron_groups.front()->add_to_intragroup(test_message2);

  neuron_groups.front()->get_neruon_vector().front()->activate();
  neuron_groups.front()->get_neruon_vector().back()->activate();

  for (auto group : neuron_groups) {
    group->start_thread();
  }

  for (auto group : neuron_groups) {
    pthread_join(group->get_thread_id(), NULL);
  }

  for (auto group : neuron_groups) {
    group->print_group();
  }

  for (auto group : neuron_groups) {
    delete group;
  }

  return 0;
}
