#include <set>
#include <utility>
#include <iostream>

int main(void)
{
  std::pair<int, int> a[5];

  a[0] = std::pair<int, int>(10, 20);
  a[1] = std::pair<int, int>(11, 22);
  a[2] = std::pair<int, int>(5, 10);
  a[3] = std::pair<int, int>(11, 22);
  a[4] = std::pair<int, int>(10, 20);

  std::set< std::pair<int, int> > s;

  for (int i = 0; i < 5; ++i) {
    s.insert(a[i]);
  }

  for (auto i : s) {
    std::cout << i.first << ", " << i.second << std::endl;
  }
  
  return 0;
}
