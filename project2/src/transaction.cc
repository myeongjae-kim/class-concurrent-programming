#include <cstring>

#include "main.h"
#include "rw_lock_table.h"
#include "directed_graph.h"

extern uint64_t N; // number of threads
extern uint64_t R; // number of records
extern uint64_t E; // maximum number of executions

// Which transaction will be aborted is decided by this array.
extern uint64_t *threads_timestamp;  // N elements

// for deadlock situation. Transaction killing order
extern uint64_t global_timestamp; 


// Lock
extern pthread_mutex_t global_mutex;
extern rw_lock_table *lock_table;

void print_tid_record(uint64_t tid, uint64_t record_id) {
  std::cout << "[tid: " << tid << ", record_id: " << record_id << "] ";
}


bool read_record(log_t &log) {
  print_tid_record(log.tid, log.i);
  std::cout << "reader lock acquire try"<< std::endl;

  if(lock_table->rdlock(log.tid, log.i,
        &global_mutex, *log.cycle_member) == false) {
    //deadlock found
    print_tid_record(log.tid, log.i);
    std::cout << "(read_record) the deadlock is found" << std::endl;
    lock_table->print_deadlock(*log.cycle_member);

    // I am the victim.

    return false;
  }

  print_tid_record(log.tid, log.i);
  std::cout << "reader lock acquired" << std::endl;
  return true;
}

bool first_write_record(log_t &log) {
  print_tid_record(log.tid, log.j);
  std::cout << "first writer lock acquire try" << std::endl;

  if(lock_table->wrlock(log.tid, FIRST_WRITE, log.j, &global_mutex,
        *log.cycle_member)
      == false) {
    //deadlock found
    print_tid_record(log.tid, log.j);
    std::cout <<"(first_write_record) the deadlock is found" << std::endl;
    lock_table->print_deadlock(*log.cycle_member);

    // I am the victim.
    
    return false;
  }

  print_tid_record(log.tid, log.j);
  std::cout << "first writer lock acquired" << std::endl;

  return true;
}

bool second_write_record(log_t &log) {
  print_tid_record(log.tid, log.k);
  std::cout << "second writer lock acquire try" << std::endl;

  if(lock_table->wrlock(log.tid, SECOND_WRITE, log.k,
        &global_mutex, *log.cycle_member) == false){
    //deadlock found
    print_tid_record(log.tid, log.k);
    std::cout << "(second_write_record) the deadlock is found" << std::endl;
    lock_table->print_deadlock(*log.cycle_member);

    // I am the victim.
    
    return false;
  }

  print_tid_record(log.tid, log.k);
  std::cout << "second writer lock acquired" << std::endl;

  return true;
}

void commit(log_t& log) {

}

void rollback_commit(log_t& log) {
  print_tid_record(log.tid, -1);
  std::cout << "(rollback_commit) commit phase rollbacking" << std::endl;
  
  // Release locks.
  // We don't need to prepare unlocking locks.
  // All locks are acquired correctly.
}

void rollback_second_write(log_t& log) {
  print_tid_record(log.tid, log.k);
  std::cout << "(rollback_second_write) second write phase rollbacking" << std::endl;
  
  // Release locks.
  if (log.current_phase == SECOND_WRITE) {
    // Second write lock acquire fail.
    // Do not try to unlcok second writer lock
  } else {
    lock_table->unlock(log.tid, log.k, *log.cycle_member);
  }
}
void rollback_first_write(log_t& log) {
  print_tid_record(log.tid, log.j);
  std::cout << "(rollback_first_write) first write phase rollbacking" << std::endl;
  
  // Release locks.
  if (log.current_phase == FIRST_WRITE) {
    // First write lock acquire fail.
    // Do not try to unlcok first writer lock
  } else {
    lock_table->unlock(log.tid, log.j, *log.cycle_member);
  }
}
void rollback_read(log_t& log) {
  print_tid_record(log.tid, log.i);
  std::cout << "(rollback_read) read phase rollbacking" << std::endl;
  
  // Release locks.
  if (log.current_phase == READ) {
    // Readlock acquire fail.
    // Do not try to unlcok read lock
  } else {
    lock_table->unlock(log.tid, log.i, *log.cycle_member);
  }
}

void rollback(log_t& log) {
  switch (log.current_phase) {
    case COMMIT:
      rollback_commit(log);

    case SECOND_WRITE:
      rollback_second_write(log);

    case FIRST_WRITE:
      rollback_first_write(log);

    case READ:
      rollback_read(log);

      break;
    default:
      std::cout << "(rollback) Default case! Error!" << std::endl;
      assert(false);
      exit(1);
  }

  print_tid_record(log.tid, log.i);
  std::cout << "(rollback) rollback is end. Go to next transaction" << std::endl;
}

