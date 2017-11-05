
#include "stamped_snap.h"


stamped_snap_t::stamped_snap_t (T value) {
  this->stamp = 0;
  this->value = value;
  this->snap.clear();
}

stamped_snap_t::stamped_snap_t (
    int64_t label, T value, std::vector<T> snap){

  // below..?
  this->stamp = label;

  this->value = value;
  this->snap = snap;
}
