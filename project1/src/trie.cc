
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include "trie.h"

#include <vector>
#include <unordered_set>


uint32_t patternID = 1;
std::vector < Answer > answers;

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
	if (trieRoot == NULL){
		return 0;
  }

	struct Trie* trieNode = trieRoot;

  Answer answerBuffer;
  printed.clear();
  answers.clear();

  char* str;
  while (*strQuery) {
    str = strQuery;

    while (*str) {
      trieNode = trieNode->chars[*str - 'a'];

      if (trieNode == NULL) {
        trieNode = trieRoot;
        break;
      } else if (trieNode -> wordID) {
        answerBuffer.startAdr = strQuery;
        answerBuffer.length = (uint32_t)(str - strQuery) + 1;
        answerBuffer.patternID = trieNode->wordID;

        // TODO: parallelize by using index.
        answers.push_back(answerBuffer);
      }

      str++;
    }

    trieNode = trieRoot;
    strQuery++;
  }

  // TODO:print
  // sort answer vector
  // check whether the answer was printed
  // if answer is not printed, print answer and add to printed set
  // clear printed set and answer vector


  char* printingTarget;
  uint32_t size = answers.size();
  if (size != 0) {
    // answer exists
    std::sort(answers.begin(), answers.end(), [](Answer lhs, Answer rhs){
          if (lhs.startAdr < rhs.startAdr) {
            return true;
          } else if (lhs.startAdr > rhs.startAdr) {
            return false;
          } else {
            if (lhs.length < rhs.length) {
              return true;
            } else {
              return false;
            }
          }
        });

    //print firstone
    Answer& tempAnswer = answers[0];
    printed.insert(tempAnswer.patternID);
    printingTarget = tempAnswer.startAdr;
    for (uint32_t i = 0; i < tempAnswer.length; ++i) {
      putchar(*printingTarget);
      printingTarget++;
    }

    //print the others
    for (uint32_t i = 1; i < size; ++i) {
      Answer& answer = answers[i];
      if (printed.find(answer.patternID) == printed.end()) {
        printed.insert(answer.patternID);
        printingTarget = answer.startAdr;
        putchar('|');
        for (uint32_t j = 0; j < answer.length; ++j) {
          putchar(*printingTarget);
          printingTarget++;
        }
      }
    }

  } else {
    // no answer
    printf("-1");
  }
  putchar('\n');

  return trieNode->wordID;
}


int haveChildren(struct Trie* trieNode)
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

  if (*str)
  {
    // Erase is so hard that I referenced this web site.
    // base source code: http://www.techiedelight.com/trie-implementation-insert-search-delete/

    // recursively find target node
    if (*trieNode != NULL && (*trieNode)->chars[*str - 'a'] != NULL &&
        erase(&((*trieNode)->chars[*str - 'a']), str + 1) &&
        (*trieNode)->wordID == 0) {

      // character found
      if (!haveChildren(*trieNode)) {
        free(*trieNode);
        (*trieNode) = NULL;
        return 1;
      } else {
        return 0;
      }
    }
  }

  if (*str == '\0' && (*trieNode)->wordID) {
    if (!haveChildren(*trieNode)) {
      // when it is leaf node
      // remove
      free(*trieNode);
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
