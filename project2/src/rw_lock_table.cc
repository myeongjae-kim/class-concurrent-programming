#include <cstring>
#include <cassert>
#include <algorithm>

#include "rw_lock_table.h"

void print_tid_record(uint64_t tid, uint64_t record_id);

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
  //    2-1-2. Iterate until there is no writer ahead of me.
  //      2-1-2-1. If I there is no writer ahead of me, break.
  //      2-1-2-2. If I should wait, do deadlock detection.
  //        2-1-2-3. If I am the vicitm, clear queue, graph and return false.
  //        2-1-2-3. If I am not, wake the victim up and sleep.
  //        2-1-2-4. Wake up! Check deadlock again.
  //  2-2. Go to get a lock
  //  2-3. Write status to lock table and increase readers_count


  // 1. Insert tid to record_wait_queues
  record_wait_queues[record_id].push_back( wait_q_elem_t({tid, READ}) );
  assert(record_wait_queues[record_id].size() >= 1);


  // 2. Check whether waiting is needed or not.
  //
  // If record_wait_queues[record_id] has one element,
  // Go to get a lock
  if (record_wait_queues[record_id].size() == 1) {
    // Do nothing.
    // Go to get a lock.
  } else {
    // Well, I am not the top of the queue.
    // Check whether a writer ahead of me is exist or not.
    // If there is no writer, I can acquire the lock.
    // If not, waiting procedure will be executed.
    
    
    // The # of queue elements should be at least 2.
    assert(record_wait_queues[record_id].size() > 1);

    // Check whether a writer is exists.
    uint64_t ahead_writer_tid = find_ahead_writer(tid, record_id);

    // If ahead_writer_tid == 0, there is no writer in front of this thread.
    if (ahead_writer_tid != 0) {
      //  2-1. Waiting
      //    2-1-1. Add edge to wait_for graph.
      //    2-1-2. Iterate until there is no writer ahead of me.
      //      2-1-2-1. If I there is no writer ahead of me, break.
      //      2-1-2-2. Do deadlock detection.
      //        2-1-2-3. If I am the vicitm, clear queue, graph and return false.
      //        2-1-2-3. If I am not, wake the victim up and sleep.
      //        2-1-2-4. Wake up! Check deadlock again.
      //                If my abort flag is on, abort this transaction.

      //    2-1-1. Add edge to wait_for graph.
      wait_for_graph->add_edge(tid, ahead_writer_tid);

      // TODO: below can be implemented as a do while function.
      while ( find_ahead_writer(tid, record_id) ) {
        //    2-1-2. Iterate until there is no writer ahead of me.
        //      2-1-2-1. If I am the top of a queue, break.
        //      2-1-2-2. Do deadlock detection.
        //        2-1-2-3. If I am the vicitm, clear queue, graph and return false.
        //        2-1-2-3. If I am not, wake the victim up and sleep.
        //        2-1-2-4. Wake up! Check deadlock again.
        //                If my abort flag is on, abort this transaction.
  

        //      2-1-2-1. If I am the top of a queue, break.
        if (record_wait_queues[record_id].begin()->tid == tid) {
          //TODO: This case can be ommitted.
          //Is this possible? ahead_writer is found but I am begin of the queue?

          // below assert is just detecting this case happen
          assert(false);
          break;
        }

        // 2-1-2-2. Do deadlock detection.


        // 2-1-2-3. If I am the vicitm, clear queue, graph and return false.
        // 2-1-2-3. If I am not, wake the victim up and sleep.
        // Victim wakeup is occurred in 'is_myself_deadlock_victim'.
        if (is_myself_deadlock_victim(tid, cycle_member)) {
          
          rdlock_clear_abort(tid, record_id);
          return false;
        }


        std::cout << "[tid: " << tid << ", record_id: " << record_id
          << "] (rdlock) Go to sleep" << std::endl;

        pthread_cond_wait(&cond_var[tid], global_mutex);

        // 2-1-2-4. Wake up! Check deadlock again.
        std::cout << "[tid: " << tid << ", record_id: " << record_id
          << "] (rdlock) Good morning!" << std::endl;


        // If my abort flag is on, abort this transaction.
        if (threads_abort_flag[tid]) {
          print_tid_record(tid, record_id);
          std::cout << "My abort flag is on. Abort this transaction" << std::endl;

          threads_abort_flag[tid] = false;

          // Check again is there really deadlock.
          // If a deadlock not exists, do not abort transaction.
          if (is_deadlock_exist(tid, cycle_member)) {


            rdlock_clear_abort(tid, record_id);
            return false;
          }

          // Deadlock was removed. Keep going to acquire a lock.
        }

      }


      // assert(finally, deadlock is not exist!);
      // assert(is_deadlock_exist(tid, cycle_member) == false);

      // 2-1-5. Waken up! Check whether current status of queue is okay or not.
      // If there is a writer in front of me,
      // stop the program. This is an error.




#ifdef DBG

      // Lock is acquired.
      // Check a lock is really acquired.

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
  //    2-1-2. Iterate until there is no thread ahead of me.
  //      2-1-2-1. Do deadlock detection.
  //        2-1-2-3. If I am the vicitm, clear queue, graph and return false.
  //        2-1-2-3. If I am not, wake the victim up and sleep.
  //        2-1-2-4. Wake up! Check deadlock again.
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
    // The # of queue elements should be at least 2.
    assert(record_wait_queues[record_id].size() > 1);

    // 2-1-1. Add edge to wait_for graph.
    // find a thread that is in front of me.

    //TODO
    uint64_t ahead_rw_tid = find_ahead_reader_or_writer(tid, record_id);

    // tid should not be zero
    assert(ahead_rw_tid != 0);

    // Add edge to wait_for graph.
    wait_for_graph->add_edge(tid, ahead_rw_tid);


    //  2-1. Waiting
    //    2-1-1. Add edge to wait_for graph.
    //    2-1-2. Iterate until there is no thread ahead of me.
    //      2-1-2-1. Do deadlock detection.
    //        2-1-2-3. If I am the vicitm, clear queue, graph and return false.
    //        2-1-2-3. If I am not, wake the victim up and sleep.
    //        2-1-2-4. Wake up! Check deadlock again.
    //                If my abort flag is on, abort this transaction.

    // TODO: below can be implemented as a do while function.
    while ( find_ahead_reader_or_writer(tid, record_id) ) {
      //    2-1-2. Iterate until there is no thread ahead of me.
      //      2-1-2-1. Do deadlock detection.
      //        2-1-2-3. If I am the vicitm, clear queue, graph and return false.
      //        2-1-2-3. If I am not, wake the victim up and sleep.
      //        2-1-2-4. Wake up! Check deadlock again.
      //                If my abort flag is on, abort this transaction.

      //      2-1-2-1. If I am the top of a queue, break.
      if (record_wait_queues[record_id].begin()->tid == tid) {
        //TODO: This case can be ommitted.
        //Is this possible? ahead_writer is found but I am begin of the queue?

        // below assert is just detecting this case happen
        assert(false);
        break;
      }

      //      2-1-2-1. Do deadlock detection.


      // 2-1-2-3. If I am the vicitm, clear queue, graph and return false.
      // 2-1-2-3. If I am not, wake the victim up and sleep.
      if (is_myself_deadlock_victim(tid, cycle_member)) {
        // Victim wakeup is occurred in 'is_myself_deadlock_victim'.

        wrlock_clear_abort(tid, record_id);
        return false;
      }

      std::cout << "[tid: " << tid << ", record_id: " << record_id
        << "] (wrlock) Go to sleep" << std::endl;

      pthread_cond_wait(&cond_var[tid], global_mutex);

      // 2-1-2-4. Wake up! Check deadlock again.
      std::cout << "[tid: " << tid << ", record_id: " << record_id
        << "] (wrlock) Good morning!" << std::endl;


      // If my abort flag is on, abort this transaction.
      if (threads_abort_flag[tid]) {
        print_tid_record(tid, record_id);
        std::cout << "My abort flag is on. Abort this transaction" << std::endl;

        threads_abort_flag[tid] = false;

        // Check again is there really deadlock.
        // If a deadlock not exists, do not abort transaction.
        if (is_deadlock_exist(tid, cycle_member)) {


          wrlock_clear_abort(tid, record_id);
          return false;
        }


        // Deadlock was removed. Keep going to acquire lock.
      }

    }


    // assert(finally, deadlock is not exist!);
    // assert(is_deadlock_exist(tid, cycle_member) == false);
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

    print_tid_record(tid, record_id);
    std::cout << "(unlock) lock status is RW_READER_LOCK. "
      << "rd_unlock() will be called."<< std::endl;

    rd_unlock(tid, record_id, cycle_member);
  } else if (table[record_id] == RW_WRITER_LOCK) { 

    print_tid_record(tid, record_id);
    std::cout << "(unlock) lock status is RW_WRITER_LOCK. "
      << "wr_unlock() will be called."<< std::endl;

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
  assert(record_wait_queues[record_id].size() > 0);

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

      print_tid_record(tid, record_id);

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
        print_tid_record(tid, record_id);
        std::cout
          << "\t(rd_unlock) New added edge makes deadlock."
          << "Wake the follower up to be aborted"
          << std::endl;

        print_tid_record(tid, record_id);
        std::cout << "wake up thread " << follower->tid << std::endl;
        pthread_cond_signal(&cond_var[follower->tid]);
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
    std::vector<uint64_t> cycle_member) {

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
    wait_for_graph->print_cycle(cycle_member);

    if (newest_tid == tid) {
      // Just return false.
      // Resource clearing is occurred out of this function.

      std::cout << "[tid: " << tid << "]\
        (is_myself_deadlock_victim) Myself is the victim! " << newest_tid << std::endl;

      std::cout << "[tid: " << tid << "]\
        (is_myself_deadlock_victim) So wake up a thread that waits me. " << std::endl;

      std::cout << "[tid: " << tid << "]\
        (is_myself_deadlock_victim) Wake up thread "
        << cycle_member[1] << std::endl;

      pthread_cond_signal(&cond_var[cycle_member[1]]);

      return true;
    } else {
      // turn abort flag on of newest tid.
      threads_abort_flag[newest_tid] = true;
      pthread_cond_signal(&cond_var[newest_tid]);

      std::cout << "[tid: " << tid << "] (is_myself_deadlock_victim) wake up a victim thread "
        << newest_tid << std::endl;

      // TODO: When a victim is waken up, it executes clean up procedure
      // and wakes up a thread that waits itself.
    }
  }

  return false;
}