void release_locks(log_t& log) {
  // Release reader lock.
  pthread_mutex_lock(&global_mutex);
  print_tid_record(log.tid, log.i);
  std::cout << "reader lock release try" << std::endl;

  lock_table->unlock(log.tid, log.i, *log.cycle_member);

  print_tid_record(log.tid, log.i);
  std::cout << "reader lock released." << std::endl;

  // Release first writer lock.
  print_tid_record(log.tid, log.j);
  std::cout << "first writer lock release try " << std::endl;

  lock_table->unlock(log.tid, log.j, *log.cycle_member);

  print_tid_record(log.tid, log.j);
  std::cout << "first writer lock released." << std::endl;

  // Release second writer lock.
  print_tid_record(log.tid, log.k);
  std::cout << "second writer lock release try " << std::endl;

  lock_table->unlock(log.tid, log.k, *log.cycle_member);

  print_tid_record(log.tid, log.k);
  std::cout << "second writer lock released." << std::endl;

  pthread_mutex_unlock(&global_mutex);
}


void* transaction(void* arg) {
  uint64_t tid = uint64_t(arg);

  // record_id

  std::vector<uint64_t> cycle_member;

  while (1) {
    std::cout << "[tid: " << tid << "] ";
    std::cout << "(transaction) ** Start new transaction **"
      << std::endl;

    // This is used to select which one is to be aborted.
    threads_timestamp[tid] = ++global_timestamp;


    // log initialization.
    log_t log;
    memset(&log, 0, sizeof(log_t));
    log.cycle_member = &cycle_member;

    log.tid = tid;

    // get distinct record_ids
    log.i = rand() % R;

    do{
      log.j = rand() % R;
    }while (log.i == log.j);

    do{
      log.k = rand() % R;
    }while (log.k == log.i || log.k == log.j);

    // Phase1: READ
    log.current_phase = READ;

    pthread_mutex_lock(&global_mutex);
    if (read_record(log) == false) {
      // Abort flag is on.
      print_tid_record(log.tid, log.i);
      std::cout << "(transaction) read_record() is fail."
        << " Abort this transaction."<< std::endl;

      print_tid_record(log.tid, log.i);
      std::cout << "(transaction) phase:" << log.current_phase << std::endl;

      rollback(log);
      // Rollback
      // 1. Remove edges from graph
      // 2. Add edges with precede node and follow node
      //
      // 3. Dequeue from graph
      // 4. Unlock (lock table status change)

      // exit(1);
      pthread_mutex_unlock(&global_mutex);
      continue;
    }
    pthread_mutex_unlock(&global_mutex);

    // Phase2: FIRST_WRITE
    log.current_phase = FIRST_WRITE;

    pthread_mutex_lock(&global_mutex);
    if(first_write_record(log) == false) {
      // Abort flag is on.
      print_tid_record(log.tid, log.j);
      std::cout << "(transaction) first_write_record() is fail."
        << " Abort this transaction."<< std::endl;

      print_tid_record(log.tid, log.j);
      std::cout << "(transaction) phase:" << log.current_phase << std::endl;

      rollback(log);
      // Rollback
      // 1. Remove edges from graph
      // 2. Add edges with precede node and follow node
      //
      // 3. Dequeue from graph
      // 4. Unlock (lock table status change)

      // exit(1);
      pthread_mutex_unlock(&global_mutex);
      continue;
    }
    pthread_mutex_unlock(&global_mutex);

    // Phase3: SECOND_WRITE
    log.current_phase = SECOND_WRITE;

    pthread_mutex_lock(&global_mutex);
    if(second_write_record(log) == false) {
      // Abort flag is on.
      print_tid_record(log.tid, log.k);
      std::cout << "(transaction) second_write_record() is fail."
        << " Abort this transaction."<< std::endl;

      print_tid_record(log.tid, log.k);
      std::cout << "(transaction) phase:" << log.current_phase << std::endl;

      rollback(log);
      // Rollback
      // 1. Remove edges from graph
      // 2. Add edges with precede node and follow node
      //
      // 3. Dequeue from graph
      // 4. Unlock (lock table status change)

      // exit(1);
      pthread_mutex_unlock(&global_mutex);
      continue;
    }
    pthread_mutex_unlock(&global_mutex);

    // Phase4: COMMIT
    log.current_phase = COMMIT;

    // If commit is fail (When commid > E)
    // do rollback and destory thread.
    commit(log);

    // Phase5: unlock
    release_locks(log);


    std::cout << "(transaction) ** End of transaction **"
      << std::endl;
  }

  return nullptr;
}
