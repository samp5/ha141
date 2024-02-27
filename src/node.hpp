#include <iostream>
#include <pthread.h>
#include <vector>

using std::cout;
extern pthread_mutex_t mutex;

class Node {
private:
  double accumulated = 0.0;
  int id;
  std::vector<Node *> neighbors;
  pthread_t thread;

public:
  Node(int _id) : id(_id) {}

  void add_neighbor(Node *neighbor) { neighbors.push_back(neighbor); }

  void *run() {
    pthread_mutex_lock(&mutex);
    cout << "Node " << id << " is running\n";
    for (Node *neighbor : neighbors) {
      cout << "Node " << id << " is sending a message to Node" << neighbor->id
           << '\n';
    }
    pthread_mutex_unlock(&mutex);
    return 0;
  }

  void start_thread() {
    pthread_create(&thread, NULL, Node::thread_helper, this);
  }

  void join_thread() { pthread_join(thread, NULL); }

  // POSIX needs a void* (*)(void*) function signature
  // This function allows us to use the run() member funciton
  static void *thread_helper(void *instance) {
    return ((Node *)instance)->run();
  }
};
