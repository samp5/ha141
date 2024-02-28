#include "node.hpp"
#include <cstdlib>
#include <pthread.h>
#include <vector>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int value = 1;
using std::vector;

void random_neighbors(vector<Node *> nodes, int number_neighbors);
void print_node_values(vector<Node *> nodes);
int main() {
  int num_nodes = 3;

  vector<Node *> nodes(num_nodes);
  for (int i = 0; i < num_nodes; i++) {
    cout << "Node " << i + 1 << " added\n";
    Node *node = new Node(i + 1);
    nodes[i] = node;
  }

  // random_neighbors(nodes, 2);

  nodes[0]->add_neighbor(nodes[1], 3);
  nodes[0]->add_neighbor(nodes[2], 2);

  for (Node *node : nodes) {
    node->start_thread();
  }

  int activate;
  std::cout << "Activate? ";
  std::cin >> activate;
  if (activate) {
    nodes[0]->activate();
    pthread_cond_signal(nodes[0]->get_cond());
  }

  for (Node *node : nodes) {
    node->join_thread();
  }

  print_node_values(nodes);

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
    i++;
  }
}

void print_node_values(vector<Node *> nodes) {
  for (Node *node : nodes) {
    cout << "Node " << node->get_id() << " has an accumulated value of "
         << node->get_accumulated() << '\n';
  }
}
