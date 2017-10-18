#include <cstring>
#include <cassert>

#include "rw_lock_table.h"

// Allocate memories
rw_lock_table::rw_lock_table(uint64_t R, uint64_t N){

  table = (rw_lock_status_t*)malloc(R * sizeof(*table));
  assert(table != NULL);
  memset(table, RW_UNLOCK, N * sizeof(*table));

  readers_count = (uint32_t*)calloc(R ,sizeof(*readers_count));
  assert(readers_count != NULL);

  record_wait_queues = new std::vector<wait_q_elem_t>[R];
  assert(record_wait_queues != NULL);

  cond_var = (pthread_cond_t*)malloc(N * sizeof(*cond_var));
  assert(cond_var != NULL);
  for (uint64_t i = 0; i < N; ++i) {
    cond_var[i] = PTHREAD_COND_INITIALIZER;
  }

  wait_for_graph = new directed_graph(N);
  assert(wait_for_graph != NULL);
}

rw_lock_table::~rw_lock_table() {
  delete wait_for_graph;
  free(cond_var);
  delete[] record_wait_queues;
  free(readers_count);
  free(table);
}


// Return true when acquiring is successful.
// Return false when a deadlock exists.
// More information is in rw_lock_table.h
bool rw_lock_table::rdlock(uint64_t tid, uint64_t record_id,
 pthread_mutex_t *global_mutex, std::vector<uint64_t> &cycle_member) {
  // 1. Insert tid to record_wait_queues
  // 2. Check whether waiting is needed or not (Writer exist?)
  //  2-1. Waiting
  //    2-1-1. Add edge to wait_for graph.
  //    2-1-2. Do deadlock detection.
  //    2-1-3. If deadlock is found, add deadlock members to cycle_member
  //          and return false.
  //    2-1-4. If deadlock is not found, do conditional wait.
  //    2-1-5. Waken up! Check whether current status of queue is okay or not.
  //  2-2. Not waiting. Go to get a lock
  //  2-3. Write status to lock table and increase readers_count


  // 1. Insert tid to record_wait_queues
  record_wait_queues[record_id].push_back( wait_q_elem_t({tid, READ}) );


  // 2. Check whether waiting is needed or not.
  //
  // If record_wait_queues[record_id] has one element,
  // Go to get a lock
  if (record_wait_queues[record_id].size() == 1) {
    // Do nothing.
    // Go to get a lock.
  } else {
    // The # of queue elements should be at least 1.
    assert(record_wait_queues[record_id].size() > 1);


    // Check whether a writer is exists.
    uint64_t ahead_writer_tid = 0;

    // Skip first one. The first one is myself.
    for (auto r_iter = record_wait_queues[record_id].rbegin() + 1;
        r_iter != record_wait_queues[record_id].rend(); ++r_iter) {

      assert(r_iter->current_phase != INVALID);

      if (r_iter->current_phase != READ) {
        // writer found.
        ahead_writer_tid = r_iter->tid;
        break;
      }
    }

    // If ahead_writer_tid == 0, there is no writer in front of this thread.
    if (ahead_writer_tid != 0) {
      // Writer is found
      //    2-1-1. Add edge to wait_for graph.
      //    2-1-2. Do deadlock detection.
      //    2-1-3. If deadlock is found, add deadlock members to cycle_member
      //          and return false.
      //    2-1-4. If deadlock is not found, do conditional wait.

      //    2-1-1. Add edge to wait_for graph.
      wait_for_graph->add_edge(tid, ahead_writer_tid);

      //    2-1-2. Do deadlock detection.

      if (is_deadlock_exist(tid, cycle_member)) {
        // Deadlock is found
        // 2-1-3. If deadlock is found, add deadlock members to cycle_member
        //       and return false.
        //
        // Deadlock members are in cycle_member.
        // is_deadlock_exist() did the work.
        return false;
      } else {
        // 2-1-4. If deadlock is not found, do conditional wait.
      }

      // Wait
      std::cout << "[tid: " << tid << "]\
        (rdlock) Go to sleep" << std::endl;

      pthread_cond_wait(&cond_var[tid], global_mutex);

      // 2-1-5. Waken up! Check whether current status of queue is okay or not.
      // If there is a writer in front of me,
      // stop the program. This is an error.

#ifdef DBG
      for (auto wait_q_elem : record_wait_queues[record_id]) {
        if (wait_q_elem.tid == tid && wait_q_elem.current_phase == READ) {
          // Normal case
          break;
        } else if (wait_q_elem.current_phase != READ) {
          // Impossible case.
          // There should be no writer in front of me.
          //
          // Is this possible? If possible, sleep again.
          assert(false);
        } else {
          // Impossible case.
          // Current thread is not in the queue. Where am I?
          assert(false);
        }
      }
#endif

    } else {
      // Writer is not found.
      // 2-2. Not waiting. Go to get a lock
    }
  }

#ifdef DBG
  if (readers_count[record_id] == 0 && table[record_id] == RW_READER_LOCK) {
    // Impossible case.
    // When a table status is RW_READER_LOCK,
    // readers_count should be same or more than one.
    assert(false);
  }
#endif
  //  2-3. Write status to lock table and increase readers_count
  readers_count[record_id]++;
  table[record_id] = RW_READER_LOCK;


  // Read lock is acquired.
  return true;
}