void rw_lock_table::rdlock_clear_abort(uint64_t tid, uint64_t record_id) {

  std::cout << "[tid: " << tid << ", record_id: " << record_id
    << "] (rdlock_clear_abort) I am the victim of deadlock. Return false "
    << std::endl;

  // Clear queue and graph
  // Add adge from follow thread to ahead thread
  // when follower thread is waiting me.
  //
  // Another deadlock can be occurred.
  //
  // I could be in the middle of the queue or back of the queue.


  // rdlock
  // Case1. I am alone in the queue
  //     1-1. Dequeue myself.
  // Case2. I am the back of the queue
  //     2-1. Check whether writer exists ahead of me.
  //      2-1-1. A writer exists.
  //        2-1-1-1. Remove edge from me to the writer.
  //        2-1-1-2. Dequeue myself.
  //      2-1-2. A writer does not exist.
  //        2-1-2-1. Dequeue myself.
  // Case3. I am the front of the queue
  //     3-1. There must be a thread that is waiting me.
  //     3-2. If the follower is writer.
  //      3-2-1. Remove edge from the writer to me.
  //      3-2-2. Wake the writer up.
  //      3-2-3. Dequeue myself.
  //     3-3. If the follower is reader.
  //      3-3-1. The reader is not waiting me. Just dequeue myself.
  //     /** 3-3-1. Wake up all following reader until the writer is found. **/
  //      3-3-2. Dequeue myself.
  // Case4. I am a middle of the queue
  //     4-1. Check whether writer exists ahead of me.
  //      4-1-1. A writer exists.
  //        4-1-1-1. Remove edge from me to the writer.
  //        4-1-1-2. If a follower is writer.
  //          4-1-1-2-1. Remove edge from the follower to me.
  //          4-1-1-2-2. Add edge from the follower to ahead thread.
  //        4-1-1-3. If a follower is reader.
  //          4-1-1-2-2. Do nothing. The follower is not waiting me.
  //        4-1-1-4. Dequeue myself.
  //      4-1-2. A writer does not exist.
  //        4-1-2-1. If a follower is writer.
  //          4-1-2-1-1. Remove edge from the follower to me.
  //          4-1-2-1-2. Add edge from the follower to ahead thread.
  //        4-1-2-2. If a follower is reader.
  //          4-1-1-2-2. Do nothing. The follower is not waiting me.
  //        4-1-2-3. Dequeue myself.


  // Precondtion: I exist in queue!
  assert(record_wait_queues[record_id].size() != 0);


  // Wakeup deadlock member who is waiting me.
  auto &&cycle_member = wait_for_graph->get_cycle(tid);

  // deadlock should exist!
  assert(cycle_member.size() != 0);
  assert(cycle_member[0] == tid);

  // cycle_member[1] is the thread that waits victim.
  pthread_cond_signal(&cond_var[cycle_member[1]]);








  auto myself = record_wait_queues[record_id].begin();
  while (myself != record_wait_queues[record_id].end()
      && myself->tid != tid) {
    myself++;
  }

  // Where am I?
  assert(myself != record_wait_queues[record_id].end());

  // Case1. I am alone in the queue
  if (record_wait_queues[record_id].size() == 1) {
    assert(record_wait_queues[record_id][0].tid == tid);
    // 1-1. Dequeue myself.

    print_tid_record(tid, record_id);
    std::cout << "(rdlock_clear_abort) Case1: I am alone in the queue." << std::endl;


    // Case2. I am the back of the queue
  } else if (myself == record_wait_queues[record_id].end() - 1) {
    // 2-1. Check whether writer exists ahead of me.

    print_tid_record(tid, record_id);
    std::cout << "(rdlock_clear_abort) Case2: I am the back of the queue." << std::endl;

    //  Writer ahead of me.
    auto ahead_writer = record_wait_queues[record_id].rbegin() + 1;
    // Search from the back of the queue.
    // If a writer is found, break.
    while (ahead_writer != record_wait_queues[record_id].rend()
        && ahead_writer->current_phase == READ) {
      ahead_writer++;
    }

    //  2-1-1. A writer exists.
    if (ahead_writer != record_wait_queues[record_id].rend()) {
      assert(ahead_writer->current_phase != READ);

      // 2-1-1-1. Remove edge from me to the writer.
      wait_for_graph->remove_edge(tid, ahead_writer->tid);

      // 2-1-1-2. Dequeue myself.

      // 2-1-2. A writer does not exist.
    } else {
      assert(ahead_writer == record_wait_queues[record_id].rend());
      //  2-1-2-1. Dequeue myself.
    }

    // Case3. I am the front of the queue
  } else if(myself == record_wait_queues[record_id].begin()){
    // Front is me.
    assert(record_wait_queues[record_id][0].tid == tid);

    print_tid_record(tid, record_id);
    std::cout << "(rdlock_clear_abort) Case3: I am the front of the queue." << std::endl;



    // 3-1. There must be a thread that is waiting me.
    assert(record_wait_queues[record_id].size() >= 2);

    auto follower = myself + 1;

    // 3-2. If the follower is writer.
    if (follower->current_phase != READ) {
      //  3-2-1. Remove edge from the writer to me.
      wait_for_graph->remove_edge(follower->tid, tid);

      //  3-2-2. Wake the writer up.
      print_tid_record(tid, record_id);
      std::cout << "(rdlock_clear_abort) I am failed lock try."
        << "Waiting is removed so wake up thread "
        << follower->tid << std::endl;

      pthread_cond_signal(&cond_var[follower->tid]);

      //  3-2-3. Dequeue myself.

      // 3-3. If the follower is reader.
    } else {
      assert(follower->current_phase == READ);
      //  3-3-1. The reader is not waiting me. Just dequeue myself.
      //  /** 3-3-1. Wake up all following reader until the writer is found. **/

      print_tid_record(tid, record_id);
      std::cout << "(rdlock_clear_abort) Case3: I am the middle of the queue." << std::endl;
      /* 
       *       // at least one follower existence is guaranteed.
       *       do {
       *         // Wake up all the following reader
       *
       *         print_tid_record(tid, record_id);
       *         std::cout << "(rdlock_clear_abort) I am failed lock try."
       *           << "Waiting is removed so wake up thread "
       *           << follower->tid << std::endl;
       *
       *         pthread_cond_signal(&cond_var[follower->tid]);
       *         // go to next thread
       *         follower++;
       *       } while (follower != record_wait_queues[record_id].end()
       *           && follower->current_phase == READ);
       *
       *  */
      //  3-3-2. Dequeue myself.
    }

    // Case4. I am a middle of the queue
  } else {
    // size is at least 3
    assert(record_wait_queues[record_id].size() >= 3);

    // I am not end of the queue
    assert(record_wait_queues[record_id].end() - 1 != myself);
    // nor the begin of the queue.
    assert(record_wait_queues[record_id].begin() != myself);



    // 4-1. Check whether writer exists ahead of me.
    auto ahead_writer = myself;
    // Find writer
    do {
      ahead_writer--;
    } while (ahead_writer->current_phase == READ
        && ahead_writer != record_wait_queues[record_id].begin());

    auto follower = myself + 1;
    assert(follower != record_wait_queues[record_id].end());

    // 4-1-1. A writer exists.
    if (ahead_writer->current_phase != READ) {
      // 4-1-1-1. Remove edge from me to the writer.
      wait_for_graph->remove_edge(tid, ahead_writer->tid);

      // 4-1-1-2. If a follower is writer.
      if (follower->current_phase != READ) {
        // 4-1-1-2-1. Remove edge from the follower to me.
        wait_for_graph->remove_edge(follower->tid, tid);
        // 4-1-1-2-2. Add edge from the follower to ahead thread.
        wait_for_graph->add_edge(follower->tid, (myself - 1)->tid);
      } else {
        // 4-1-1-3. If a follower is reader.
        //   4-1-1-2-2. Do nothing. The follower is not waiting me.
      }
      // 4-1-1-2. Dequeue myself.


      // 4-1-2. A writer does not exist.
    } else {
      // 4-1-2-1. If a follower is writer.
      if (follower->current_phase != READ) {
        //   4-1-2-1-1. Remove edge from the follower to me.
        wait_for_graph->remove_edge(follower->tid, tid);
        //   4-1-2-1-2. Add edge from the follower to ahead thread.
        wait_for_graph->add_edge(follower->tid, (myself - 1)->tid);
      }
      // 4-1-2-2. If a follower is reader.
      //   4-1-1-2-2. Do nothing. The follower is not waiting me.
      // 4-1-2-3. Dequeue myself.
    }
  }
  // Finally, dequeue myself!
  record_wait_queues[record_id].erase(myself);

  threads_abort_flag[tid] = false;

  return;
}

