/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : snapshot.h
 * Due date             : 2017-11-6
 * Compilation Standard : c++11 */

#ifndef __SNAPSHOT_H__
#define __SNAPSHOT_H__

#include "main.h"
#include <array>

class snapshot_t {
public:
  virtual void update(T v, const int64_t) = 0;
  virtual T* scan() = 0;
};

#endif
