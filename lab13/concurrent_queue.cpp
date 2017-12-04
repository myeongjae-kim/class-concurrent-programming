#include <stdio.h>
#include <inttypes.h>
#include <pthread.h>
#include <assert.h>

#define NUM_PRODUCER                4
#define NUM_CONSUMER                NUM_PRODUCER
#define NUM_THREADS                 (NUM_PRODUCER + NUM_CONSUMER)
#define NUM_ENQUEUE_PER_PRODUCER    10000000
#define NUM_DEQUEUE_PER_CONSUMER    NUM_ENQUEUE_PER_PRODUCER

bool flag_verification[NUM_PRODUCER * NUM_ENQUEUE_PER_PRODUCER];
void enqueue(int key);
int dequeue();

// -------- Queue with coarse-grained locking --------
// ------------------- Modify here -------------------
#define QUEUE_SIZE      1024

struct QueueNode {
  int key;
  int flag;
};

QueueNode queue[QUEUE_SIZE];
uint64_t front;
char padding[128];
uint64_t rear;

pthread_mutex_t mutex_for_queue = PTHREAD_MUTEX_INITIALIZER;

void init_queue(void) {
  front = 0;
  rear = 0;
}

void enqueue(int key) {
  int ticket = __sync_fetch_and_add(&rear, 1);
  int idx = ticket % QUEUE_SIZE;
  int round = ticket / QUEUE_SIZE;

  // while ((queue[idx].flag & 1) == 1// data exists
  //       || (queue[idx].flag / 2) != round) {
  //   pthread_yield();
  // }
  // 오류: flag값이 바뀌는 경우가 있다.
  //
  // [ticket 1] [ticket 5]
  //
  //      [flag = 1]
  //
  // 이 상황에서 ticket 5의 행동에 문제가 있다.
  // while의 2줄 사이에서 context switch가 일어나면
  // 데이터가 들어있는데 덮어씌운다던가, 데이터가 없는데 빼가는 문제가 생김.
  // 아래처럼 flag값을 읽어와서 들고있으면 문제 생기지 않는다.

  while (1){
    int flag = queue[idx].flag;

    if ((flag & 1) == 1// data exists
        || (flag / 2) != round){  // or not my turn 
      // printf("[ticket:%d, key:%d] yield\n", ticket, key);
      pthread_yield();
    } else {
      break;
    }
  }

  queue[idx].key = key;
  // __sync_synchronize();
  queue[idx].flag++; // it does not need to be an atmoic operation
}

int dequeue(void) {
  int ticket = __sync_fetch_and_add(&front, 1);
  int idx = ticket % QUEUE_SIZE;
  int round = ticket / QUEUE_SIZE;


  while (1){
    int flag = queue[idx].flag;

    if ((flag & 1) == 0// data exists
        || (flag / 2) != round){  // or not my turn 
      // printf("[ticket:%d] yield\n", ticket);
      pthread_yield();
    } else {
      break;
    }
  }

  int ret_key = queue[idx].key;
  queue[idx].key = 0;
  // __sync_synchronize();
  queue[idx].flag++; // it does not need to be an atmoic operation

  return ret_key;
}
// ------------------------------------------------

void* ProducerFunc(void* arg) {
  long tid = (long)arg;

  int key_enqueue = NUM_ENQUEUE_PER_PRODUCER * tid;
  for (int i = 0; i < NUM_ENQUEUE_PER_PRODUCER; i++) {
    enqueue(key_enqueue);
    key_enqueue++;
  }

  return NULL;
}

void* ConsumerFunc(void* arg) {
  for (int i = 0; i < NUM_DEQUEUE_PER_CONSUMER; i++) {
    int key_dequeue = dequeue();
    flag_verification[key_dequeue] = true;
  }

  return NULL;
}

int main(void) {
  pthread_t threads[NUM_THREADS];

  init_queue();

  for (long i = 0; i < NUM_THREADS; i++) {
    if (i < NUM_PRODUCER) {
      pthread_create(&threads[i], 0, ProducerFunc, (void**)i);
    } else {
      pthread_create(&threads[i], 0, ConsumerFunc, NULL);
    }
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  // verify
  for (int i = 0; i < NUM_PRODUCER * NUM_ENQUEUE_PER_PRODUCER; i++) {
    if (flag_verification[i] == false) {
      printf("INCORRECT!\n");
      return 0;
    }
  }
  printf("CORRECT!\n");

  return 0;
}