void rw_lock_table::wrlock_clear_abort(uint64_t tid, 
    uint64_t record_id) {

  std::cout << "[tid: " << tid << ", record_id: " << record_id
    << "] (wrlock_clear_abort) I am the victim of deadlock. Return false "
    << std::endl;

  // Clear queue and graph
  // Add adge from follow thread to ahead thread
  // when follower thread is waiting me.
  //
  // Another deadlock can be occurred.
  //
  // I could be in the middle of the queue or back of the queue.


  // Find me in the queue.

  // Case1. I am alone in the queue
  //     1-1. Dequeue myself.
  // Case2. I am the back of the queue
  //     2-1. There must be a thread I am waiting,
  //         Remove edge from me to the thread.
  //     2-2. Dequeue myself.
  // Case3. I am the front of the queue
  //     3-1. There must be a thread that is waiting me.
  //     3-2. Remove edge from the thread to me,
  //            wake it up, and dequeue myself.
  // Case4. I am a middle of the queue
  //        There must be a thread that I am waiting(ahead) and
  //         is waiting me(follower).
  //     4-1. Remove edge from the follower to me.
  //     4-2. Remove edge from me to ahead thread.
  //     4-3. Ahead == writer
  //       4-3-1. Add edge from follower thread to ahead thread.
  //       4-3-2. Dequeue myself.
  //     4-4. Ahead == reader
  //       4-4-1. follower == writer
  //         4-4-1-1. Add edge from follower thread to ahead thread.
  //         4-4-1-2. Dequeue myself.
  //
  //       4-4-2. follower == reader
  //         4-4-2-1. Find whether a writer exist in ahead of me.
  //           4-4-2-1-1. If a writer not exists, wake the follower up.
  //           4-4-2-1-2. If a writer exists,
  //                     add edge from follower to the writer.
  //         4-4-2-2. Dequeue myself.
  //


  // Precondtion: I exist in queue!
  assert(record_wait_queues[record_id].size() != 0);

  // Wakeup deadlock member who is waiting me.
  auto &&cycle_member = wait_for_graph->get_cycle(tid);

  // deadlock should exist!
  assert(cycle_member.size() != 0);
  assert(cycle_member[0] == tid);

  // cycle_member[1] is the thread that waits victim.
  pthread_cond_signal(&cond_var[cycle_member[1]]);







  auto myself = record_wait_queues[record_id].begin();
  while (myself != record_wait_queues[record_id].end()
      && myself->tid != tid) {
    myself++;
  }

  // Where am I?
  assert(myself != record_wait_queues[record_id].end());

  // Case1. I am alone in the queue
  if (record_wait_queues[record_id].size() == 1) {
    assert(record_wait_queues[record_id][0].tid == tid);
    // 1-1. Dequeue myself.

    print_tid_record(tid, record_id);
    std::cout << "(wrlock_clear_abort) Case1: I am alone in the queue." << std::endl;

    // Case2. I am the back of the queue
  } else if(myself == record_wait_queues[record_id].end() - 1) {
    // 2-1. There must be a thread I am waiting,
    //     Remove edge from me to the thread.
    assert(record_wait_queues[record_id].size() >= 2);

    print_tid_record(tid, record_id);
    std::cout << "(wrlock_clear_abort) Case2: I am the back of the queue." << std::endl;

    // edge from me to the thread.
    wait_for_graph->remove_edge(tid, (myself-1)->tid);

    // 2-2. Dequeue myself.

    // Case3. I am the front of the queue
  } else if(myself == record_wait_queues[record_id].begin()) {
    // 3-1. There must be a thread that is waiting me.
    // 3-2. Remove edge from the thread to me,
    assert(record_wait_queues[record_id].size() >= 2);


    print_tid_record(tid, record_id);
    std::cout << "(wrlock_clear_abort) Case3: I am the front of the queue." << std::endl;


    // Remove edge from the thread to me,
    auto follower = myself + 1;
    wait_for_graph->remove_edge(follower->tid, tid);
    // wake it up, and dequeue myself.


    print_tid_record(tid, record_id);
    std::cout << "(wrlock_clear_abort) I am failed lock try."
      << "Waiting is removed so wake up thread "
      << follower->tid << std::endl;


    pthread_cond_signal(&cond_var[follower->tid]);


    // Case4. I am a middle of the queue
  } else {
    // size is at least 3
    assert(record_wait_queues[record_id].size() >= 3);

    // I am not end of the queue
    assert(record_wait_queues[record_id].end() - 1 != myself);
    // nor the begin of the queue.
    assert(record_wait_queues[record_id].begin() != myself);


    print_tid_record(tid, record_id);
    std::cout << "(wrlock_clear_abort) Case4: I am the middle of the queue." << std::endl;


    // must be a thread that I am waiting(ahead) and
    // waiting me(follower).
    auto ahead = myself - 1;
    auto follower = myself + 1;

    // 4-1. Remove edge from the follower to me.
    // 4-2. Remove edge from me to ahead thread.
    wait_for_graph->remove_edge(follower->tid, tid);
    wait_for_graph->remove_edge(tid, ahead->tid);


    // 4-3. Ahead == writer
    if (ahead->current_phase != READ) {
      //   4-3-1. Add edge from follower thread to ahead thread.
      //   4-3-2. Dequeue myself.

      print_tid_record(tid, record_id);
      std::cout << "ahead is writer. Just add edges" << std::endl;

      //  Add edge from follower thread to ahead thread.
      wait_for_graph->add_edge(follower->tid, ahead->tid);

      //   4-3-2. Dequeue myself.

      // 4-4. Ahead == reader
    } else {
      assert(ahead->current_phase == READ);
      //   4-4-1. follower == writer
      if (follower->current_phase != READ) {
        // 4-4-1-1. Add edge from follower thread to ahead thread.
        // 4-4-1-2. Dequeue myself.


        print_tid_record(tid, record_id);
        std::cout << "ahead is reader and follower is writer."
          << " Just add edges" << std::endl;


        // Add edge from follower thread to ahead thread.
        wait_for_graph->add_edge(follower->tid, ahead->tid);
        // Dequeue myself.

        // 4-4-2. follower == reader
      } else {
        assert(follower->current_phase == READ);
        //  4-4-2-1. Find whether a writer exist in ahead of me.
        //    4-4-2-1-1. If a writer not exists, wake the follower up.
        //    4-4-2-1-2. If a writer exists,
        //              add edge from follower to the writer.
        //  4-4-2-2. Dequeue myself.

        auto ahead_writer = myself;
        bool ahead_writer_exist = false;

        do {
          ahead_writer--;

          if (ahead_writer->current_phase != READ) {
            ahead_writer_exist = true;

            print_tid_record(tid, record_id);
            std::cout << "In case 4, ahead writer is found" << std::endl;

            break;
          }
        }while(ahead_writer != record_wait_queues[record_id].begin());

        //    4-4-2-1-1. If a writer not exists, wake the follower up.
        if (ahead_writer_exist == false) {
          print_tid_record(tid, record_id);
          std::cout << "(wrlock_clear_abort) I am failed lock try."
            << "Waiting is removed so wake up thread "
            << follower->tid << std::endl;
          pthread_cond_signal(&cond_var[follower->tid]);


        } else {
          //    4-4-2-1-2. If a writer exists,
          //              add edge from follower to the writer.
          print_tid_record(tid, record_id);
          std::cout << "ahead_writer is found. add edge from "
            << "follower to ahead_writer" << std::endl;
          wait_for_graph->add_edge(follower->tid, ahead_writer->tid);
        }


        //  4-4-2-2. Dequeue myself.
      }
    }
  }

  // Finally dequeue myself.
  record_wait_queues[record_id].erase(myself);

  threads_abort_flag[tid] = false;
  return;
}


