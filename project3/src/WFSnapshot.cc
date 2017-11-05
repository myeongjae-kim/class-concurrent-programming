/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : WFSnapshot.cc
 * Due date             : 2017-11-6
 * Compilation Standard : c++11 */

#include "WFSnapshot.h"

extern int64_t num_of_threads;


static void delete_collection(stamped_snap_t** table) {
  for (int i = 0; i < num_of_threads; ++i) {
    delete table[i];
  }
  delete[] table;
}

WFSnapshot_t::WFSnapshot_t (int capacity, T init) {
  a_table = new stamped_snap_t*[capacity];
  for (int i = 0; i < capacity; ++i) {
    a_table[i] = new stamped_snap_t(init);
  }

  this->capacity = capacity;
}

stamped_snap_t** WFSnapshot_t::collect() {
  stamped_snap_t** copy = new stamped_snap_t*[capacity];
  for (int i = 0; i < capacity; ++i) {
    copy[i] = new stamped_snap_t(a_table[i]->stamp,
        a_table[i]->value,
        a_table[i]->snap);
  }

  return copy;
}

WFSnapshot_t::~WFSnapshot_t (){
  for (int i = 0; i < capacity; ++i) {
    delete a_table[i];
  }
  delete[] a_table;
}


void WFSnapshot_t::update(T value, const int64_t tid){
  T* snap = scan();

  a_table[tid]->stamp++;
  a_table[tid]->value = value;

  T* to_be_deleted = a_table[tid]->snap;
  a_table[tid]->snap = snap;
  delete[] to_be_deleted;
}


T* WFSnapshot_t::scan() {
  stamped_snap_t** old_copy;
  stamped_snap_t** new_copy;

  bool *moved = new bool[num_of_threads];
  memset(moved, 0, sizeof(*moved) * num_of_threads);

  old_copy = collect();
collect:
  while (true) {
    new_copy = collect();
    for (int j = 0; j < num_of_threads; ++j) {
      if (old_copy[j]->stamp != new_copy[j]->stamp) {
        if (moved[j]) {

          T* return_snap = new T[num_of_threads];
          memcpy(return_snap, old_copy[j]->snap,
              sizeof(*return_snap) * num_of_threads);

          delete_collection(new_copy);
          delete_collection(old_copy);

          return return_snap;
        } else {
          moved[j] = true;

          delete_collection(old_copy);
          old_copy = nullptr;

          old_copy = new_copy;
          new_copy = nullptr;
          goto collect;
        }
      }
    }

    delete[] moved;
    moved = nullptr;

    T* result = new T[num_of_threads];
    for (int j = 0; j < num_of_threads; ++j) {
      result[j] = new_copy[j]->value;
    }

    delete_collection(old_copy);
    old_copy = nullptr;
    delete_collection(new_copy);
    new_copy = nullptr;


    return result;
  }
}
