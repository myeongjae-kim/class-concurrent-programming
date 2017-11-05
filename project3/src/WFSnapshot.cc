#include "WFSnapshot.h"

WFSnapshot_t::WFSnapshot_t (int capacity, T init) {
  a_table.reserve(capacity);

  stamped_snap_t buffer(init);

  for (int i = 0; i < capacity; ++i) {
    a_table.push_back(buffer);
  }
}


std::vector< stamped_snap_t > WFSnapshot_t::collect() {
  std::vector< stamped_snap_t > copy(a_table);
  return copy;
}

WFSnapshot_t::~WFSnapshot_t (){

}


void WFSnapshot_t::update(T value, const int64_t tid){
  std::vector<T> &&snap = scan();

  stamped_snap_t old_value = a_table[tid];
  stamped_snap_t new_value(old_value.stamp + 1, value, snap);
  
  a_table[tid] = new_value;
}


std::vector<T> WFSnapshot_t::scan() {
  std::vector<stamped_snap_t> old_copy;
  std::vector<stamped_snap_t> new_copy;

  int a_table_size = a_table.size();
  std::vector<bool> moved(a_table_size, false);

  old_copy = collect();
  while (true) {
    new_copy = collect();
    for (int j = 0; j < a_table_size; ++j) {
      if (old_copy[j].stamp != new_copy[j].stamp) {
        if (moved[j]) {
          return old_copy[j].snap;
        } else {
          moved[j] = true;
          old_copy = new_copy;
          continue;
        }
      }
    }

    std::vector<T> result(a_table_size, 0);
    for (int j = 0; j < a_table_size; ++j) {
      result[j] = new_copy[j].value;
    }
    return result;
  }
}
