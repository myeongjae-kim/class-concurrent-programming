/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : main.h
 * Due date             : 2017-10-22
 * Compilation Standard : c++11 */

#ifndef MAIN_H
#define MAIN_H

#define DBG

enum phase_t {INVALID, READ, FIRST_WRITE, SECOND_WRITE};

typedef struct _log {
  // phase_t done_phase; // for rollback. How many did you write?
  // bool is_done; // 'true' whan a transaction is done and before commit.
  uint64_t commit_id;
  uint64_t i; // record idx
  uint64_t j;
  uint64_t k;
  int64_t value_of_i; // value of record
  int64_t value_of_j;
  int64_t value_of_k;
} log_t;


typedef struct _wait_q_elem {
  uint64_t tid;
  
  phase_t current_phase;
} wait_q_elem_t;

#endif
