#include <cstring>
#include <cassert>
#include <algorithm>

#include "rw_lock_table.h"

// Allocate memories
rw_lock_table::rw_lock_table(uint64_t R, uint64_t N,
    uint64_t *threads_timestamp){
  if (threads_timestamp == nullptr) {
    std::cout << "(rw_lock_table) threads_timestampe\
      should be an allocated heap memory" << std::endl;
    exit(1);
  }

  this->threads_timestamp = threads_timestamp;

  table = (rw_lock_status_t*)malloc(R * sizeof(*table));
  assert(table != NULL);
  memset(table, RW_UNLOCK, R * sizeof(*table));

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

  threads_abort_flag = (bool*)calloc(N, sizeof(*threads_abort_flag));
}

rw_lock_table::~rw_lock_table() {
  free(threads_abort_flag);
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
  //    2-1-3. If deadlock is found, add deadlock members to cycle_member.
  //      2-1-3-1. Find the newest thread among cycle_member.
  //      2-1-3-2. If I am the newest one, return false.
  //      2-1-3-3. If I am not, turn abort flag of victim on.
  //    2-1-4. Do conditional wait.
  //    2-1-5. Waken up! Check whether current status of queue is okay or not.
  //    2-1-6. If an abort flag is truned on,
  //          turn of the flag and return false.
  //    2-1-7. If a deadlock detected again, at least I am not the victim.
  //          Yield until a deadlock is removed.
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
      if (is_myself_deadlock_victim(tid, cycle_member)) {
        // I am the victim of deadlock. return false.
        // TODO: Clear queue and graph

        // I must be in the back of the queue.
        assert(record_wait_queues[record_id].rbegin()->tid == tid);
        return false;
      }

      //  2-1-4. Do conditional wait.
      std::cout << "[tid: " << tid << ", record_id: " << record_id
        << "] (rdlock) Go to sleep" << std::endl;

      pthread_cond_wait(&cond_var[tid], global_mutex);

      std::cout << "[tid: " << tid << ", record_id: " << record_id
       << "] (rdlock) Good morning!" << std::endl;

      //    2-1-6. If an abort flag is truned on,
      //          turn of the flag and return false.
      //
      // If I am a victim of deadlock problem,
      // abort myself.
      if (threads_abort_flag[tid]) {
        // turn off the flag (reinitialization)
        
        // Check a deadlock is really exists.
        assert(is_deadlock_exist(tid, cycle_member));
        

        std::cout << "[tid: " << tid << ", record_id: " << record_id
          << "] (rdlock) I am the victim of deadlock. Return false "
          << std::endl;

        // TODO: Clear queue and graph
        // Add adge from follow thread to ahead thread
        // when follower thread is waiting me.
        //
        // Another deadlock can be occurred.
        //
        // I could be in the middle of the queue or back of the queue.

        threads_abort_flag[tid] = false;
        return false;
      }

      // 2-1-7. If a deadlock detected again, at least I am not the victim.
      // yield until a deadlock is removed.
      while(is_deadlock_exist(tid, cycle_member)) {
        pthread_yield();
      }
      
      // assert(finally, deadlock is not exist!);
      assert(is_deadlock_exist(tid, cycle_member) == false);

      // 2-1-5. Waken up! Check whether current status of queue is okay or not.
      // If there is a writer in front of me,
      // stop the program. This is an error.

#ifdef DBG
      bool i_am_found = false;
      for (auto wait_q_elem : record_wait_queues[record_id]) {
        if (wait_q_elem.current_phase == READ) {
          if (wait_q_elem.tid == tid) {
            // normal case.
            i_am_found = true;
            break;
          }
          continue;
        } else {
          // Impossible case.
          // There should be no writer in front of me.
          //
          // Is this possible? If possible, sleep again.
          assert(false);
        }
      }
      assert(i_am_found == true);
#endif

    } else {
      // Writer is not found.
      // 2-2. Not waiting. Go to get a lock
    }
  }

