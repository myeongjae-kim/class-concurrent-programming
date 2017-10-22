/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : rw_lock_table.h
 * Due date             : 2017-10-22
 * Compilation Standard : c++11 */

#ifndef __RW_LOCK_TABLE_H__
#define __RW_LOCK_TABLE_H__

#include <vector>

#include <pthread.h>

#include "main.h"
#include "directed_graph.h"

// every status gets one byte.
enum rw_lock_status_t : uint8_t {
  RW_INVALID,
  RW_UNLOCK,
  RW_READER_LOCK,
  RW_WRITER_LOCK
};

// Features:
// 1. Decide which thread is winner or loser.
//    - We need queues for ordering threads.
// 2. Make a thread sleep when it is not a winner.
//    - We need conditional variable.
// 3. Deadlock checking
//    - We need wait-for graph.
class rw_lock_table
{
public:
  // ** Warning: The global N in main is increased by one in the first of **
  // **                                    program, so here we use N - 1. **
  //
  // The record # is R, and the thread # is N - 1 (tid 0 is not used).
  // This is a table that has R records, and N - 1 threads access this table.
  //
  // The record id starts from 0 to R.
  // The thread id starts from 1 to N-1.
  rw_lock_table (uint64_t R, uint64_t N, uint64_t *threads_timestamp);
  virtual ~rw_lock_table ();

  // Return true when acquiring is successful.
  // Return false when a deadlock exists.
  // When a deadlock is detected, what we need to do is
  // checking who is the newest transaction in deadlock
  // and abort it. The deadlock members' tid is in cycle_number.
  // Finding which one is newest transaction is occurred out of this class.
  bool rdlock(uint64_t tid, uint64_t record_id,
   pthread_mutex_t *global_mutex, std::vector<uint64_t> &cycle_member);
  bool wrlock(uint64_t tid, phase_t phase, uint64_t record_id,
   pthread_mutex_t *global_mutex, std::vector<uint64_t> &cycle_member);


  // Detailed comment of this function is in rw_lock_table.cc
  bool unlock(uint64_t tid, uint64_t record_id,
      std::vector<uint64_t> &cycle_member);

  // It is just a wrapper of wait_for_graph->print_cycle;
  void print_deadlock(std::vector<uint64_t> &cycle_member);


private:
  // It is an array of R elements
  rw_lock_status_t *table;
  uint32_t *readers_count;

  // It is an array of R vectors
  std::vector<wait_q_elem_t> *record_wait_queues;

  // It is an array of N elements
  pthread_cond_t *cond_var;

  // It is a graph that has N node.
  directed_graph *wait_for_graph;

  // Return true when acquiring is successful.
  // Return false when a deadlock exists.
  // More information is in rw_lock_table.h
  bool is_deadlock_exist(uint64_t tid, std::vector<uint64_t>& cycle_member);

  // Subfunctions of unlock();
  bool rd_unlock(uint64_t tid, uint64_t record_id,
      std::vector<uint64_t> &cycle_member);
  bool wr_unlock(uint64_t tid, uint64_t record_id);

  // Abort functions
  void rdlock_clear_abort(uint64_t tid, uint64_t record_id);
  void wrlock_clear_abort(uint64_t tid, uint64_t record_id);

  // Functions to find reader or wrtier in ahead of me.
  uint64_t find_ahead_writer(uint64_t tid, uint64_t record_id);
  uint64_t find_ahead_reader_or_writer(uint64_t tid, uint64_t record_id);
};

#endif
