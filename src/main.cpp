#include "node.hpp"
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <vector>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int value = 1;
using std::vector;

void random_neighbors(vector<Node *> nodes, int number_neighbors);
int main() {
  int num_nodes = 5;

  vector<Node *> nodes(num_nodes);
  for (int i = 0; i < num_nodes; i++) {
    cout << "Node " << i + 1 << " added\n";
    Node *node = new Node(i + 1);
    nodes[i] = node;
  }

  random_neighbors(nodes, 6);

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
void random_neighbors(vector<Node *> nodes, int number_neighbors) {
  cout << "Adding Random Neighbors\n";
  int size = nodes.size();
  int i = 0;
  while (i < number_neighbors) {
    int from = rand() % size;
    int to = rand() % size;
    if (from == to) {
      continue;
    }

    nodes[from]->add_neighbor(nodes[to], rand() % 5 + 1);
    cout << "Edge from Node " << from + 1 << " to Node " << to + 1
         << " added\n";
    i++;
  }
}