// Return true when acquiring is successful.
// Return false when a deadlock exists.
// More information is in rw_lock_table.h
bool rw_lock_table::wrlock(uint64_t tid, phase_t phase, uint64_t record_id,
    pthread_mutex_t *global_mutex, std::vector<uint64_t> &cycle_member) {
  // 1. Insert tid to record_wait_queues
  // 2. Check whether waiting is needed or not (Am I the top of the queue?)
  //  2-1. Waiting
  //    2-1-1. Add edge to wait_for graph.
  //    2-1-2. Do deadlock detection.
  //    2-1-3. If deadlock is found, add deadlock members to cycle_member
  //          and return false.
  //    2-1-4. If deadlock is not found, do conditional wait.
  //    2-1-5. Waken up! Check whether current status of queue is okay or not.
  //  2-2. Not waiting. Go to get a lock
  //  2-3. Write status to lock table

  // 1. Insert tid to record_wait_queues
  assert(phase == FIRST_WRITE || phase == SECOND_WRITE);
  record_wait_queues[record_id].push_back( wait_q_elem_t({tid, phase}) );

  // 2. Check whether waiting is needed or not (Am I the top of the queue?)
  //
  // If I am the top of queue,
  // go to get a lock.
  if (record_wait_queues[record_id][0].tid == tid) {
    // Do nothing,
    // Go to get a lock.
  } else {
    // The # of queue elements should be at least 1.
    assert(record_wait_queues[record_id].size() > 1);

    // 2-1-1. Add edge to wait_for graph.
    // find a thread that is in front of me.
    uint64_t ahead_rw_tid = 0;
    for (auto r_iter = record_wait_queues[record_id].rbegin();
        r_iter != record_wait_queues[record_id].rend(); ++r_iter) {
      // I am found
      if (r_iter->tid == tid) {
        r_iter++;
        ahead_rw_tid = r_iter->tid;
        break;
      }
    }

    // tid should not be zero
    assert(ahead_rw_tid != 0);

    // Add edge to wait_for graph.
    wait_for_graph->add_edge(tid, ahead_rw_tid);

    //    2-1-2. Do deadlock detection.
    if (is_deadlock_exist(tid, cycle_member)) {
      // Deadlock is found
      // 2-1-3. If deadlock is found, add deadlock members to cycle_member
      //       and return false.
      //
      // Deadlock members are in cycle_member.
      // is_deadlock_exist() did the work.
      return false;
    } else {
      // 2-1-4. If deadlock is not found, do conditional wait.
    }

    // Wait
    std::cout << "[tid: " << tid << "]\
      (wrlock) Go to sleep" << std::endl;

    pthread_cond_wait(&cond_var[tid], global_mutex);

    // 2-1-5. Waken up! Check whether current status of queue is okay or not.
    // If the top of queue is me, it means that I acquired lock.
    // If I am not the top, go to sleep again. (Who wakes me up?)
    while(record_wait_queues[tid][0].tid != tid) {
      std::cout << "[tid: " << tid << "]\
        (wrlock) I am not the top of queue. Go to sleep again!" << std::endl;
      pthread_cond_wait(&cond_var[tid], global_mutex);
    }
  }

  //  2-2. Not waiting. Go to get a lock
  //  Error case
  assert(table[record_id] != RW_INVALID);
  
  // Lock status should not be a RW_WRITER_LOCK
  assert(table[record_id] != RW_WRITER_LOCK);

  // No reader should be exists.
  assert(readers_count[record_id] == 0);
  // lock acquired.

  //  2-3. Write status to lock table
  table[record_id] = RW_WRITER_LOCK;

  return true;
}


bool rw_lock_table::is_deadlock_exist(uint64_t tid,
    std::vector<uint64_t>& cycle_member) {

  cycle_member = wait_for_graph->get_cycle(tid);
  if (cycle_member.size() != 0) {
    // Deadlock is found
    // 2-1-3. If deadlock is found, add deadlock members to cycle_member
    //       and return false.
    return true;
  } else {
    // Deadlock is not found.
    return false;
  }

}


