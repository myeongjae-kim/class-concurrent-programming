#include <iostream>

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include "trie.h"

#include <vector>
#include <unordered_set>


uint32_t patternID = 1;
std::unordered_set <uint32_t> printed;

struct Trie* createTrieNode() {
  // create new node
	struct Trie* node = (struct Trie*)calloc(1, sizeof(*node));
	return node;
}

void insert(struct Trie* *trieRoot, char* str)
{
	struct Trie* trieNode = *trieRoot;
	while (*str)
	{
		if (trieNode->chars[*str - 'a'] == NULL){
			trieNode->chars[*str - 'a'] = createTrieNode();
    }

		trieNode = trieNode->chars[*str - 'a'];
		str++;
	}

	trieNode->wordID = patternID++;
}

int search(struct Trie* trieRoot, char* str)
{
  if (trieRoot == NULL){
    return 0;
  }

  struct Trie* trieNode = trieRoot;
  while (*str)
  {
    trieNode = trieNode->chars[*str - 'a'];

    if (trieNode == NULL) {
      return 0;
    }

    str++;
  }

  return trieNode->wordID;
}

int searchAllPatterns(struct Trie* trieRoot, char* strQuery)
{
	// return 0 if Trie is empty
	if (trieRoot == NULL)
		return 0;

	struct Trie* curr = trieRoot;

  bool firstPrint = true;
  bool hasAnswer = false;
  printed.clear();

  char* str;
  while (*strQuery) {
    str = strQuery;

    while (*str)
    {
      // go to next node
      curr = curr->chars[*str - 'a'];

      // if string is invalid (reached end of path in Trie)
      if (curr == NULL) {
        curr = trieRoot;
        break;
      } else if (curr -> wordID) {
        if (printed.find(curr->wordID) != printed.end()) {
          // do not print if it was printed.
        } else {
          printed.insert(curr->wordID);
          // print start
          if (!firstPrint) {
            putchar('|');
          }

          char* print = strQuery;
          while(print <= str) {
            putchar(*print);
            print++;
          }
          firstPrint = false;
          hasAnswer = true;
        }
      }

      // move to next chars
      str++;
    }

    curr = trieRoot;
    strQuery++;
  }


  if (hasAnswer == false) {
    printf("-1");
  }

  putchar('\n');
  // if current node is a leaf and we have reached the
  // end of the string, return 1
  return curr->wordID;
}


int childExist(struct Trie* trieNode)
{
  for (int i = 0; i < ALPHA_NUM; i++){
    if (trieNode->chars[i]){
      return 1;
    }
  }

  return 0;
}

int erase(struct Trie* *trieNode, char* str) {
  if (*trieNode == NULL){
    return 0;
  }

  if (*str) {
    // recursively find target node

    // when node is not null
    if (*trieNode != NULL &&

        // and it has a node to target string
        (*trieNode)->chars[*str - 'a'] != NULL &&

        // find next chars recursively and erase it.
        erase(&((*trieNode)->chars[*str - 'a']), str + 1) &&

        // if current node is not the end of string
        (*trieNode)->wordID == 0) {


      // erase node if it has no children node.
      if (!childExist(*trieNode)) {
        free(*trieNode);
        (*trieNode) = NULL;
        return 1;
      } else {
        return 0;
      }
    }
  }

  // this is a case
  if (*str == '\0' && (*trieNode)->wordID) {
    if (!childExist(*trieNode)) {
      // when it is leaf node
      // remove
      free(*trieNode);

      // remove
      (*trieNode) = NULL;
      return 1;
    } else {
      // when it is not leaf node
      // do not remove. just makes wordID zero.
      (*trieNode)->wordID = 0;
      return 0;
    }
  }

  return 0;
}

int TestTrie() {
  struct Trie* trieRoot = createTrieNode();

  insert(&trieRoot, (char*)"app");
  insert(&trieRoot, (char*)"apple");
  insert(&trieRoot, (char*)"pineapple");
  insert(&trieRoot, (char*)"leap");

  searchAllPatterns(trieRoot, (char*)"apple");
  // setWasPrintedFalse(trieRoot);
  searchAllPatterns(trieRoot, (char*)"pineapple");
  // setWasPrintedFalse(trieRoot);
  // searchAllPatterns(trieRoot, "penpineappleapplepen");


  return 0;
}
