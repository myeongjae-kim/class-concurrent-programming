// TestAhoCorasik.cpp : Defines the entry point for the console application.
//

#include "StdAfx.h"
#include "SuffixTrie.h"

#include <stdio.h>
#include <iostream>
#include <cstdint>

void TestAhoCoarsik(void){
	CSuffixTrie aTree;
	aTree.AddString("barak");
	aTree.AddString("arakoo");
	aTree.AddString("barakoo");
	aTree.AddString("barako565");

  aTree.AddString("apple");
  aTree.AddString("pineapple");
  aTree.AddString("leap");

  aTree.BuildTreeIndex();
  CSuffixTrie::DataFoundVector aDataFound;
  // aDataFound=aTree.SearchAhoCorasikMultiple("1236h6h6barakoo6arakoo123");
  aDataFound=aTree.SearchAhoCorasikMultiple("penpineappleapplepen");

  for (uint64_t iCount=0;
      iCount<aDataFound.size();
      ++iCount)
    printf("%s %i\n",aDataFound[iCount].sDataFound.c_str(),aDataFound[iCount].iFoundPosition);



}
