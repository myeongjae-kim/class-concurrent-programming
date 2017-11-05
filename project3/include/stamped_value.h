/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : snapshot.h
 * Due date             : 2017-11-6
 * Compilation Standard : c++11 */

#ifndef __STAMPED_VALUE_H__
#define __STAMPED_VALUE_H__

#define IN
#define OUT

#include "main.h"

class stamped_value_t {
public:
  int64_t stamp;
  T value;
  // initial value with zero timestamp
  stamped_value_t(T init);
  // later values with timestamp provided
  stamped_value_t(int64_t stamp, T value);

  static stamped_value_t max(
      stamped_value_t &x, stamped_value_t &y);
  static stamped_value_t MIN_VALUE;

  bool operator==(stamped_value_t &rhs);
};

#endif
