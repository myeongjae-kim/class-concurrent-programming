
#include <iostream>
#include <unordered_map>

int main(int argc, char *argv[])
{
  std::unordered_map<int, int*> a;

  int b[256];
  a[0] = b;
    
  std::cout << bool(a.find(0) == a.end()) << std::endl;


  return 0;
}
