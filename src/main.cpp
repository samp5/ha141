#include "node.hpp"
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main() {
  Node *node1 = new Node(1);
  Node *node2 = new Node(2);
  Node *node3 = new Node(3);

  node1->add_neighbor(node2);
  node2->add_neighbor(node3);

  node1->start_thread();
  node2->start_thread();
  node3->start_thread();

  node1->join_thread();
  node2->join_thread();
  node3->join_thread();

  delete node1;
  delete node2;
  delete node3;

  pthread_mutex_destroy(&mutex);
  return 0;
}
