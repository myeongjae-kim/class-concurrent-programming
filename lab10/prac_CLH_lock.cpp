#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define NUM_THREAD  8
#define NUM_WORK    1000000

int cnt_global;
int gap[128]; // to allocate cnt_global & object_tas in different cache line
int object_ttas;

bool** tail;

struct nodes_t {
  bool* my_node;
  bool* my_pred;
  char dummy[112];
};

nodes_t nodes[NUM_THREAD];

void lock(int* lock_object, long tid) {
  nodes[tid].my_node = (bool*)calloc(1, sizeof(*nodes[tid].my_node));
  *nodes[tid].my_node = true;
  nodes[tid].my_pred = __sync_lock_test_and_set(tail, nodes[tid].my_node);
  while(*nodes[tid].my_pred == true){pthread_yield();}

  /* free(nodes[tid].my_pred);
   * nodes[tid].my_pred = nullptr; */
}

void unlock(int* lock_object, long tid) {
  free(nodes[tid].my_pred);
  nodes[tid].my_pred = nullptr;
  *nodes[tid].my_node = false;
}

void* Work(void* args) {
  long tid = (long)args;
  for (int i = 0; i < NUM_WORK; ++i) {
    lock(&object_ttas, tid);
    cnt_global++;
    // printf("[tid:%ld] Increase! %d\n", tid, cnt_global);
    unlock(&object_ttas, tid);
  }
  return NULL;
}

int main(void)
{
  pthread_t threads[NUM_THREAD];
  tail = (bool**)calloc(1, sizeof(*tail));
  *tail = (bool*)calloc(1, sizeof(**tail));

  for (long i = 0; i < NUM_THREAD; ++i) {
    pthread_create(&threads[i], 0, Work, (void*)i);
  }
  for (int i = 0; i < NUM_THREAD; ++i) {
    pthread_join(threads[i], 0);
  }

  printf("cnt_global: %d\n", cnt_global);

  free(*tail);
  free(tail);
  return 0;
}
