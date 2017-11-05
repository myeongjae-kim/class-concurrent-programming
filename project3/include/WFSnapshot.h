#ifndef __WFSNAPSHOT_H__
#define __WFSNAPSHOT_H__

#include "snapshot.h"
#include "stamped_snap.h"

class WFSnapshot_t : public snapshot_t {
public:
  WFSnapshot_t (int capacity, T init);
  virtual ~WFSnapshot_t ();

  void update(T value, const int64_t tid);
  std::vector<T> scan();

private:
  std::vector< stamped_snap_t > a_table;
  std::vector< stamped_snap_t > collect();
};

#endif
