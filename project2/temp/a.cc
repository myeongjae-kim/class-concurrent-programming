#include <iostream>
#include <cstdint>
#include <vector>

int main(void)
{
  std::vector<int> v;

  v.push_back(1);
  v.push_back(2);

  auto iter = v.begin();
  auto before_begin = iter - 1;
  before_begin += 1;

  if (iter == before_begin) {
    std::cout << "same" << std::endl;
    std::cout << *iter << std::endl;
    std::cout << *before_begin << std::endl;
  } else {
    std::cout << "not same" << std::endl;
    std::cout << *iter << std::endl;
    std::cout << *before_begin << std::endl;
  }




  return 0;
}

