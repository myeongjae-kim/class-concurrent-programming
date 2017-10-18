#include <iostream>
#include <cstdint>

int main(void)
{
  enum Color : uint8_t { red, green, blue };

  std::cout << sizeof(Color) << std::endl;

  return 0;
}

