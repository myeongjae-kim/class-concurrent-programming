/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : main.h
 * Due date             : 2017-10-22
 * Compilation Standard : c++11 */

#ifndef __MAIN_H__
#define __MAIN_H__
#include <cstdint>

#define DBG

enum phase_t {INVALID, READ, FIRST_WRITE, SECOND_WRITE, COMMIT};

typedef struct _wait_q_elem {
  uint64_t tid;
  
  phase_t current_phase;
} wait_q_elem_t;


typedef struct _log {
  uint64_t tid;
  std::vector<uint64_t> *cycle_member;

  phase_t current_phase; // for rollback. How many did you write?
  bool is_done; // 'true' whan a transaction is done and before commit.
  uint64_t commit_id;
  uint64_t i; // record idx
  uint64_t j;
  uint64_t k;
  int64_t value_of_i; // value of record
  int64_t value_of_j;
  int64_t value_of_k;
} log_t;



#endif
