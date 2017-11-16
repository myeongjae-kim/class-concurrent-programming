#include <stdio.h>
#include <pthread.h>

#define NUM_THREAD 10
#define NUM_INCREMENT 1000000

int cnt_global = 0;
bool flag[NUM_THREAD * NUM_INCREMENT];

void* ThreadFunc(void* arg) {
  long ticket = 0;
  for (int i = 0; i < NUM_INCREMENT; ++i) {
    ticket = __sync_fetch_and_add(&cnt_global, 1);
    flag[ticket] = true;
  }
  return NULL;
}

int main(void)
{
  pthread_t threads[NUM_THREAD];

  int i;
  // create threads
  for (i = 0; i < NUM_THREAD; ++i) {
    if (pthread_create(&threads[i], 0, ThreadFunc, NULL) < 0) {
      printf("pthread_create error!\n");
      return 0;
    }
  }

  // wait threads end
  for (i = 0; i < NUM_THREAD; ++i) {
    pthread_join(threads[i], NULL);
  }

  for (i = 0; i < NUM_THREAD * NUM_INCREMENT; ++i) {
    if (flag[i] == false) {
      printf("ERROR!!\n");
      break;
    }
  }
  if (i == NUM_THREAD * NUM_INCREMENT) {
    printf("ALL FLAGS ON!\n");
  }
  printf("global count: %d\n", cnt_global);

  return 0;
}
