#include "neuron.hpp"
#include "functions.hpp"
#include <pthread.h>
#include <unistd.h>

Neuron::Neuron(int _id, int _excit_inhib_value) {
  id = _id;
  excit_inhib_value = _excit_inhib_value;
  const char *inhib =
      _excit_inhib_value == -1 ? "excitatory\0" : "inhibitory\0";
  print_time();
  cout << "Neuron " << id << " added (" << inhib << " type) \n";
}

Neuron::~Neuron() { pthread_cond_destroy(&cond); }

void Neuron::add_neighbor(Neuron *neighbor, double weight) {
  print_time();
  cout << "Edge from Neuron " << id << " to Neuron " << neighbor->get_id()
       << " added\n";
  _postsynaptic[neighbor] = weight;
  Neuron *this_neuron = this;
  neighbor->add_previous(this_neuron, weight);
}

void Neuron::add_previous(Neuron *neighbor, double weight) {
  _presynaptic[neighbor] = weight;
  print_time();
  cout << "Neuron " << neighbor->id << " added to _presynaptic of Neuron " << id
       << '\n';
}

void *Neuron::run() {

  pthread_mutex_lock(&mutex);
  while (!active) {
    print_time();
    cout << "Neuron " << id << " is waiting\n";
    pthread_cond_wait(&cond, &mutex);
  }

  if (finish) {
    pthread_exit(NULL);
  }

  pthread_mutex_unlock(&mutex);

  pthread_mutex_lock(&mutex);
  membrane_potential = value == INITIAL_MEMBRANE_POTENTIAL
                           ? membrane_potential
                           : membrane_potential + value;
  value = INITIAL_MEMBRANE_POTENTIAL;
  print_time();
  cout << "Neuron " << id << " is activated, accumulated equal to "
       << membrane_potential << "\n";
  recieved = true;
  pthread_cond_broadcast(&cond);

  pthread_mutex_unlock(&mutex);

  if (_postsynaptic.empty()) {

    print_time();
    cout << "Neuron " << id << " does not have any neigbors!\n";
  } else if (membrane_potential < ACTIVATION_THRESHOLD) {
    print_time();
    cout << "Membrane potential for Neuron " << id
         << " is below the threshold, not firing\n";
  } else {
    for (const auto &pair : _postsynaptic) {
      if (membrane_potential < ACTIVATION_THRESHOLD) {
        print_time();
        cout << "Membrane potential for Neuron " << id
             << " is below the threshold, not firing\n";
        break;
      }

      print_time();
      cout << "Neuron " << id << " is sending a message to Neuron "
           << pair.first->id << '\n';

      pthread_mutex_lock(&mutex);

      double message =
          membrane_potential * _postsynaptic[pair.first] * excit_inhib_value;
      print_time();
      cout << "Accumulated value for Neuron " << id << " is "
           << membrane_potential << '\n';
      print_time();
      cout << "Weight for Neuron " << id << " to Neuron " << pair.first->id
           << " is " << pair.second << '\n';
      print_time();
      cout << "Neuron " << id << " modifier is " << excit_inhib_value << '\n';
      print_time();
      cout << "Message is " << message << '\n';

      value = message;

      // activate neighbor
      pair.first->activate();
      // get condition
      pthread_cond_t *neighbor_con = pair.first->get_cond();
      // signal start
      pthread_cond_signal(neighbor_con);
      pthread_mutex_unlock(&mutex);

      pthread_mutex_lock(&mutex);
      while (!pair.first->recieved) {
        pthread_cond_wait(neighbor_con, &mutex);
      }
      pthread_mutex_unlock(&mutex);
    }
    print_time();
    cout << "Neuron " << id << " fired, entering refractory phase\n";
    this->refractory();
    print_time();
    cout << "Neuron " << id << " completed refractory phase, running\n";
  }

  active = false;
  this->run();
  pthread_exit(NULL);
}

void Neuron::start_thread() {
  pthread_create(&thread, NULL, Neuron::thread_helper, this);
}

void Neuron::join_thread() { pthread_join(thread, NULL); }

void Neuron::refractory() {
  membrane_potential = -70;
  print_time();
  cout << "Neuron " << id << " potential set to " << membrane_potential << '\n';
  usleep(2000);
}
