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
  //    2-1-2. Check whether deadlock is made or not.
  //      2-1-2-1. If deadlock is occurred, abort this locking process.
  //      2-1-2-2. If not, keep going to get a lock.
  //    2-1-2. Sleep until there is no writer ahead of me.
  //  2-2. Go to get a lock
  //  2-3. Write status to lock table and increase readers_count


  // 1. Insert tid to record_wait_queues
  record_wait_queues[record_id].push_back( wait_q_elem_t({tid, READ}) );
  assert(record_wait_queues[record_id].size() >= 1);


  // 2. Check whether waiting is needed or not.
  
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

    // Check whether a writer exists.
    uint64_t ahead_writer_tid = find_ahead_writer(tid, record_id);

    // If ahead_writer_tid == 0, there is no writer in front of this thread.
    if (ahead_writer_tid != 0) {
      //  2-1. Waiting
      //    2-1-1. Add edge to wait_for graph.
      //    2-1-2. Check whether deadlock is made or not.
      //      2-1-2-1. If deadlock is occurred, abort this locking process.
      //      2-1-2-2. If not, keep going to get a lock.
      //    2-1-2. Sleep until there is no writer ahead of me.

      //    2-1-1. Add edge to wait_for graph.
      wait_for_graph->add_edge(tid, ahead_writer_tid);


      // 2-1-2. Check whether deadlock is made or not.
      if (is_deadlock_exist(tid, cycle_member)) {
        // 2-1-2-1. If deadlock is occurred, abort this locking process.
        rdlock_clear_abort(tid, record_id);
        return false;
      }

      // 2-1-2-2. If not, keep going to get a lock.
      do {
#ifdef DBG
        std::cout << "[tid: " << tid << ", record_id: " << record_id
          << "] (rdlock) Go to sleep" << std::endl;
#endif

        //2-1-2. Sleep until there is no writer ahead of me.
        pthread_cond_wait(&cond_var[tid], global_mutex);

#ifdef DBG
        std::cout << "[tid: " << tid << ", record_id: " << record_id
          << "] (rdlock) Good morning!" << std::endl;
#endif

      } while ( find_ahead_writer(tid, record_id) );

    } else {
      // Writer is not found.
      // 2-2. Go to get a lock
    }
  }

  //  2-2. Go to get a lock
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
  //     2-1-1-1. If there are readers ahead of me,
  //             add all edge from me to readers
  //     2-1-1-2. If deadlock is occurred after adding edges, remove added
  //             edges and abort this acuiring.
  //    2-1-2. Iterate until there is no thread ahead of me.
  //      2-1-2-1. Do deadlock detection.
  //        2-1-2-3. If I am the vicitm, clear queue, graph and return false.
  //        2-1-2-3. If I am not, wake the victim up and sleep.
  //        2-1-2-4. Wake up! Check deadlock again.
  //  2-2. Not waiting. Go to get a lock
  //  2-3. Write status to lock table

  assert(phase == FIRST_WRITE || phase == SECOND_WRITE);
  // 1. Insert tid to record_wait_queues
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

    //    2-1-1. Add edge to wait_for graph.
    //     2-1-1-1. If there are readers ahead of me,
    //             add all edge from me to readers
    //     2-1-1-2. If deadlock is occurred after adding edges, remove added
    //             edges and abort this acuiring.



    // Find ahead readers
    auto myself = record_wait_queues[record_id].end() - 1;
    assert(myself->tid == tid);

    auto ahead = myself;
    bool ahead_is_reader = (ahead - 1)->current_phase == READ;

    // If right ahead is writer, just one edge is added.
    // Else, add edge to every ahead reader.
    // Do not add edge when a writer is found.
    if (ahead_is_reader) {
      // right ahead is reader 
      do {
        ahead--;
        wait_for_graph->add_edge(tid, ahead->tid);
      } while(
          ahead != record_wait_queues[record_id].begin()
          && (ahead - 1)->current_phase == READ
          );
    } else {
      // ahead is writer
      ahead--;
      wait_for_graph->add_edge(tid, ahead->tid);
    }


    if (is_deadlock_exist(tid, cycle_member)){

      // 2-1-1-2. If deadlock is occurred after adding edges, remove added
      //         edges and abort this acuiring.
      wrlock_clear_abort(tid, record_id);

      return false;
    }


    //    2-1-2. Iterate until there is no thread ahead of me.
    //      2-1-2-1. Do deadlock detection.
    //        2-1-2-3. If I am the vicitm, clear queue, graph and return false.
    //        2-1-2-3. If I am not, wake the victim up and sleep.
    //        2-1-2-4. Wake up! Check deadlock again.
    do {
#ifdef DBG
      std::cout << "[tid: " << tid << ", record_id: " << record_id
        << "] (wrlock) Go to sleep" << std::endl;
#endif

      pthread_cond_wait(&cond_var[tid], global_mutex);

#ifdef DBG
      std::cout << "[tid: " << tid << ", record_id: " << record_id
        << "] (wrlock) Good morning!" << std::endl;
#endif
    } while ( find_ahead_reader_or_writer(tid, record_id) );
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

