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
#include <cstring>

#include <pthread.h>

#include "main.h"
#include "rw_lock_table.h"
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
rw_lock_table *lock_table;

// for deadlock situation. Transaction killing order
uint64_t global_timestamp = 0; 

// this value is used as a commid_id.
uint64_t global_execution_order = 0; 

pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;

//TODO: commit and rollback implementation. how??


void initialize_global_variables();
void deallocate_global_variables();
void rw_lock_table_test();


int main(const int argc, const char * const argv[])
{
  // Disable IO sync with libc.
  // This makes huge speed up.
  std::ios::sync_with_stdio(false);

  if (argc != 4) {
    std::cout << "** This program requires three additional\
      command line arguments **" << std::endl;
    std::cout << "Usage: "<< argv[0]
      <<" [thread_num] [record_num] [max_execution_num]" << std::endl;
    return EXIT_FAILURE;
  }

  // setting arguments
  N = strtol(argv[1], NULL, 0) + 1; // # of threads. + 1 for not using zero.
  R = strtol(argv[2], NULL, 0); // # of records
  E = strtol(argv[3], NULL, 0); // max # of commit_id

  if (N == 0 || R == 0 || E == 0) {
    std::cout << "** Command line arguments\
      should be number except zero. **" << std::endl;
    return EXIT_FAILURE;
  }

#ifdef DBG
  std::cout << "Hello, world!" << std::endl;
  std::cout << "thread_num: " << N - 1 << std::endl;
  std::cout << "record: " << R << std::endl;
  std::cout << "max_execution_num: " << E << std::endl;
#endif

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


  lock_table = new rw_lock_table(R, N, threads_timestamp);

}

void deallocate_global_variables() {
  // Free allocated memories
  
  delete lock_table;

  free(threads_abort_flag);
  free(threads_timestamp);
  free(records);
  free(threads);

  for (uint64_t i = 0; i < N; ++i) {
    pthread_cond_destroy(&cond_var[i]);
  }
  free(cond_var);
}




/* Below are transaction samples
 * Only transacion related codes are exist below. */

/* void read_record(log_t &log) {
 *   std::cout << "[tid: " << log.tid << ", record_id: " << log.i << "] ";
 *   std::cout << "reader lock acquire try"<< std::endl;
 *
 *   if(lock_table->rdlock(log.tid, log.i, &global_mutex, *log.cycle_member) == false) {
 *     //deadlock found
 *     std::cout << "[tid: " << log.tid << ", record_id: " << log.i << "] ";
 *     std::cout << "(read_record) the deadlock is found" << std::endl;
 *     lock_table->print_deadlock(*log.cycle_member);
 *
 *     exit(1);
 *   }
 *
 *   std::cout << "[tid: " << log.tid << ", record_id: " << log.i << "] ";
 *   std::cout << "reader lock acquired" << std::endl;
 * }
 *
 * void first_write_record(log_t &log) {
 *   std::cout << "[tid: " << log.tid << ", record_id: " << log.j << "] ";
 *   std::cout << "first writer lock acquire try" << std::endl;
 *
 *   if(lock_table->wrlock(log.tid, FIRST_WRITE, log.j, &global_mutex,
 *         *log.cycle_member)
 *       == false) {
 *     //deadlock found
 *     std::cout << "[tid: " << log.tid << ", record_id: " << log.j << "] ";
 *     std::cout <<"(first_write_record) the deadlock is found" << std::endl;
 *     lock_table->print_deadlock(*log.cycle_member);
 *     exit(1);
 *   }
 *
 *   std::cout << "[tid: " << log.tid << ", record_id: " << log.j << "] ";
 *   std::cout << "first writer lock acquired" << std::endl;
 *
 * }
 *
 * void second_write_record(log_t &log) {
 *   std::cout << "[tid: " << log.tid << ", record_id: " << log.k << "] ";
 *   std::cout << "second writer lock acquire try" << std::endl;
 *
 *   if(lock_table->wrlock(log.tid, SECOND_WRITE, log.k, &global_mutex,
 *         *log.cycle_member)
 *       == false){
 *     //deadlock found
 *     std::cout << "[tid: " << log.tid << ", record_id: " << log.k << "] ";
 *     std::cout << "(second_write_record) the deadlock is found" << std::endl;
 *     lock_table->print_deadlock(*log.cycle_member);
 *     exit(1);
 *   }
 *
 *   std::cout << "[tid: " << log.tid << ", record_id: " << log.k << "] ";
 *   std::cout << "second writer lock acquired" << std::endl;
 *
 * }
 *
 * void commit() {
 *
 * }
 *
 * void* transaction_sample(void* arg) {
 *   uint64_t tid = uint64_t(arg);
 *
 *   uint64_t i, j, k;
 *
 *   std::vector<uint64_t> cycle_member;
 *
 *   while(1) {
 *     // This is used to select which one is to be aborted.
 *     threads_timestamp[tid] = ++global_timestamp;
 *
 *     // log
 *     log_t log;
 *     memset(&log, 0, sizeof(log_t));
 *     log.cycle_member = &cycle_member;
 *
 *     log.tid = tid;
 *     log.i = i = rand() % R;
 *
 *     do{
 *       log.j = j = rand() % R;
 *     }while (i == j);
 *
 *     do{
 *       log.k = k = rand() % R;
 *     }while (k == i || k == j);
 *
 *
 *     pthread_mutex_lock(&global_mutex);
 *     read_record(log);
 *     pthread_mutex_unlock(&global_mutex);
 *
 *     for (int i = 0; i < 10; ++i) {
 *       pthread_yield();
 *       // do something
 *     }
 *
 *     pthread_mutex_lock(&global_mutex);
 *     first_write_record(log);
 *     pthread_mutex_unlock(&global_mutex);
 *
 *     for (int i = 0; i < 10; ++i) {
 *       pthread_yield();
 *       // do something
 *     }
 *
 *     pthread_mutex_lock(&global_mutex);
 *     second_write_record(log);
 *     pthread_mutex_unlock(&global_mutex);
 *
 *     for (int i = 0; i < 10; ++i) {
 *       pthread_yield();
 *       // do something
 *     }
 *
 *     commit();
 *
 *     pthread_mutex_lock(&global_mutex);
 *     std::cout << "[tid: " << tid << ", record_id: " << i << "] ";
 *     std::cout << "reader lock release try" << std::endl;
 *
 *     lock_table->unlock(tid, i, cycle_member);
 *
 *     std::cout << "[tid: " << tid << ", record_id: " << i << "] ";
 *     std::cout << "reader lock released." << std::endl;
 *
 *     std::cout << "[tid: " << tid << ", record_id: " << j << "] ";
 *     std::cout << "first writer lock release try " << std::endl;
 *
 *     lock_table->unlock(tid, j, cycle_member);
 *
 *     std::cout << "[tid: " << tid << ", record_id: " << j << "] ";
 *     std::cout << "first writer lock released." << std::endl;
 *
 *     std::cout << "[tid: " << tid << ", record_id: " << k << "] ";
 *     std::cout << "second writer lock release try " << std::endl;
 *
 *     lock_table->unlock(tid, k, cycle_member);
 *
 *     std::cout << "[tid: " << tid << ", record_id: " << k << "] ";
 *     std::cout << "second writer lock released." << std::endl;
 *
 *     pthread_mutex_unlock(&global_mutex);
 *   }
 *
 *   return nullptr;
 * } */

