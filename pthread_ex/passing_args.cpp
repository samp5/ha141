#include <iostream>
#include <pthread.h>

// all functions destine for threads must return void*
void *worker(void *arg) {
  std::cout << "This is worker thread #" << (long)arg << '\n';
  pthread_exit(NULL);
}
int main() {
  // array of threads
  pthread_t workers[5];

  long id;
  for (id = 1; id <= 5; id++) {
    pthread_create(&(workers[id]), NULL, &worker, (void *)id);
  }
  return 0;
}
