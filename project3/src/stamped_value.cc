/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : main.cc
 * Due date             : 2017-11-6
 * Compilation Standard : c++11 */

#include "stamped_value.h"

extern int64_t num_of_threads;

stamped_value_t::stamped_value_t(T init) {
  stamp = 0;
  value = init;
}

stamped_value_t::stamped_value_t(int64_t stamp, T value) {
  this->stamp = stamp;
  this->value = value;
}


stamped_value_t stamped_value_t::max(
      stamped_value_t &x, stamped_value_t &y){
  if (x.stamp > y.stamp) {
    return x;
  } else {
    return y;
  }
}

stamped_value_t stamped_value_t::MIN_VALUE(0);


bool stamped_value_t::operator==(stamped_value_t &rhs) {
  return (this->stamp == rhs.stamp) && (this->value == rhs.value);
}