#ifdef DBG
  /* if (readers_count[record_id] == 0 && table[record_id] == RW_READER_LOCK) {
   *   // Impossible case.
   *   // When a table status is RW_READER_LOCK,
   *   // readers_count should be same or more than one.
   *   //
   *   // Really impossible? Possible case!
   *   // If another reader is on top and released lock, it does not change
   *   // lock status because follow writer is exist.
   *   assert(false);
   * } */
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
    if (is_myself_deadlock_victim(tid, cycle_member)) {
      // I am the victim of deadlock. return false.
      // TODO: Clear queue and graph

      // I must be in the back of the queue.
      assert(record_wait_queues[record_id].rbegin()->tid == tid);
      return false;
    }

    // 2-1-4. If deadlock is not found, do conditional wait.
    std::cout << "[tid: " << tid << ", record_id: " << record_id
      << "] (wrlock) Go to sleep." << " write_phase:" << phase << std::endl;

    pthread_cond_wait(&cond_var[tid], global_mutex);

    std::cout << "[tid: " << tid << ", record_id: " << record_id
      << "] (wrlock) Good morning!"
      << " write_phase:" << phase << std::endl;

    //    2-1-6. If an abort flag is truned on,
    //          turn of the flag and return false.
    //
    // If I am a victim of deadlock problem,
    // abort myself.
    if (threads_abort_flag[tid]) {
      // turn off the flag (reinitialization)

      // Check a deadlock is really exists.
      assert(is_deadlock_exist(tid, cycle_member));

      std::cout << "[tid: " << tid << ", record_id: " << record_id
        << "] (rdlock) I am the victim of deadlock. Return false "
        << std::endl;

      // TODO: Clear queue and graph
      // Add adge from follow thread to ahead thread
      // when follower thread is waiting me.
      //
      // Another deadlock can be occurred.
      //
      // I could be in the middle of the queue or back of the queue.

      threads_abort_flag[tid] = false;
      return false;
    }

    // 2-1-7. If a deadlock detected again, at least I am not the victim.
    // yield until a deadlock is removed.
    while(is_deadlock_exist(tid, cycle_member)) {
      pthread_yield();
    }

    // assert(finally, deadlock is not exist!);
    assert(is_deadlock_exist(tid, cycle_member) == false);


    // 2-1-5. Waken up! Check whether current status of queue is okay or not.
    // If the top of queue is me, it means that I acquired lock.
    // If I am not the top, go to sleep again. (Who wakes me up?)
    while(record_wait_queues[record_id][0].tid != tid) {

      std::cout << "[tid: " << tid << ", record_id: " << record_id
        << "] (wrlock) I am not the top of queue. Go to sleep again!"
        << std::endl;

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

  // I am the top of queue.
  assert(record_wait_queues[record_id][0].tid == tid);


  // lock acquired.
  //  2-3. Write status to lock table
  table[record_id] = RW_WRITER_LOCK;

  return true;
}


//  This function finds deadlock. If deadlock is found, it returns true
// and deadlock members' tid is inserted to cycle_member
//
// If deadlock is not found, it returns false.
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



bool rw_lock_table::unlock(uint64_t tid, uint64_t record_id,
    std::vector<uint64_t> &cycle_member) {
  // 1. Is lock status RW_READER_LOCK or RW_WRITER_LOCK?
  if (table[record_id] == RW_READER_LOCK) {
    rd_unlock(tid, record_id, cycle_member);
  } else if (table[record_id] == RW_WRITER_LOCK) { 
    wr_unlock(tid, record_id);
  } else {
    std::cout << "(unlock) lock status: " << int(table[record_id])
      << ". Why did you call unlock?"<< std::endl;
    assert(false);
  }

  return true;
}


