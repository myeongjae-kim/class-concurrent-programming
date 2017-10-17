/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : main.cc
 * Due date             : 2017-10-22
 * Compilation Standard : c++11 */


#include <iostream>
#include <queue>

#include <cstdint>
#include <cassert>
#include <cstdlib>

#include <pthread.h>

#include "main.h"
#include "transaction.h"
#include "directed_graph.h"

/* global variables */

uint64_t N; // number of threads
uint64_t R; // number of records
uint64_t E; // maximum number of executions

/* Below variables will be an array of N elements. */
// We do not use thread whose number is zero.
// If thread ID is zero, it is an invalid thread.
pthread_t *threads; // N elements

// each thread gets a conditional variable.
pthread_cond_t *cond_var; // N elements

// Which transaction will be aborted is decided by this array.
uint64_t *threads_timestamp;  // N elements

// A transaction will execute the rollback process
// if a value of below array is Zero.
bool *threads_abort_flag; // N elements


/* Below variables will be an array of R elements. */
int64_t *records;
std::vector<wait_q_elem_t> *record_wait_queues;

// graph for detecting deadlock
directed_graph *wait_for_graph;

uint64_t global_timestamp = 0; // for deadlock situation. transaction killing order
uint64_t global_execution_order = 0; // this value is used as a commid_id.

pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;

// each record gets a conditional variable.
pthread_rwlock_t *rw_lock;


//TODO: commit and rollback implementation. how??


void initialize_global_variables();
void deallocate_global_variables();

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

  N++;// + 1 for not using zero.

  initialize_global_variables();
  srand(time(NULL));
  // tid starts from one.
  for (uint64_t i = 1; i < N; i++) {
    if (pthread_create(&threads[i], 0, transaction, (void*)(i) ) < 0) {
      std::cout << "(main) thread creation has been failed." << std::endl;
      deallocate_global_variables();
      return 1;
    }
  }


  for (uint64_t i = 1; i < N; i++) {
    pthread_join(threads[i], NULL);
  }



  deallocate_global_variables();
  return 0;
}

void initialize_global_variables() {
  // lock initialization
  
  rw_lock = (pthread_rwlock_t*)malloc(R * sizeof(*rw_lock));
  assert(rw_lock != nullptr);

  for (uint64_t i = 0; i < R; ++i) {
    rw_lock[i] = PTHREAD_RWLOCK_INITIALIZER;
  }

  cond_var = (pthread_cond_t*)malloc((N) * sizeof(*cond_var));
  assert(cond_var != nullptr);

  for (uint64_t i = 0; i < (N); ++i) {
    cond_var[i] = PTHREAD_COND_INITIALIZER;
  }

  // memory allocation
  threads = (pthread_t*)malloc(N * sizeof(*threads));
  assert(threads != nullptr);

  records = (int64_t*)malloc(R * sizeof(*records));
  assert(records != nullptr);

  threads_timestamp = (uint64_t*)malloc((N) * sizeof(*threads_timestamp));
  assert(threads_timestamp != nullptr);

  threads_abort_flag = (bool*)calloc((N), sizeof(*threads_abort_flag));
  assert(threads_abort_flag != nullptr);

  for (uint64_t i = 0; i < R; ++i) {
    records[i] = 100;
  }


  record_wait_queues = new std::vector<wait_q_elem_t>[R];
  assert(record_wait_queues != nullptr);

  wait_for_graph = new directed_graph(N);
}

void deallocate_global_variables() {
  // Free allocated memories
  
  delete wait_for_graph;
  delete[] record_wait_queues;

  free(threads_abort_flag);
  free(threads_timestamp);
  free(records);
  free(threads);

  for (uint64_t i = 0; i < N; ++i) {
    pthread_cond_destroy(&cond_var[i]);
  }
  for (uint64_t i = 0; i < R; ++i) {
    pthread_rwlock_destroy(&rw_lock[i]);
  }
  free(cond_var);
  free(rw_lock);
}