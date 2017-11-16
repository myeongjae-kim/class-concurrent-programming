#include <stdio.h>
#include <pthread.h>

#define NUM_THREAD  8
#define NUM_WORK    1000000

int cnt_global;
int gap[128]; // to allocate cnt_global & object_tas in different cache line
int object_tas;

void lock(int* lock_object) {
  while (__sync_lock_test_and_set(lock_object, 1) == 1) {}
}

void unlock(int* lock_object) {
  *lock_object = 0;
  __sync_synchronize();
}

void* Work(void* args) {
  for (int i = 0; i < NUM_WORK; ++i) {
    lock(&object_tas);
    cnt_global++;
    unlock(&object_tas);
  }
  return NULL;
}

int main(void)
{
  pthread_t threads[NUM_THREAD];

  for (long i = 0; i < NUM_THREAD; ++i) {
    pthread_create(&threads[i], 0, Work, 0);
  }
  for (int i = 0; i < NUM_THREAD; ++i) {
    pthread_join(threads[i], 0);
  }

  printf("cnt_global: %d\n", cnt_global);
  return 0;
}
