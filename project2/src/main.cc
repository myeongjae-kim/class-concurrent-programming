/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : main.cc
 * Due date             : 2017-10-22
 * Compilation Standard : c++11 */


#include <iostream>
#include <queue>

#include <cstdint>
#include <cassert>

#include <pthread.h>
#include "main.h"

/* global variables */

uint64_t N; // number of threads
uint64_t R; // number of records
uint64_t E; // maximum number of executions

pthread_t *threads;
log_t* thread_logs;
int64_t *records;

std::queue<wait_q_elem_t> *record_wait_queues;
// TODO: wait-for graph implementation


uint64_t timestamp = 0; // for deadlock situation. transaction killing order
uint64_t global_execution_order = 0;

pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_rwlock_t rw_lock;

//TODO: commit and rollback implementation. how??

int main(const int argc, const char * const argv[])
{
  // Disable IO sync with libc.
  // This makes huge speed up.
  std::ios::sync_with_stdio(false);

  if (argc != 4) {
    std::cout << "** This program requires three additional command line arguments **" << std::endl;
    std::cout << "Usage: "<< argv[0] <<" [thread_num] [record_num] [max_execution_num]" << std::endl;
    return EXIT_FAILURE;
  }

  // setting arguments
  N = strtol(argv[1], NULL, 0);
  R = strtol(argv[2], NULL, 0);
  E = strtol(argv[3], NULL, 0);

  if (N == 0 || R == 0 || E == 0) {
    std::cout << "** Command line arguments should be number except zero. **" << std::endl;
    return EXIT_FAILURE;
  }

#ifdef DBG
  std::cout << "Hello, world!" << std::endl;
  std::cout << "thread_num: " << N << std::endl;
  std::cout << "record: " << R << std::endl;
  std::cout << "max_execution_num: " << E << std::endl;
#endif

  // lock initialization
  if (pthread_rwlock_init(&rw_lock, NULL) != 0) {
    std::cout << "(main) rwlock_init is failed." << std::endl;
    return EXIT_FAILURE;
  }

  // memory allocation
  threads = (pthread_t*)malloc(N * sizeof(*threads));
  assert(threads != nullptr);
  thread_logs = (log_t*)malloc(N * sizeof(*thread_logs));
  assert(thread_logs != nullptr);

  records = (int64_t*)malloc(R * sizeof(*records));
  assert(records != nullptr);

  record_wait_queues = new std::queue<wait_q_elem_t>[R];
  assert(record_wait_queues != nullptr);






  // Free allocated memories
  pthread_rwlock_destroy(&rw_lock);
  
  delete[] record_wait_queues;

  free(records);
  free(thread_logs);
  free(threads);
  return 0;
}