bool rw_lock_table::rd_unlock(uint64_t tid, uint64_t record_id,
    std::vector<uint64_t> &cycle_member) {
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

  // Readers count should be positive number.
  assert(readers_count[record_id] > 0);

  //   2-1-1. Find my location.
  auto iter = record_wait_queues[record_id].begin();
  for (;iter != record_wait_queues[record_id].end() ; iter++) {
    //    2-1-1-1. If any writer is exist in front of me, it is an error.
    assert(iter->current_phase == READ);
    if (iter->tid == tid) {
      // I am found
      break;
    }
  }

  // I am not found. Where am I?
  assert(iter != record_wait_queues[record_id].end());

  auto follower = iter + 1;
  // 2-1-1-2. Case1. I am top of a queue
  if (iter == record_wait_queues[record_id].begin()) {
    //  2-1-1-2-1. If I am alone in queue, change lock status to RW_UNLOCK
    //            and dequeue myself.
    //  2-1-1-2-2. If a writer follows, remove edge from the writer to me
    //            , wake the writer up, and dequeue myself.
    //            Do not change lock status. A following thread will change
    //            lock status to RW_WRITER_LOCK
    //  2-1-1-2-3. If a reader follows, do not change lock status, and
    //            dequeue myself.

    //  2-1-1-2-1. If I am alone in queue, change lock status to RW_UNLOCK
    //            and dequeue myself.
    if (follower == record_wait_queues[record_id].end()) {
      // Change lock status to RW_UNLOCK
      // Dequeue will be occurred in the end of this function.
      table[record_id] = RW_UNLOCK;
    } else if (follower->current_phase != READ) {
      //  2-1-1-2-2. If a writer follows, remove edge from the writer to me
      //            , wake the writer up, and dequeue myself.
      //            Chagne lock status to RW_UNLOCK

      // A writer found
      //
      // Remove edge from the writer to me,
      // wake the writer up, and dequeue myself.

      // Remove edge
      wait_for_graph->remove_edge(follower->tid, tid);
      // Change lock status to unlock
      table[record_id] = RW_UNLOCK;

      // Wake the writer up.
      pthread_cond_signal(&cond_var[follower->tid]);

      std::cout << "[tid: " << tid << ", record_id: " << record_id
        << "] (rd_unlock) wake up thread "
        << follower->tid << std::endl;

    } else {
      assert(follower->current_phase == READ);
      //  2-1-1-2-3. If a reader follows, do not change lock status, and
      //            dequeue myself.
    }
  } else {
    // 2-1-1-3. Case2. I am not the top of a queue.
    //                It means that in front of me there is at least
    //                one reader.


    // 2-1-1-3-1. No follower, just dequeue myself.
    //           Do not change lock status. It is still RW_READER_LOCK
    if (follower == record_wait_queues[record_id].end()) {
      // Just dequeue myself
    } else if (follower->current_phase != READ) {
      //  2-1-1-3-2. If a writer follows, remove edge from the writer to me
      // , add an edge from the writer to ahead reader, and
      // dequeue myself.
      //
      // Do not change lock status. It is still RW_READER_LOCK

      // A writer found
      //
      // Remove an edge from the writer to me,
      // Add an edge from the writer to ahead reader
      // wake the writer up, and dequeue myself.

      // Remove an edge from the writer to me,
      wait_for_graph->remove_edge(follower->tid, tid);
      // Add an edge from the writer to ahead reader
      wait_for_graph->add_edge(follower->tid, (iter - 1)->tid);
      // There is no need to wake a writer up
      // because at least one reader exists in front of a writer.

      // Deadlock Detection!!
      if (is_myself_deadlock_victim(follower->tid, cycle_member)) {
        // If I am the deadlock victim,
        // I will release locks soon.
        std::cout
          << "\t(rd_unlock) If I am a victim, I will release a lock soon."
          << std::endl;

        std::cout
          << "\t(rd_unlock) tid: " << tid << ", record_id: " << record_id
          << std::endl;

        std::cout
          << "\t(rd_unlock) abort second newest transaction, thread "
          << cycle_member[1]
          << std::endl;


        //TODO: Do I have to send signals?
        threads_abort_flag[cycle_member[1]] = true;
        pthread_cond_signal(&cond_var[cycle_member[1]]);

      }


      // Dequeue myself.
    } else {
      assert(follower->current_phase == READ);
      //  2-1-1-3-3. If a reader follows, do not change lock status, and
      // dequeue myself.
      //
      //  Do not change lock status. It is still RW_READER_LOCK
    }
  }

  // Dequeue myself.
  readers_count[record_id]--;
  record_wait_queues[record_id].erase(iter);
  return true;
}
bool rw_lock_table::wr_unlock(uint64_t tid, uint64_t record_id) {
  // 2-2. RW_WRITER_LOCK.
  //   2-2-1. Find my location. I should be a top of a queue.
  //         If I am not a top, it means an error is occurred.
  //   2-2-2. If any reader or writer follows me, remove edge from it to me
  //         , wake it up, and dequeue myself.
  //         Change lock status to RW_UNLOCK
  //   2-2-2. If I am alone, just dequeue myself.
  //         Change lock status to RW_UNLOCK



  //   2-2-1. Find my location. I should be a top of a queue.
  auto top = record_wait_queues[record_id].begin();
  //         If I am not a top, it means an error is occurred.
  assert(top->tid == tid);

  auto follower = top + 1;
  if (follower != record_wait_queues[record_id].end()) {
    // 2-2-2. If any reader or writer follows me, remove edge from it to me
    //       , wake it up, and dequeue myself.

    wait_for_graph->remove_edge(follower->tid, tid);
    pthread_cond_signal(&cond_var[follower->tid]);

    std::cout << "[tid: " << tid << ", record_id: " << record_id
      << "] (wr_unlock) wake up thread "
      << follower->tid << std::endl;



    // If the follower is reader, wake up all reader before meeting a writer
    if (follower->current_phase == READ) {
      for (follower++;

          follower != record_wait_queues[record_id].end() &&
          follower->current_phase == READ;

          follower++) {

        wait_for_graph->remove_edge(follower->tid, tid);
        pthread_cond_signal(&cond_var[follower->tid]);

        std::cout << "[tid: " << tid << ", record_id: " << record_id
          << "] (wr_unlock) wake up thread "
          << follower->tid << std::endl;
      }
    }


  } else {
    // 2-2-2. If I am alone, just dequeue myself.
  }

  // Dequeue myself.
  record_wait_queues[record_id].erase(top);

  // Change lock status to unlock
  table[record_id] = RW_UNLOCK;

  return true;
}