uint64_t rw_lock_table::find_ahead_writer(uint64_t tid, uint64_t record_id){
  // Remember the last writer before meet me.
  // Return the last writer's tid.
  // If there is no writer, this function will return zero.

  // Check whether a writer is exists.
  uint64_t ahead_writer_tid = 0;
  bool i_am_found = false;

  auto iter = record_wait_queues[record_id].begin();
  while (iter != record_wait_queues[record_id].end()) {
    if (iter->tid == tid) {
      i_am_found = true;
      break;
    }

    assert(iter->current_phase != INVALID);

    if (iter->current_phase != READ) {
      // writer found.
      ahead_writer_tid = iter->tid;
    }
    iter++;
  }

  // If I am not in the queue, it is an error.
  assert(i_am_found);

  // If a ahead_writer_tid is my tid, it is an error.
  assert(tid != ahead_writer_tid);


  // If a writer's id is zero, there is no writer.
  return ahead_writer_tid;
}


uint64_t rw_lock_table::find_ahead_reader_or_writer(uint64_t tid,
    uint64_t record_id) {
  // If there is no reader or writer, this function will return zero.

  // Check whether a writer is exists.
  uint64_t ahead_tid = 0;
  bool i_am_found = false;

  auto iter = record_wait_queues[record_id].begin();
  while (iter != record_wait_queues[record_id].end()) {
    if (iter->tid == tid) {
      i_am_found = true;
      break;
    }

    assert(iter->current_phase != INVALID);

    // ahead reader or writer is found.
    ahead_tid = iter->tid;
    iter++;
  }

  // If I am not in the queue, it is an error.
  assert(i_am_found);

  // If a ahead_tid is my tid, it is an error.
  assert(tid != ahead_tid);

  // If a writer's id is zero, there is no writer.
  return ahead_tid;

}