#ifdef DBG
    print_tid_record(tid, record_id);
    std::cout << "(unlock) lock status is RW_READER_LOCK. "
      << "rd_unlock() will be called."<< std::endl;
#endif

    rd_unlock(tid, record_id, cycle_member);
  } else if (table[record_id] == RW_WRITER_LOCK) { 

#ifdef DBG
    print_tid_record(tid, record_id);
    std::cout << "(unlock) lock status is RW_WRITER_LOCK. "
      << "wr_unlock() will be called."<< std::endl;
#endif

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
  //      2-1-1-2-3. If a reader follows, explore the queue and find a writer.
  //                If a writer is found, remove edge from the writer to me.
  //                Because writer has every edge from the writer to ahead
  //                readers, one of the edge whose destination is me should
  //                be removed.
  //
  //                Do not change lock status, and dequeue myself.
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
  //      2-1-1-3-3. If a reader follows, explore the queue and find a writer.
  //                If a writer is found, remove edge from the writer to me.
  //                Because writer has every edge from the writer to ahead
  //                readers, one of the edge whose destination is me should
  //                be removed.
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
      pthread_cond_signal(&cond_var[follower->tid]);


#ifdef DBG
      std::cout << "[tid: " << tid << ", record_id: " << record_id
        << "] (rd_unlock) wake up thread "
        << follower->tid << std::endl;
#endif

    } else {
      assert(follower->current_phase == READ);

      // 2-1-1-2-3. If a reader follows, explore the queue and find a writer.
      //           If a writer is found, remove edge from the writer to me.
      //           Because writer has every edge from the writer to ahead
      //           readers, one of the edge whose destination is me should
      //           be removed.
      //
      //           Do not change lock status, and dequeue myself.

      // Find writer and remove edge
      do{
        follower++;
      }while(follower != record_wait_queues[record_id].end()
          && follower->current_phase == READ);

      if (follower != record_wait_queues[record_id].end()) {
        assert(follower->current_phase != READ);
        // Writer is found.
        // Remove edge!
        wait_for_graph->remove_edge(follower->tid, tid);
      }

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

      // Remove an edge from the writer to me,
      wait_for_graph->remove_edge(follower->tid, tid);
      // There is no need to wake a writer up
      // because at least one reader exists in front of a writer.

      // Dequeue myself.
    } else {
      assert(follower->current_phase == READ);

      // 2-1-1-3-3. If a reader follows, explore the queue and find a writer.
      //           If a writer is found, remove edge from the writer to me.
      //           Because writer has every edge from the writer to ahead
      //           readers, one of the edge whose destination is me should
      //           be removed.
      //
      //           Do not change lock status. It is still RW_READER_LOCK

      // Find writer and remove edge
      do{
        follower++;
      }while(follower != record_wait_queues[record_id].end()
          && follower->current_phase == READ);

      if (follower != record_wait_queues[record_id].end()) {
        assert(follower->current_phase != READ);
        // Writer is found
        // remove edge!
        wait_for_graph->remove_edge(follower->tid, tid);
      }
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
  //   2-2-3. If I am alone, just dequeue myself.
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

#ifdef DBG
    std::cout << "[tid: " << tid << ", record_id: " << record_id
      << "] (wr_unlock) wake up thread "
      << follower->tid << std::endl;
#endif

    // If the follower is reader, wake up all reader before meeting a writer
    if (follower->current_phase == READ) {
      for (follower++;

          follower != record_wait_queues[record_id].end() &&
          follower->current_phase == READ;

          follower++) {

        wait_for_graph->remove_edge(follower->tid, tid);
        pthread_cond_signal(&cond_var[follower->tid]);

#ifdef DEBG
        std::cout << "[tid: " << tid << ", record_id: " << record_id
          << "] (wr_unlock) wake up thread "
          << follower->tid << std::endl;
#endif
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


// It is just a wrapper of wait_for_graph->print_cycle();
void rw_lock_table::print_deadlock(std::vector<uint64_t> &cycle_member) {
  wait_for_graph->print_cycle(cycle_member);
}

void rw_lock_table::rdlock_clear_abort(uint64_t tid, uint64_t record_id) {

#ifdef DBG
  std::cout << "[tid: " << tid << ", record_id: " << record_id
    << "] (rdlock_clear_abort) I am the victim of deadlock. Return false "
    << std::endl;
#endif

  // Preconditions:
  //  - I must exist in the queue.
  //  - Deadlock which includes current tid must exists.

  // Wake up a threads that waits me.
  // After I am aborted, the thread can proceed.

  // I must be at the back of the queue
  // Check whether writer exists ahead of me.
  //  1. A writer exists.
  //    1-1. Remove edge from me to the writer.
  //    1-2. Dequeue myself.
  //  2. A writer does not exist.
  //    2-1. Dequeue myself.


  // Precondtion: I exist in queue!
  assert(record_wait_queues[record_id].size() != 0);

  // Wakeup deadlock member who is waiting me.
  auto &&cycle_member = wait_for_graph->get_cycle(tid);

  // deadlock should exist!
  assert(cycle_member.size() != 0);
  assert(cycle_member[0] == tid);

  // Wake up a threads that waits me.
  // After I am aborted, the thread can proceed.
  // cycle_member[1] is the thread that waits victim.
  pthread_cond_signal(&cond_var[cycle_member[1]]);


  // Find myself to remove from the queue
  auto myself = record_wait_queues[record_id].begin();
  while (myself != record_wait_queues[record_id].end()
      && myself->tid != tid) {
    myself++;
  }

  // Where am I?
  assert(myself != record_wait_queues[record_id].end());

  // I must be in the back of the queue
  assert(myself == record_wait_queues[record_id].end() - 1);

  
  // Check whether writer exists ahead of me.

  print_tid_record(tid, record_id);
  std::cout << "(rdlock_clear_abort) Case2: I am the back of the queue." << std::endl;


  //    2-1. Dequeue myself.

  //  Writer ahead of me.
  auto ahead_writer = record_wait_queues[record_id].rbegin() + 1;
  // Search from the back of the queue.
  // If a writer is found, break.
  while (ahead_writer != record_wait_queues[record_id].rend()
      && ahead_writer->current_phase == READ) {
    ahead_writer++;
  }

  //  1. A writer exists.
  if (ahead_writer != record_wait_queues[record_id].rend()) {
    assert(ahead_writer->current_phase != READ);

    // 1-1. Remove edge from me to the writer.
    wait_for_graph->remove_edge(tid, ahead_writer->tid);

    // 1-2. Dequeue myself.

    // 2. A writer does not exist.
  } else {
    assert(ahead_writer == record_wait_queues[record_id].rend());
    //  2-1. Dequeue myself.
  }

  // Finally, dequeue myself!
  record_wait_queues[record_id].erase(myself);

  return;
}

void rw_lock_table::wrlock_clear_abort(uint64_t tid, 
    uint64_t record_id) {

#ifdef DBG
  std::cout << "[tid: " << tid << ", record_id: " << record_id
    << "] (wrlock_clear_abort) I am the victim of deadlock. Return false "
    << std::endl;
#endif

  // Clear queue and graph
  // Add adge from follow thread to ahead thread
  // when follower thread is waiting me.
  //
  // Another deadlock can be occurred.
  //
  // I could be in the middle of the queue or back of the queue.

  // Find me in the queue.

  // I must be at the back of the queue
  //  1. There must be a thread I am waiting,
  //    Remove edge from me to the thread.
  //   1-1. If there are readers ahead of me, remove all edges
  //       from me to the readers which are added before entering
  //       current abort function.
  //  2. Dequeue myself.

  // Precondtion: I exist in queue!
  assert(record_wait_queues[record_id].size() != 0);

  // Wakeup deadlock member who is waiting me.
  auto &&cycle_member = wait_for_graph->get_cycle(tid);

  // deadlock should exist!
  assert(cycle_member.size() != 0);
  assert(cycle_member[0] == tid);

  // cycle_member[1] is the thread that waits victim.
  pthread_cond_signal(&cond_var[cycle_member[1]]);


  auto myself = record_wait_queues[record_id].end() - 1;
  assert(myself->tid == tid);

  // Where am I?
  assert(myself != record_wait_queues[record_id].end());

  // I must be in the back of the queue
  assert(myself == record_wait_queues[record_id].end() - 1);

  // 1. There must be a thread I am waiting,
  //     Remove edge from me to the thread.
  assert(record_wait_queues[record_id].size() >= 2);

#ifdef DBG
  print_tid_record(tid, record_id);
  std::cout << "(wrlock_clear_abort) Case2: I am the back of the queue." << std::endl;
#endif

  // 1-1. If there are readers ahead of me, remove all edges
  //       from me to the readers which are added before entering
  //       current abort function.

  // edge from me to the thread.
  auto ahead = myself;

  if ( (ahead - 1) ->current_phase == READ) {
    //ahead is reader
    do {
      ahead--;
      wait_for_graph->remove_edge(tid, ahead->tid);
    } while (
        ahead != record_wait_queues[record_id].begin()
        && (ahead - 1)->current_phase == READ
        );

  } else {
    //ahead is writer
    ahead--;
    wait_for_graph->remove_edge(tid, ahead->tid);
  }


  // 2-2. Dequeue myself.

  // Finally dequeue myself.
  record_wait_queues[record_id].erase(myself);

  return;
}


// This function finds a tid of a writer which is ahead of current thread.
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


// This function finds a tid of a reader or a writer
//  which is ahead of current thread.
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
