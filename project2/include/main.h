/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : main.h
 * Due date             : 2017-10-22
 * Compilation Standard : c++11 */

#ifndef __MAIN_H__
#define __MAIN_H__
#include <vector>
#include <cstdint>

// #define DBG

// Every transaction can get a four phases.
enum phase_t {INVALID, READ, FIRST_WRITE, SECOND_WRITE, COMMIT};

// In the wait queue, needed information is thread id and current phase
typedef struct _wait_q_elem {
  uint64_t tid;
  
  phase_t current_phase;
} wait_q_elem_t;


// Log. Do you need more explanation?
typedef struct _log {
  uint64_t tid;

  // This is a pointer of a vector that is used to detect cycle and save
  // cycle member threads. Every thread gets a cycle_member vector.
  // There are many times to call deadlock detection function in rw_lock_table
  // object. I would not like to call the constructor of vector whenever
  // I use the vector in the object, so I made it as a pointer.
  std::vector<uint64_t> *cycle_member;

  phase_t current_phase; // for rollback. How many did you write?
  bool is_done; // 'true' whan a transaction is done and before commit.
  uint64_t commit_id;
  uint64_t i; // record idx
  uint64_t j;
  uint64_t k;
  long long value_of_i; // value of record
  long long value_of_j;
  long long value_of_k;
} log_t;



#endif
