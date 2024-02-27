#include <iostream>
#include <pthread.h>

// volatile ensures that each modification of the variable reads and writes that
// variable in memory
volatile int counter = 0;
pthread_mutex_t mutex;
void *mutex_test(void *arg) {
  int i;
  for (i = 0; i < 5; i++) {
    pthread_mutex_lock(&mutex);
    counter++;
    std::cout << "Thread: " << (long)arg << " Counter: " << counter << '\n';
    pthread_mutex_unlock(&mutex);
  }
  pthread_exit(NULL);
}
int main() {
  pthread_mutex_init(&mutex, 0);
  pthread_t threads[5];
  for (long i = 0; i <= 4; i++) {
    pthread_create(&threads[i], 0, mutex_test, (void *)i);
  }
  for (int i = 0; i <= 4; i++) {
    pthread_join(threads[i], 0);
  }

  pthread_mutex_destroy(&mutex);
  return 0;
}
