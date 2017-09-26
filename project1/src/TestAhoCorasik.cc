// TestAhoCorasik.cpp : Defines the entry point for the console application.
//

#include "StdAfx.h"

#include <stdio.h>

#include "SuffixTrie.h"


#include <cstdint>
#include <iostream>

int main(int argc, char* argv[])
{
	CSuffixTrie aTree;
	aTree.AddString(L"barak");
	aTree.AddString(L"arakoo");
	aTree.AddString(L"barakoo");
	aTree.AddString(L"barako565");
	
	aTree.BuildTreeIndex();
	CSuffixTrie::DataFoundVector aDataFound;
	aDataFound=aTree.SearchAhoCorasikMultiple(L"1236h6h6barakoo6arakoo123");

	for (uint64_t iCount=0;
		 iCount<aDataFound.size();
		 ++iCount) {
		printf("%S %i\n",aDataFound[iCount].sDataFound.c_str(),aDataFound[iCount].iFoundPosition);
  }


	return 0;
}
