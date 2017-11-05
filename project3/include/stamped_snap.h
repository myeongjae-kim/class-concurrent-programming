

#ifndef __STAMPED_SNAP_H__
#define __STAMPED_SNAP_H__

#include "main.h"
#include <vector>

class stamped_snap_t
{
public:
  int64_t stamp;
  T value;
  std::vector<T> snap;

  stamped_snap_t (T value);
  stamped_snap_t (int64_t label, T value, std::vector<T> snap);
  // virtual ~stamped_snap_t ();
};

#endif
