#include "trie.h"

#include <iostream>

#define RESERVED_CAPACITY 1024

int main(void)
{
  // TestTrie();

  std::ios::sync_with_stdio(false);

  int numberOfStrings = 0;
  std::cin >> numberOfStrings;

  // reserve capacity for performance
  std::string strBuffer;
  strBuffer.reserve(RESERVED_CAPACITY);

  struct Trie* head = createTrieNode();
  for (int i = 0; i < numberOfStrings; ++i) {
    std::cin >> strBuffer;
    insert(&head, (char*)strBuffer.c_str());
  }

  std::cout << 'R' << std::endl;

  // To the end of stdin
  char cmd;
  while (1) {
    if ( !(std::cin >> cmd)) {
      break;
    }

    std::cin.get();

    // get argument
    getline(std::cin, strBuffer);
    switch (cmd) {
      case 'Q':



        searchAllPatterns(head, (char*)strBuffer.c_str());
        // setWasPrintedFalse(head);
        break;
      case 'A':
        insert(&head, (char*)strBuffer.c_str());
        break;
      case 'D':
        erase(&head, (char*)strBuffer.c_str());
        if (head == NULL) {
          head = createTrieNode();
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

  // free other memories

  // Wait threads end

  return 0;
}
