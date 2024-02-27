#include <iostream>
#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t condition2 = PTHREAD_COND_INITIALIZER;

bool activate_hello = false;
bool activate_world = false;

void *hello(void *arg) {
  pthread_mutex_lock(&mutex);

  while (!activate_hello) {
    pthread_cond_wait(&condition1, &mutex);
  }

  pthread_mutex_unlock(&mutex);
  std::cout << "Hello\n";
  pthread_exit(NULL);
}

void *world(void *arg) {
  pthread_mutex_lock(&mutex);
  while (!activate_world) {
    pthread_cond_wait(&condition2, &mutex);
  }

  pthread_mutex_unlock(&mutex);
  std::cout << "world\n";
  pthread_exit(NULL);
}

int main() {
  pthread_t hello_id, world_id;

  pthread_create(&hello_id, NULL, hello, NULL);
  pthread_create(&world_id, NULL, world, NULL);

  int choice;
  int cond_count = 1;
  while (cond_count <= 2) {
    std::cout << "Which condition should be unlocked? (1|2) ";
    std::cin >> choice;

    if (choice == 1) {
      pthread_mutex_lock(&mutex);
      activate_hello = true;
      pthread_cond_signal(&condition1);
      pthread_mutex_unlock(&mutex);
      pthread_join(hello_id, NULL);
    } else {
      pthread_mutex_lock(&mutex);
      activate_world = true;
      pthread_cond_signal(&condition2);
      pthread_mutex_unlock(&mutex);
      pthread_join(world_id, NULL);
    }
    cond_count++;
  }

  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&condition1);

  return 0;
}
