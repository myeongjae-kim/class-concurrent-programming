#include <iostream>
#include <vector>

int main(void)
{
  std::vector<uint64_t> v;
  v.push_back(1);
  v.push_back(2);
  v.push_back(3);

  auto last_elem = v.end() - 1;

  std::cout << *last_elem << std::endl;

  v.erase(v.begin());
  v.erase(v.begin());
  v.erase(v.begin());

  std::cout << *last_elem << std::endl;

  // v.erase(last_elem);

  std::cout << v.size() << std::endl;

  return 0;
}
