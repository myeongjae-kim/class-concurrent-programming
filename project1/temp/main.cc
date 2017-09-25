// put every code into main for performance.

#include <main.h>

// global variables
std::set<BMString> patterns;

int main(int argc, char *argv[])
{
  std::ios::sync_with_stdio(false);

  int numberOfStrings = 0;
  std::cin >> numberOfStrings;

  // reserve capacity for performance
  std::string strBuffer;
  strBuffer.reserve(RESERVED_CAPACITY);

  for (int i = 0; i < numberOfStrings; ++i) {
    std::cin >> strBuffer;
    patterns.insert(*(new BMString(strBuffer)));
  }

  // insert complete
  std::cout << 'R' << std::endl;


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
        std::cout << query(strBuffer) << std::endl;
        break;
      case 'A':
        patterns.insert(*(new BMString(strBuffer)));
        break;
      case 'D':
        patterns.erase(strBuffer);
        break;

      case 'R':
        // get argument
        std::cout << strBuffer << " is ";
        if(patterns.find(strBuffer) != patterns.end()) {
          std::cout << "registered." << std::endl;
        } else {
          std::cout << "not registered."  << std::endl;
        }

        break;

      default:
        std::cout << "(in switch) Default case is not exist.\n" << std::endl;
#ifdef DBG
        assert(false);
#else
        return -1;
#endif
    }
  }

  return 0;
}
