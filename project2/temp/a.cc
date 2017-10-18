#include <iostream>
#include <cstdint>

typedef struct _dest_and_count {
  uint64_t dest;
  int64_t count;

  bool operator==(struct _dest_and_count &rhs) {
    return dest == rhs.dest;
  }
  bool operator!=(struct _dest_and_count &rhs) {
    return dest != rhs.dest;
  }
  bool operator<=(struct _dest_and_count &rhs) {
    return dest <= rhs.dest;
  }
  bool operator>=(struct _dest_and_count &rhs) {
    return dest >= rhs.dest;
  }
  bool operator<(struct _dest_and_count &rhs) {
    return dest < rhs.dest;
  }
  bool operator>(struct _dest_and_count &rhs) {
    return dest > rhs.dest;
  }
}dest_and_count_t;


int main(void)
{
  dest_and_count_t dummy1 = {2, 1};
  dest_and_count_t dummy2 = {3, 1};

  std::cout << bool(dummy1 == dummy2) << std::endl;
  std::cout << bool(dummy1 != dummy2) << std::endl;
  std::cout << bool(dummy1 <= dummy2) << std::endl;
  std::cout << bool(dummy1 >= dummy2) << std::endl;
  std::cout << bool(dummy1 < dummy2) << std::endl;
  std::cout << bool(dummy1 > dummy2) << std::endl;


  return 0;
}