void rw_lock_table::print_deadlock(std::vector<uint64_t> &cycle_member) {
  wait_for_graph->print_cycle(cycle_member);
}



uint64_t rw_lock_table::get_newest_tid(
    std::vector<uint64_t> &cycle_member) {

  // Find the largest timestamp
  std::sort(cycle_member.begin(),
      cycle_member.end(),
      [this](uint64_t lhs_tid, uint64_t rhs_tid){
      return threads_timestamp[lhs_tid] >
      threads_timestamp[rhs_tid];
      });

  // return a tid of largest time stamp
  return cycle_member[0];
}


// This function detects deadlock.
// If a deadlock is found, it trun on a victim's abort flag.
// If current thread is a victim, this function returns true.
// Otherwise, false.
bool rw_lock_table::is_myself_deadlock_victim(uint64_t tid, 
    std::vector<uint64_t> &cycle_member){

  uint64_t newest_tid;

  /* std::cout <<
   *   "(is_myself_deadlock_victim) printing cycle_member" << std::endl;
   * std::cout << "(is_myself_deadlock_victim) return: "  <<
   *   is_deadlock_exist(tid, cycle_member) << std::endl; */

  if (is_deadlock_exist(tid, cycle_member)) {
    // Deadlock is found
    //
    // 2-1-3. If deadlock is found, add deadlock members to cycle_member.
    //   2-1-3-1. Find the newest thread among cycle_member.
    //   2-1-3-2. If I am the newest one, return true.
    //   2-1-3-3. If I am not, turn abort flag of victim on.

    // Find the newest thread and turn its abort flag on.
    newest_tid = get_newest_tid(cycle_member);

    std::cout << "[tid: " << tid << "]\
      (is_myself_deadlock_victim) Deadlock found. The victim: " << newest_tid << std::endl;

    if (newest_tid == tid) {
      // Just return false.
      // Resource clearing is occurred out of this function.

      std::cout << "[tid: " << tid << "]\
        (is_myself_deadlock_victim) Myself is the victim! " << newest_tid << std::endl;
      return true;
    } else {
      // turn abort flag on of newest tid.
      threads_abort_flag[newest_tid] = true;
      pthread_cond_signal(&cond_var[newest_tid]);

      std::cout << "[tid: " << tid << "] (wr_unlock) wake up a victim thread "
        << newest_tid << std::endl;

    }
  }

  return false;
}


/* void rw_lock_table::clear_failed_rdlock(uint64_t tid, uint64_t record_id,
 *     std::vector<uint64_t> &cycle_member) {
 *   //TODO
 * }
 * void rw_lock_table::clear_failed_wrlock(uint64_t tid, uint64_t record_id,
 *     std::vector<uint64_t> &cycle_member) {
 *   //TODO
 * } */
