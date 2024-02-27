#include <iostream>
#include <map>
#include <pthread.h>
#include <vector>

using std::cout;
extern pthread_mutex_t mutex;
extern int value;

class Node {
private:
  double accumulated = 4;
  int id;
  std::map<Node *, double> neighbors;
  pthread_t thread;
  pthread_cond_t cond;
  bool active = false;

public:
  Node(int _id) : id(_id) {}
  ~Node() { pthread_cond_destroy(&cond); }

  void add_neighbor(Node *neighbor, double weight) {
    neighbors[neighbor] = weight;
  }

  void *run() {

    pthread_mutex_lock(&mutex);
    while (!active) {
      cout << "Node " << id << " is waiting\n";
      pthread_cond_wait(&cond, &mutex);
    }

    accumulated = value;
    cout << "Node " << id << " is activated, setting accumulated to " << value
         << "\n";

    cout << "Node " << id << " is running\n";
    for (const auto &pair : neighbors) {
      cout << "Node " << id << " is sending a message to Node" << pair.first->id
           << '\n';

      double message = accumulated * neighbors[pair.first];
      cout << "Accumulated value for Node " << id << " is " << accumulated
           << '\n';
      cout << "Weight for Node " << id << " to Node " << pair.first->id
           << " is " << pair.second << '\n';
      cout << "Message is " << message << '\n';

      pthread_mutex_unlock(&mutex);
      value = message;
      pthread_mutex_unlock(&mutex);

      pair.first->activate();

      pthread_cond_t *neighbor_con = pair.first->get_cond();

      pthread_cond_signal(neighbor_con);
    }
    pthread_mutex_unlock(&mutex);
    return 0;
  }

  void start_thread() {
    pthread_create(&thread, NULL, Node::thread_helper, this);
  }

  void join_thread() { pthread_join(thread, NULL); }

  pthread_cond_t *get_cond() { return &cond; }

  void activate() { active = true; }
  void deactivate() { active = false; }

  // POSIX needs a void* (*)(void*) function signature
  // This function allows us to use the run() member funciton
  static void *thread_helper(void *instance) {
    return ((Node *)instance)->run();
  }
};
