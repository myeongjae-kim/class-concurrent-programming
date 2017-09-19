#include <iostream>
#include <string>

int main(void)
{
  std::string a("");

  a += "Hello, World!|";

  // *(--a.end()) = '\0';

  std::cout << a << std::endl;

  return 0;
}
