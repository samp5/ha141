#include "globals.hpp"
void Mutex::destroy_mutexes() {
  pthread_mutex_destroy(&potential);
  pthread_mutex_destroy(&log);
  pthread_mutex_destroy(&message);
  pthread_mutex_destroy(&activation);
  pthread_mutex_destroy(&stimulus);
}
