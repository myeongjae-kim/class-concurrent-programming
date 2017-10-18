/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : rw_lock_table.h
 * Due date             : 2017-10-22
 * Compilation Standard : c++11 */

#ifndef RW_LOCK_TABLE_H
#define RW_LOCK_TABLE_H

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
  rw_lock_table (uint64_t R, uint64_t N);
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

  // Release acquired lock.
  // 1. Check whether a winner tid is same with me. If not, assert().
  // 2. Is lock status RW_READER_LOCK or RW_WRITER_LOCK?
  //  
  //  Dequeue from wait queues.
  //  Removed edge from wait_for_graph.
  //  Wake up sleeping thread who is waiting me.
  //   
  //   2-1. RW_READER_LOCK.
  //     2-1-1. Find my location.
  //     2-1-2. Remove from queue.
  //     2-1-3. If a writer is top of a queue, wake a writer up.
  //
  //
  //   2-2. RW_WRITER_LOCK.
  //
  //TODO
  bool unlock(uint64_t tid, uint64_t record_id);

  /***  When aborting a transaction
   * 1. Remove edges from the wait_for_graph.
   * 2. Dequeue from the record_wait_queues.
   *     (find the threads that is waiting this transaction)
   * 3. Wakeup threads that is waiting this transaction.
   * ***/

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


  bool is_deadlock_exist(uint64_t tid, std::vector<uint64_t>& cycle_member);
};

#endif
