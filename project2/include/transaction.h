/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : transaction.h
 * Due date             : 2017-10-22
 * Compilation Standard : c++11 */


#ifndef __TRANSACTION_H__
#define __TRANSACTION_H__

#define TRX_DBG

//
// 1. Choose distinct three record id (i, j, k)
// 2. Acquire global mutex
// 3. Try to acquire reader lock of i
//  3-1. If trying is fail, it means there is a deadlock.
//      'i' is the member of deadlock.
//      Find deadlock members and abort newest thread.
//    3-1-1. If I am the newest thread, abort myself.
//          Call Rollback().
//      3-1-1-1. Rollback()
//             - It is quite similar to unlock().
//             - There is a four case. READ, FIRST_WRITE, SECOND_WRITE, COMMIT
//             - Restore resources(record_wait_queues, wait_for_graph)
//             - Lock table is not changed yet. So lock_table need not to be
//              restored.
//    3-1-2. If I am not the newest thread, abort the newest one.
//          Turn on 'abort' flag and wake it up. When a newest thread wakes up,
//          It sees the flag and it will start rollback procedure.
//      3-1-2-1. Go to sleep. 
//              Wait until a rollback procedure is done.
//              If an aborted thread is the one I waited, I will be waken up.
//              If not, sleep until someone wakes me up.
//    
//
//
void* transaction(void* arg);

#endif
