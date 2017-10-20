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


  // unlock().
  // This function releases acquired lock.
  // 1. Is lock status RW_READER_LOCK or RW_WRITER_LOCK?
  //  
  //  What we have to do is below:
  //   - Dequeue from wait queues.
  //   - Removed edge from wait_for_graph.
  //   - Wake up sleeping thread who is waiting me.
  //
  // 2-1. RW_READER_LOCK.
  //   2-1-1. Find my location.
  //    2-1-1-1. If any writer is exist in front of me, it is an error.
  //    2-1-1-2. Case1. I am top of a queue
  //      2-1-1-2-1. If I am alone in queue, change lock status to RW_UNLOCK
  //                and dequeue myself.
  //      2-1-1-2-2. If a writer follows, remove edge from the writer to me
  //                , wake the writer up, and dequeue myself.
  //                Chagne lock status to RW_UNLOCK
  //      2-1-1-2-3. If a reader follows, do not change lock status, and
  //                dequeue myself.
  //
  //    2-1-1-3. Case2. I am not the top of a queue.
  //                   It means that in front of me there is at least
  //                   one reader.
  //      2-1-1-3-1. No follower, just dequeue myself.
  //                Do not change lock status. It is still RW_READER_LOCK
  //
  //      2-1-1-3-2. If a writer follows, remove edge from the writer to me
  //                , add an edge from the writer to ahead reader, and
  //                dequeue myself.
  //
  //                Do not change lock status. It is still RW_READER_LOCK
  //
  //      2-1-1-3-3. If a reader follows, do not change lock status, and
  //                dequeue myself.
  //
  //                Do not change lock status. It is still RW_READER_LOCK
  //
  // 2-2. RW_WRITER_LOCK.
  //   2-2-1. Find my location. I should be a top of a queue.
  //         If I am not a top, it means an error is occurred.
  //   2-2-2. If any reader or writer follows me, remove edge from it to me
  //         , wake it up, and dequeue myself.
  //         Change lock status to RW_UNLOCK
  //   2-2-2. If I am alone, just dequeue myself.
  //         Change lock status to RW_UNLOCK
  bool unlock(uint64_t tid, uint64_t record_id,
      std::vector<uint64_t> &cycle_member);


  // This function is a wrapper of wait_for_graph->print_cycle()
  void print_deadlock(std::vector<uint64_t> &cycle_member);

  /***  When aborting a transaction
   * 1. Remove edges from the wait_for_graph.
   * 2. Dequeue from the record_wait_queues.
   *     (find the threads that is waiting this transaction)
   * 3. Wakeup threads that is waiting this transaction.
   * ***/
  void clear_failed_rdlock(uint64_t tid, uint64_t record_id,
      std::vector<uint64_t> &cycle_member);
  void clear_failed_wrlock(uint64_t tid, uint64_t record_id,
      std::vector<uint64_t> &cycle_member);

private:
  // It is an array of R elements
  rw_lock_status_t *table;
  uint32_t *readers_count;

  // It is an array of R vectors
  std::vector<wait_q_elem_t> *record_wait_queues;

  // It is an array of N elements
  pthread_cond_t *cond_var;
  bool *threads_abort_flag;

  // It is a graph that has N node.
  directed_graph *wait_for_graph;

  uint64_t *threads_timestamp;


  // Return true when acquiring is successful.
  // Return false when a deadlock exists.
  // More information is in rw_lock_table.h
  bool is_deadlock_exist(uint64_t tid, std::vector<uint64_t>& cycle_member);

  // Subfunctions of unlock();
  bool rd_unlock(uint64_t tid, uint64_t record_id,
      std::vector<uint64_t> &cycle_member);
  bool wr_unlock(uint64_t tid, uint64_t record_id);

  uint64_t get_newest_tid(std::vector<uint64_t> &cycle_member);

  bool is_myself_deadlock_victim(uint64_t tid, 
      std::vector<uint64_t> &cycle_member);
};

#endif
