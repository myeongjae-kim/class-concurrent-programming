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
#include <BloomFilter.h>

#include <vector>

#define RESERVED_CAPACITY 1024


void TestBloomFilter();
void TestFindSubstring();

int main(void)
{
  // TestBloomFilter();
  
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

  // To the end of stdin
  while (strBuffer.length() != 0) {

    switch (strBuffer[0]) {
      case 'Q':
        // get argument
        std::cin >> strBuffer;

#ifdef DBG
        std::cout << "(main) call query" << std::endl;
#endif

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

#ifdef DBG
      case 'R':
        // get argument
        std::cin >> strBuffer;

        std::cout << strBuffer << " is ";
        if(s.isRegistered(strBuffer)) {
          std::cout << "registered." << std::endl;
        } else {
          std::cout << "not registered." << std::endl;
        }

        break;
#endif

      default:
        ERROR_MSG("(in switch) Default case is not exist.\n");
#ifdef DBG
        assert(false);
#else
        return -1;
#endif
    }
    strBuffer.clear();

    // get next command
    std::cin >> strBuffer;
  }

#ifdef DBG
  s.printLengthsOfStringsInBloomFilter();
#endif

  return 0;
}

void TestBloomFilter() {
  
  uint64_t n = 0;
  std::cin >> n;
  std::string tempStr;
  std::getline(std::cin, tempStr);
  double errProb = 0.01;

  BloomFilter bf(n, errProb);

  std::vector<std::string> v;

  std::string strBuffer;
  strBuffer.reserve(RESERVED_CAPACITY);

  for (uint64_t i = 0; i < n; ++i) {
    // TODO: Parallelizing or not. Which one is fast?
    std::getline(std::cin, strBuffer);

    v.push_back(strBuffer);
    std::cout << strBuffer << std::endl;

    bf.insert(strBuffer);
    strBuffer.clear();
  }


  for (auto i : v) {
    if (bf.lookup(i)) {
      std::cout << i << " is found" << std::endl;
    } else {
      std::cout << i << " is not found" << std::endl;
    }
  }

  exit(0);
}
