#include <iostream>
#include <cstdio>
#include <cstdint>

#include "main.h"
#include "StdAfx.h"
#include "SuffixTrie.h"

using namespace std;

int main(void)
{
  TestAhoCoarsik();
  exit(0);

  std::ios::sync_with_stdio(false);

  int numberOfStrings = 0;
  std::cin >> numberOfStrings;

  // reserve capacity for performance
  std::string strBuffer;
  strBuffer.reserve(RESERVED_CAPACITY);

	CSuffixTrie aTree;
  for (int i = 0; i < numberOfStrings; ++i) {
    std::cin >> strBuffer;
    aTree.AddString(strBuffer);
  }
	aTree.BuildTreeIndex();

  // insert complete
  std::cout << 'R' << std::endl;


  CSuffixTrie::DataFoundVector aDataFound;
  std::string searchResult("");

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
        aDataFound = aTree.SearchAhoCorasikMultiple(strBuffer);
        for (auto data : aDataFound) {
          searchResult = data.sDataFound + "|";
        }

        if (searchResult.length() != 0) {
          searchResult.pop_back();
        } else {
          searchResult = "-1";
        }

        std::cout << searchResult << std::endl;

        searchResult.clear();
        aDataFound.clear();
        break;
      case 'A':
        aTree.AddString(strBuffer);
        aTree.BuildTreeIndex();
        break;
      case 'D':
        aTree.DeleteString(strBuffer);
        aTree.BuildTreeIndex();
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
