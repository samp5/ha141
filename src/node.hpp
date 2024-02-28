#include <iostream>
#include <map>
#include <pthread.h>
#include <vector>

using std::cout;
extern pthread_mutex_t mutex;
extern volatile int value;

class Node {
private:
  double accumulated = 4;
  int id;
  std::map<Node *, double> neighbors;
  pthread_t thread;
  pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
  bool active = false;
  bool recieved = false;

public:
  Node(int _id) : id(_id) {}
  ~Node();
  void add_neighbor(Node *neighbor, double weight);
  void *run();
  void start_thread();
  void join_thread();

  void activate() { active = true; }
  void deactivate() { active = false; }

  //>>>>>>>>>>>>>> Access to private variables <<<<<<<<<<<
  pthread_cond_t *get_cond() { return &cond; }
  int get_id() { return id; }
  double get_accumulated() { return accumulated; }

  /*--------------------------------------------------------------*\
   *                  Thread helper:
   *    POSIX needs a void* (*)(void*) function signature
   *    This function allows us to use the run() member funciton
  \--------------------------------------------------------------*/
  static void *thread_helper(void *instance) {
    return ((Node *)instance)->run();
  }
};
