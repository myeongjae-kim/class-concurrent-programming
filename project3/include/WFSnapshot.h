/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : WFSnapshot.h
 * Due date             : 2017-11-6
 * Compilation Standard : c++11 */

#ifndef __WFSNAPSHOT_H__
#define __WFSNAPSHOT_H__

#include "snapshot.h"
#include "stamped_snap.h"

class WFSnapshot_t : public snapshot_t {
public:
  WFSnapshot_t (int capacity, T init);
  virtual ~WFSnapshot_t ();

  void update(T value, const int64_t tid);
  T* scan();

private:
  stamped_snap_t** a_table;
  int capacity;
  stamped_snap_t** collect();
};

#endif
