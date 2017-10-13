/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : transaction.cc
 * Due date             : 2017-10-22
 * Compilation Standard : c++11 */

#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <fstream>

#include <pthread.h>

#include "transaction.h"

extern uint64_t E; // execution limit
extern uint64_t R; // number of records
extern uint64_t global_execution_order;

extern pthread_mutex_t global_mutex;
extern pthread_rwlock_t *rw_lock;

extern int64_t *records;

void file_writing_test(std::ofstream &log_file);

void* transaction(void* arg) {
  uint64_t tid = (uint64_t)arg;

#ifdef TRX_DBG
  std::cout << "(thread) Hello, I am thread #" << tid << std::endl;
#endif

  // make file.
  std::string file_name = "thread" + std::to_string(tid + 1) + ".txt";
    
  // TODO: how about using a memory buffer to collect results?
  std::ofstream log_file(file_name);

#ifdef TRX_DBG
  // test file writing
  file_writing_test(log_file);
  log_file.close();
  return nullptr;
#endif

  while (global_execution_order <= E) {
    uint64_t i,j,k;

    // each variable gets a unique record.
    i = rand() % R;
    j = rand() % R;

    while (i == j) {
      j = rand() % R;
    }

    while (i == k || j == k) {
      k = rand() % R;
    }




    // For read record i
    pthread_mutex_lock(&global_mutex);

    //TODO: If it needs waiting, do deadlock checking
    pthread_rwlock_rdlock(&rw_lock[i]);
    pthread_mutex_unlock(&global_mutex);

    // read i
    uint64_t value_of_i = records[i];

    // For write j
    pthread_mutex_lock(&global_mutex);

    //TODO: If it needs waiting, do deadlock checking
    pthread_rwlock_wrlock(&rw_lock[j]);
    pthread_mutex_unlock(&global_mutex);

    // write j
    records[j] += value_of_i + 1;


    // For write k
    pthread_mutex_lock(&global_mutex);

    //TODO: If it needs waiting, do deadlock checking
    pthread_rwlock_wrlock(&rw_lock[k]);
    pthread_mutex_unlock(&global_mutex);

    // write k
    records[k] -= value_of_i;


    //TODO: Commit!


    pthread_mutex_lock(&global_mutex);
    pthread_rwlock_unlock(&rw_lock[i]);
    pthread_rwlock_unlock(&rw_lock[j]);
    pthread_rwlock_unlock(&rw_lock[k]);

    uint64_t commit_id = ++global_execution_order;
    if (commit_id > E) {
      //TODO: Undo

      pthread_mutex_unlock(&global_mutex);
      break;
    }


    //commit log append.
    //[commit_id] [i] [j] [k] [value of record i] [value of record j] [value of record k]
    log_file << commit_id << " " << i << " " << j << " " << k << " "
      << records[i] << " " << records[j] << " " << records[k] << std::endl;

    pthread_mutex_unlock(&global_mutex);

  }

  log_file.close();
  return nullptr;
}



void file_writing_test(std::ofstream &log_file) {
  uint64_t commit_id_temp;
  while (global_execution_order <= E) {
    pthread_mutex_lock(&global_mutex);
    commit_id_temp = ++global_execution_order;
    pthread_mutex_unlock(&global_mutex);

    if (commit_id_temp > E) {
      break;
    }
    log_file << commit_id_temp << ": File writing test" << std::endl;
    pthread_yield();
  }
}
