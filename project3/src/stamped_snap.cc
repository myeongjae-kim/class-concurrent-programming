/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : stamped_snap.cc
 * Due date             : 2017-11-6
 * Compilation Standard : c++11 */

#include "stamped_snap.h"

extern int64_t num_of_threads;

stamped_snap_t::stamped_snap_t (T value) {
  this->stamp = 0;
  this->value = value;
  this->snap = new T[num_of_threads];
  memset(this->snap, 0, sizeof(*this->snap) * num_of_threads);
}

stamped_snap_t::stamped_snap_t (
    int64_t label, T value, T* snap){
  this->stamp = label;
  this->value = value;
  this->snap = new T[num_of_threads];
  memcpy(this->snap, snap, sizeof(*this->snap) * num_of_threads);
}

stamped_snap_t::~stamped_snap_t () {
  delete[] this->snap;
}

