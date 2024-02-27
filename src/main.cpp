#include "node.hpp"
#include <pthread.h>
#include <vector>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int value = 1;
using std::vector;

int main() {
  int num_nodes = 3;

  vector<bool> activation(num_nodes, false);

  vector<Node *> nodes(num_nodes);
  for (int i = 0; i < num_nodes; i++) {
    cout << "Node " << i + 1 << " added\n";
    Node *node = new Node(i + 1);
    nodes[i] = node;
  }

  nodes[0]->add_neighbor(nodes[1], 2);
  cout << "Node 2 added as neighbor to Node 1\n";
  nodes[1]->add_neighbor(nodes[2], 3);
  cout << "Node 3 added as neighbor to Node 2\n";

  nodes[0]->activate();

  for (Node *node : nodes) {
    node->start_thread();
  }

  for (Node *node : nodes) {
    node->join_thread();
  }

  cout << value;

  for (Node *node : nodes) {
    delete node;
  }

  pthread_mutex_destroy(&mutex);
  return 0;
}
