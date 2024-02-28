#include "node.hpp"

class Node;

Node::~Node() { pthread_cond_destroy(&cond); }

void Node::add_neighbor(Node *neighbor, double weight) {
  cout << "Edge from Node " << id << " to Node " << neighbor->get_id()
       << " added\n";
  neighbors[neighbor] = weight;
}

void *Node::run() {

  pthread_mutex_lock(&mutex);
  while (!active) {
    cout << "Node " << id << " is waiting\n";
    pthread_cond_wait(&cond, &mutex);
  }
  pthread_mutex_unlock(&mutex);

  pthread_mutex_lock(&mutex);
  accumulated = value;
  cout << "Node " << id << " is activated, accumulated set to " << accumulated
       << "\n";
  recieved = true;
  pthread_cond_broadcast(&cond);

  cout << "Node " << id << " is running\n";

  pthread_mutex_unlock(&mutex);
  if (neighbors.empty()) {
    cout << "Node " << id << " does not have any neigbors!\n";
  }

  for (const auto &pair : neighbors) {

    cout << "Node " << id << " is sending a message to Node " << pair.first->id
         << '\n';

    pthread_mutex_lock(&mutex);

    double message = accumulated * neighbors[pair.first];
    cout << "Accumulated value for Node " << id << " is " << accumulated
         << '\n';
    cout << "Weight for Node " << id << " to Node " << pair.first->id << " is "
         << pair.second << '\n';
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
  // active = false;
  // this->run();
  pthread_exit(NULL);
}

void Node::start_thread() {
  pthread_create(&thread, NULL, Node::thread_helper, this);
}

void Node::join_thread() { pthread_join(thread, NULL); }
