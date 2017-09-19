/** 
 * Author               : Kim, Myeong Jae
 * File Name            : main.cc
 * Due date             : 2017-09-30
 * Compilation Standard : c11
 **/

#include <iostream>
#include <cstdlib>
#include <cassert>
#include <string>

#include <main.h>
#include <signal.h>

#define RESERVED_CAPACITY 1024

int main(void)
{
  int numberOfStrings = 0;
  std::cin >> numberOfStrings;

  Signal s(numberOfStrings);

  // reserve capacity for performance
  std::string strBuffer;
  strBuffer.reserve(RESERVED_CAPACITY);

  for (int i = 0; i < numberOfStrings; ++i) {
    // TODO: Parallelizing or not. Which one is fast?
    std::cin >> strBuffer;
    s.add(strBuffer);
    strBuffer.clear();
  }

  // insert complete
  std::cout << 'R' << std::endl;

  // get command
  std::cin >> strBuffer;
  while (strBuffer.length() != 0) {

    switch (strBuffer[0]) {
      case 'Q':
        // get argument
        std::cin >> strBuffer;

        std::cout << s.query(strBuffer) << std::endl;
        break;
      case 'A':
        // get argument
        std::cin >> strBuffer;

        s.add(strBuffer);
        break;
      case 'D':
        // get argument
        std::cin >> strBuffer;

        s.del(strBuffer);
        break;
      default:
        ERROR_MSG("(in switch) Default case is not exist.\n");
        assert(false);
    }

    strBuffer.clear();
    // get next command
    std::cin >> strBuffer;
  }

  printf("Hello, world!\n");
  return 0;
}
