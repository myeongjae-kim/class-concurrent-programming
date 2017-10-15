/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : transaction.cc
 * Due date             : 2017-10-22
 * Compilation Standard : c++11 */

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <pthread.h>

#include "main.h"
#include "transaction.h"
#include "directed_graph.h"

extern uint64_t E; // execution limit
extern uint64_t R; // number of records
extern uint64_t global_execution_order;

extern int64_t *records;

extern std::vector<wait_q_elem_t> *record_wait_queues;
extern directed_graph *wait_for_graph;

extern pthread_mutex_t global_mutex;
extern pthread_rwlock_t *rw_lock;
extern pthread_cond_t *cond_var;
extern uint64_t *threads_timestamp;
extern bool *threads_abort_flag;

extern uint64_t global_timestamp;


void file_writing_test(std::ofstream &log_file);
std::pair<bool, uint64_t> is_deadlock_exist(uint64_t from);
void rollback(log_t &log);
void abort_target_transaction(uint64_t target_tid);

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
  /* // test file writing
   * file_writing_test(log_file);
   * log_file.close();
   * return nullptr; */
#endif

  while (global_execution_order <= E) {
    uint64_t i,j,k;
    log_t log;
    // log initializing.
    memset(&log, 0, sizeof(log));

    // flag initializing.
    threads_abort_flag[tid] = false;

    // each variable gets a unique record.
    i = rand() % R;
    j = rand() % R;

    while (i == j) {
      j = rand() % R;
    }

    k = rand() % R;
    while (i == k || j == k) {
      k = rand() % R;
    }

    log.i = i;
    log.j = j;
    log.k = k;


    // For read record i
    pthread_mutex_lock(&global_mutex);
    log.current_phase = READ;
    // Get a timestamp to ordering threads.
    // This value is used when a deadlock is occur.
    // Newest transaction (largest timestampe) will be aborted.
    threads_timestamp[tid] = ++global_timestamp;

    //If it needs waiting, do deadlock checking

    // Insert to queue
    record_wait_queues[i].push_back( wait_q_elem_t({tid, READ}) );
    log.i_queue_location = record_wait_queues[i].end() - 1;

    // Check whether writer lock exist in front of this thread.
    // Iterated wait queue from back.

    bool writer_exist = false;
    uint64_t ahead_writer_tid;

    // Find ahead writer and do deadlock checking
    // rbegin() + 1 for skip the element we pushed.
    for (auto iter = record_wait_queues[i].rbegin() + 1; iter != record_wait_queues[i].rend(); ++iter) {
      // If writer is found, add edge to wait_for graph and sleep
      if (iter->current_phase != READ) {
        writer_exist = true;

        // add edge to wait_for graph
        ahead_writer_tid = iter->tid;
        wait_for_graph->add_edge(tid, ahead_writer_tid);

        // deadlock checking
        auto &&deadlock_exist_and_newest_tid = is_deadlock_exist(tid);
        if (deadlock_exist_and_newest_tid.first) {
          // deadlock exist!

          // When I am the newest one
          if (tid == deadlock_exist_and_newest_tid.second) {
            // Abort current transaction.
            threads_abort_flag[tid] = true;
            break;
          } else {
            // Abort newest transaction.
            // Newest transaction is in sleep, so abort checking
            // should be occurred right after a thread is waken up.
            abort_target_transaction(deadlock_exist_and_newest_tid.second);
          }
        }
        
        // deadlock is not exist.

        // If an aborted transaction is the one we waited for,
        // Go to get a read lock
        if (deadlock_exist_and_newest_tid.first && 
            deadlock_exist_and_newest_tid.second == ahead_writer_tid) {

          // TODO: Is this possible case? What we waited for is
          // newer than us??
          std::cout << "(transaction) Impossible case is occurred." << std::endl;
          break;
        }

        //go to sleep
        pthread_cond_wait(&cond_var[tid], &global_mutex);

        // waken up! get rw_lock
        break;
      }
    }

    // Abort this transaction if a deadlock is occurred
    // and current transaction is the newest one.
    if (threads_abort_flag[tid]) {
      // Start of rollbacking process
      // Remove graph edge.
      if (writer_exist) {
        wait_for_graph->remove_edge(tid, ahead_writer_tid);
        writer_exist = false;
      }
      
      rollback(log);
      // Rollback is end.
      pthread_mutex_unlock(&global_mutex);
      continue;
    }
    
    // If ahead writer is not exist, get read lock.
    pthread_rwlock_rdlock(&rw_lock[i]);


    // remove element from wait_for graph
    if (writer_exist) {
      wait_for_graph->remove_edge(tid, ahead_writer_tid);
      writer_exist = false;
    }

    pthread_mutex_unlock(&global_mutex);

    // read i
    uint64_t value_of_i = records[i];
    log.value_of_i = value_of_i;

    // For write j
    pthread_mutex_lock(&global_mutex);
    log.current_phase = FIRST_WRITE;
    
    // Insert to wait queue
    record_wait_queues[j].push_back( wait_q_elem_t({tid, FIRST_WRITE}) );
    log.j_queue_location = record_wait_queues[j].end() - 1;

    // Check whether reader or writer exist in front of this thread.
    // Iterated wait queue from back.
    bool reader_or_writer_exist = false;
    uint64_t ahead_rw_tid;

    // If any reader or writer is exist in front of current thread,
    // current thread cannot get a lock.
    // It means that a queue size is not 1,
    // the current thread cannot get a lock.
    if (record_wait_queues[j].size() != 1) {
      // ahead thread is exist.
      reader_or_writer_exist = true;

      ahead_rw_tid = (record_wait_queues[j].end() - 2)->tid;
      wait_for_graph->add_edge(tid, ahead_rw_tid);


      // deadlock checking
      auto &&deadlock_exist_and_newest_tid = is_deadlock_exist(tid);
      if (deadlock_exist_and_newest_tid.first) {
        // deadlock exist!

        // When I am the newest one
        if (tid == deadlock_exist_and_newest_tid.second) {
          // Abort current transaction.
          threads_abort_flag[tid] = true;
          break;
        } else {
          // Abort newest transaction.
          // Newest transaction is in sleep, so abort checking
          // should be occurred right after a thread is waken up.
          abort_target_transaction(deadlock_exist_and_newest_tid.second);
        }
      }

      // deadlock is not exist.

      // If an aborted transaction is the one we waited for,
      // Go to get a read lock
      if (deadlock_exist_and_newest_tid.first &&
          deadlock_exist_and_newest_tid.second == ahead_rw_tid) {
        // Do not sleep. Go to get a lock.
        // TODO: Is this possible case? What we waited for is
        // newer than us??

        std::cout << "(transaction) Impossible case is occurred." << std::endl;
      } else {
        // wait ahead thread
        pthread_cond_wait(&cond_var[tid], &global_mutex);
        // waken up! get rw_lock
      }
    }

    // Abort this transaction if a deadlock is occurred
    // and current transaction is the newest one.
    if (threads_abort_flag[tid]) {
      // Start of rollbacking process
      // Remove graph edge.
      
      // remove element from wait_for graph
      if (reader_or_writer_exist) {
        wait_for_graph->remove_edge(tid, ahead_rw_tid);
        reader_or_writer_exist = false;
      }



      rollback(log);
      // Rollbacking end
      pthread_mutex_unlock(&global_mutex);
      continue;
    }


    // If ahead writer is not exist, get read lock.
    pthread_rwlock_wrlock(&rw_lock[j]);

    // remove element from wait_for graph
    if (reader_or_writer_exist) {
      wait_for_graph->remove_edge(tid, ahead_rw_tid);
      reader_or_writer_exist = false;
    }

    pthread_mutex_unlock(&global_mutex);

    // write j
    records[j] += value_of_i + 1;
    log.value_of_j = records[j];


    // For write k
    pthread_mutex_lock(&global_mutex);
    log.current_phase = SECOND_WRITE;

    // Insert to wait queue
    record_wait_queues[k].push_back( wait_q_elem_t({tid, SECOND_WRITE}) );
    log.k_queue_location = record_wait_queues[k].end() - 1;

    // Check whether reader or writer exist in front of this thread.
    // Iterated wait queue from back.

    // reinitialization.
    reader_or_writer_exist = false;
    ahead_rw_tid = 0;

    // If any reader or writer is exist in front of current thread,
    // current thread cannot get a lock.
    // It means that a queue size is not 1,
    // the current thread cannot get a lock.

    if (record_wait_queues[k].size() != 1) {
      // ahead thread is exist.
      reader_or_writer_exist = true;

      ahead_rw_tid = (record_wait_queues[k].end() - 2)->tid;
      wait_for_graph->add_edge(tid, ahead_rw_tid);

      // deadlock checking
      auto &&deadlock_exist_and_newest_tid = is_deadlock_exist(tid);
      if (deadlock_exist_and_newest_tid.first) {
        // deadlock exist!

        // When I am the newest one
        if (tid == deadlock_exist_and_newest_tid.second) {
          // Abort current transaction.
          threads_abort_flag[tid] = true;
          break;
        } else {
          // Abort newest transaction.
          // Newest transaction is in sleep, so abort checking
          // should be occurred right after a thread is waken up.
          abort_target_transaction(deadlock_exist_and_newest_tid.second);
        }
      }

      // deadlock is not exist.

      // If an aborted transaction is the one we waited for,
      // Go to get a read lock
      if (deadlock_exist_and_newest_tid.first &&
          deadlock_exist_and_newest_tid.second == ahead_rw_tid) {
        // Do not sleep. Go to get a lock.
        // TODO: Is this possible case? What we waited for is
        // newer than us??

        std::cout << "(transaction) Impossible case is occurred." << std::endl;
      } else {
        // wait ahead thread
        pthread_cond_wait(&cond_var[tid], &global_mutex);
        // waken up! get rw_lock
      }
    }

    // Abort this transaction if a deadlock is occurred
    // and current transaction is the newest one.
    if (threads_abort_flag[tid]) {
      // Start of rollbacking process
      
      // remove element from wait_for graph
      if (reader_or_writer_exist) {
        wait_for_graph->remove_edge(tid, ahead_rw_tid);
        reader_or_writer_exist = false;
      }


      rollback(log);
      // Rollbacking is end.
      pthread_mutex_unlock(&global_mutex);
      continue;
    }

    // If ahead writer is not exist, get read lock.
    pthread_rwlock_wrlock(&rw_lock[k]);

    // remove element from wait_for graph
    if (reader_or_writer_exist) {
      wait_for_graph->remove_edge(tid, ahead_rw_tid);
      reader_or_writer_exist = false;
    }

    pthread_mutex_unlock(&global_mutex);

    // write k
    records[k] -= value_of_i;
    log.value_of_k = records[k];


    // Transaction is done.
    log.is_done = true;

    // Commit!
    pthread_mutex_lock(&global_mutex);
    log.current_phase = COMMIT;

    // TODO: Error check. Remove below code when it works well.
    if (record_wait_queues[i][0].tid != tid ||
        record_wait_queues[j][0].tid != tid ||
        record_wait_queues[k][0].tid != tid 
       ) {
      std::cout << "(transaction)ERROR! The top of queues is not current transaction." << std::endl;
      assert(false);
    }

    // Dequeue from all wait queues.
    record_wait_queues[i].erase(record_wait_queues[i].begin());
    record_wait_queues[j].erase(record_wait_queues[j].begin());
    record_wait_queues[k].erase(record_wait_queues[k].begin());
    
    // Remove edges from wait_for graph that pointing this transaction.
    if (record_wait_queues[i].size() != 0) {
      wait_for_graph->remove_edge(record_wait_queues[i][0].tid, tid);
    }
    
    if (record_wait_queues[j].size() != 0) {
      wait_for_graph->remove_edge(record_wait_queues[j][0].tid, tid);
    }

    if (record_wait_queues[k].size() != 0) {
      wait_for_graph->remove_edge(record_wait_queues[k][0].tid, tid);
    }

    // Release all rw_lock
    pthread_rwlock_unlock(&rw_lock[i]);
    pthread_rwlock_unlock(&rw_lock[j]);
    pthread_rwlock_unlock(&rw_lock[k]);

    uint64_t commit_id = ++global_execution_order;
    log.commit_id = commit_id;
    if (commit_id > E) {
      //TODO: Undo
      rollback(log);

      pthread_mutex_unlock(&global_mutex);
      break;
    }


    //commit log append.
    //[commit_id] [i] [j] [k] [value of record i] [value of record j] [value of record k]
    log_file << log.commit_id << " " << log.i << " " << log.j << " " << log.k << " "
      << log.value_of_i << " " << log.value_of_j << " " << log.value_of_k << std::endl;

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


std::pair<bool, uint64_t> is_deadlock_exist(uint64_t from) {
  std::pair<bool, uint64_t> exist_and_newest_tid(false, 0);

  // deadlock checking
  auto &&cycle_member = wait_for_graph->get_cycle(from);
  if (cycle_member.size() != 0) {
    // deadlock exist!
    exist_and_newest_tid.first = true;

    // find the newest transaction and abort it.
    std::sort(cycle_member.begin(), cycle_member.end(), [](uint64_t lhs_tid, uint64_t rhs_tid){
        return threads_timestamp[lhs_tid] > threads_timestamp[rhs_tid];
        });

    // Newest one is found.
    exist_and_newest_tid.second = cycle_member[0];
  }

  return exist_and_newest_tid;
}

void rollback(log_t &log) {
  // Edges of wait_for graph is already removed.



  // rollback case
  switch (log.current_phase) {
    case COMMIT:
      // TODO: just recovering values.
      records[log.k] += log.value_of_i;
      records[log.j] -= (log.value_of_i + 1);

      break;

    case SECOND_WRITE:
      // Every queue and graph edge has been cleaned.
      // What we need to do is just recovering records.
      

#ifdef TRX_DBG
      std::cout << "(rollback) Log is SECOND_WRITE phase." << std::endl;
#endif
      // TODO: clear queue. Unlock acquired locks.
      // Here is before acquring first write lock.
      record_wait_queues[log.i].erase(log.i_queue_location);
      record_wait_queues[log.j].erase(log.j_queue_location);
      record_wait_queues[log.k].erase(log.k_queue_location);
      pthread_rwlock_unlock(&rw_lock[log.i]);
      pthread_rwlock_unlock(&rw_lock[log.j]);

      records[log.j] -= (log.value_of_i + 1);
      break;

    case FIRST_WRITE:
#ifdef TRX_DBG
      std::cout << "(rollback) Log is FIRST_WRITE phase." << std::endl;
#endif
      // Recover value of j
      // TODO: clear queue. Unlock acquired locks.
      // Here is before acquring first write lock.

      record_wait_queues[log.i].erase(log.i_queue_location);
      record_wait_queues[log.j].erase(log.j_queue_location);
      pthread_rwlock_unlock(&rw_lock[log.i]);


      // write is not yet occurred.
      // There is no value to recover.
      break;

    case READ:
      // TODO: clear queue. Unlock acquired locks.
      // Here is before acquring read lock.
#ifdef TRX_DBG
      std::cout << "(rollback) Log is READ phase." << std::endl;
#endif
      record_wait_queues[log.i].erase(log.i_queue_location);


      // Remove from queue.

      break;
    case INVALID:
      std::cout << "(rollback) ERROR! Invalid log is inserted." << std::endl;
      assert(false);
      break;
    default:
      std::cout << "(rollback) ERROR! Deafult case is not exist." << std::endl;
      assert(false);
  }
}


// ** Precondition **
// This function is called while a global lock is locked.
void abort_target_transaction(uint64_t target_tid) {

  // Abort target transaction.
  // Target transaction is in sleep, so abort checking
  // should be occurred right after a thread is waken up.
  threads_abort_flag[target_tid] = true;

  // Wake the threads up and wait a transaction uitil it finishes its rollback.
  pthread_cond_signal(&cond_var[target_tid]);
  pthread_mutex_unlock(&global_mutex);

  // wait rollbacking
  while (threads_abort_flag[target_tid] == true) {
    pthread_yield();
  }

  pthread_mutex_lock(&global_mutex);

}
