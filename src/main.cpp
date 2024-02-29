#include "functions.hpp"
#include "neuron.hpp"
#include <pthread.h>
#include <unistd.h>

#define RAND_SEED 100123
#define NUMBER_NODES 3
#define NUMBER_EDGES 2

using std::cin;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t barrier;
volatile double value = 0;
bool finish = false;

int main() {

  srand(RAND_SEED);

  int num_neurons = NUMBER_NODES;

  pthread_barrier_init(&barrier, NULL, NUMBER_NODES + 1);

  vector<Neuron *> neurons(num_neurons);
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
    cout << "Activate neuron ( or [-1] to quit )\n";
    for (Neuron *neuron : neurons) {
      cout << " Neuron " << neuron->get_id() << '\n';
    }
    cout << "Input: ";
    cin >> activate;
    if (activate == -1) {

      pthread_mutex_lock(&mutex);
      finish = true;
      for (Neuron *neuron : neurons) {
        neuron->activate();
        pthread_cond_signal(neuron->get_cond());
      }
      pthread_mutex_unlock(&mutex);
    } else if (activate <= num_neurons && activate >= 0) {
      neurons[activate - 1]->activate();
      pthread_cond_signal(neurons[activate - 1]->get_cond());
    }
  }

  for (Neuron *node : neurons) {
    node->join_thread();
  }

  print_node_values(neurons);

  for (Neuron *node : neurons) {
    delete node;
  }

  pthread_mutex_destroy(&mutex);
  pthread_barrier_destroy(&barrier);
  return 0;
}
