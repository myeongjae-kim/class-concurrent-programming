// put every code into main for performance.

#include <main.h>


BMString::BMString(const std::string& pat)
  : std::string(pat), search(this->begin(), this->end()) {
    patLength = pat.length();

    // initialize bad match table
    for (long i = 0; i < ALPHA_NUM; ++i) {
      badMatchTable[i] = patLength;
    }


    // make bad match table
    for (long i = 0; i < patLength; ++i) {
      long jump = patLength - i - 1;
      if (jump == 0) {
        jump++;
      }
      badMatchTable[(uint8_t)pat[i]] = jump;
    }

  }

char* BMString::BMH(const std::string& qry) {
  long qryLength = qry.length();
  long i = patLength - 1;
  char last = (*this)[i];

  char qryChar;
  long firstCharIdx;


  while (i < qryLength) {
    qryChar = qry[i];
    if (qryChar == last) {
      firstCharIdx = i - patLength + 1; 
      if (qry.substr(firstCharIdx, patLength) == *this) {
        return (char*)firstCharIdx;
      }
    }
    i += badMatchTable[(uint8_t)qryChar];
  }

  return (char*)0xFFFFFFFFFFFFFFFF;
}


// global variables
std::set<BMString> patterns;

int main(void)
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
