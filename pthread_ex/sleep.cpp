#include <iostream>
#include <map>
#include <pthread.h>
#include <unistd.h>
#include <vector>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

using graph = std::map<pthread_t, std::vector<pthread_t>>;

using std::cout;

void *run_thread(void *arg);
void wait(pthread_t thread_id);

int main() {
  pthread_t threads[4];
  for (long i = 0; i < 4; i++) {
    pthread_create(&(threads[i]), NULL, run_thread, NULL);
  }
  for (long i = 0; i < 4; i++) {
    pthread_join(threads[i], NULL);
  }

  pthread_mutex_destroy(&mutex);
  pthread_exit(NULL);
}

void *run_thread(void *arg) {
  pthread_t id = pthread_self();

  pthread_mutex_lock(&mutex);
  wait(id);
  pthread_mutex_unlock(&mutex);

  pthread_exit(NULL);
}
void wait(pthread_t thread_id) {
  cout << "Thread " << (long)thread_id << " is sleeping \n";
  usleep(1000000);
  cout << "Thread " << (long)thread_id << "  finished sleeping\n";
}
