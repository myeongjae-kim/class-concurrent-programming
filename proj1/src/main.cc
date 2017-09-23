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
#include <Signal.h>

#include <vector>

#define RESERVED_CAPACITY 1024


int main(void)
{
  std::ios::sync_with_stdio(false);
  
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

  /* int counter = 1;
   * for (auto i : s.inputWords) {
   *   std::cout << counter << " : "<< i << std::endl;
   *   counter++;
   * }
   * exit(0); */


  // To the end of stdin
  char cmd;
  while (std::cin >> cmd) {
    std::cin.get();
    
    // get argument
    getline(std::cin, strBuffer);
    switch (cmd) {
      case 'Q':
#ifdef DBG
        std::cout << "(main) call query" << std::endl;
#endif
        std::cout << s.query(strBuffer) << std::endl;
        break;
      case 'A':
        s.add(strBuffer);
        break;
      case 'D':
        s.del(strBuffer);
        break;

      case 'R':
        // get argument
        std::cout << strBuffer << " is ";
        if(s.isRegistered(strBuffer)) {
          std::cout << "registered." << std::endl;
        } else {
          std::cout << ANSI_COLOR_RED "not registered." ANSI_COLOR_RESET << std::endl;
        }

        break;

      default:
        ERROR_MSG("(in switch) Default case is not exist.\n");
#ifdef DBG
        assert(false);
#else
        return -1;
#endif
    }
    strBuffer.clear();
  }

#ifdef DBG
  s.printLengthsOfStringsInBloomFilter();
#endif

  return 0;
}
