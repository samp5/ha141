#include "functions.hpp"
#include "log.hpp"
#include "neuron.hpp"
#include <pthread.h>
#include <unistd.h>

#define RAND_SEED time(0)
// #define RAND_SEED 1

#define NUMBER_NODES 6
#define NUMBER_EDGES 5

using std::cin;

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
LogLevel level = INFO;

int main() {

  srand(RAND_SEED);

  int num_neurons = NUMBER_NODES;

  // pthread_barrier_init(&barrier, NULL, NUMBER_NODES + 1);

  vector<Neuron *> neurons(num_neurons);

  cout << "Time format is |HH:MM:SS:mircroseconds|\n";

  cout << "\nAdding Neurons\n";
  cout << "----------------\n\n";

  for (int i = 0; i < num_neurons; i++) {
    Neuron *neuron = new Neuron(i + 1, get_inhibitory_status());
    neurons[i] = neuron;
  }

  random_neighbors(neurons, NUMBER_EDGES);

  for (Neuron *node : neurons) {
    node->start_thread();
  }

  int activate;
  while (!finish) {

    usleep(100000);
    cout << "\nActivate neuron ( or [-1] to quit )\n";
    for (Neuron *neuron : neurons) {
      cout << " Neuron " << neuron->get_id() << '\n';
    }
    cout << "Input: ";
    cin >> activate;
    cout << '\n';
    if (activate == -1) {
      cout << "Exiting...\n";
      for (Neuron *neuron : neurons) {
        finish = true;
        neuron->activate();
        pthread_cond_t *this_cond = neuron->get_cond();
        pthread_cond_signal(this_cond);
      }
    } else if (activate <= num_neurons && activate >= 0) {
      neurons[activate - 1]->activate();
      pthread_cond_signal(neurons[activate - 1]->get_cond());
    } else {
      continue;
    }
  }

  for (Neuron *node : neurons) {
    pthread_join(node->get_thread_id(), NULL);
  }

  print_node_values(neurons);

  cout << "\nDeallocation\n";
  cout << "------------\n\n";
  for (Neuron *neuron : neurons) {
    lg.log_neuron_state(INFO, "Deleting Neuron %d", neuron->get_id());
    delete neuron;
  }

  lg.write_data();
  pthread_mutex_destroy(&mutex);
  // pthread_barrier_destroy(&barrier);

  return 0;
}
