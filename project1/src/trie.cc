#include <vector>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <cstdint>
#include "trie.h"

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


pthread_mutex_t vectorMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queryMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t threads[THREAD_NUM];
ThreadArg threadArgs[THREAD_NUM];

char* globalStrQuery = nullptr;
long globalIndex = 0;
long globalQueryLen = 0;

// argument is dynamically allocated.
void* searchForThread(void* tid) {
  ThreadArg data = threadArgs[uint64_t(tid)];

  Answer answerBuffer;

  char* str;
  struct Trie* trieNode;
  while (1) {
    trieNode = data.trieRoot;

    pthread_mutex_lock(&queryMutex);
    data.strQuery = globalStrQuery + globalIndex++;
    if (globalIndex > globalQueryLen) {
      pthread_mutex_unlock(&queryMutex);
      break;
    } else {
      pthread_mutex_unlock(&queryMutex);
    }

    
    str = data.strQuery;

    while (*str) {
      trieNode = trieNode->chars[*str - 'a'];

      if (trieNode == NULL) {
        break;
      } else if (trieNode -> wordID) {
        answerBuffer.startAdr = data.strQuery;
        answerBuffer.length = (uint32_t)(str - data.strQuery) + 1;
        answerBuffer.patternID = trieNode->wordID;

        // TODO: parallelize by using index.
        pthread_mutex_lock(&vectorMutex);
        answers.push_back(answerBuffer);
        pthread_mutex_unlock(&vectorMutex);
      }

      str++;
    }
  }

  return nullptr;
}

int searchAllPatterns(struct Trie* trieRoot, char* strQuery)
{
  // return 0 if Trie is empty
  if (trieRoot == NULL){
    return 0;
  }

  globalStrQuery = strQuery;
  globalQueryLen = strlen(strQuery);

  printed.clear();
  answers.clear();

  for (uint64_t i = 0; i < THREAD_NUM; ++i) {
    pthread_join(threads[i], NULL);
    threadArgs[i].strQuery = strQuery;
    threadArgs[i].trieRoot = trieRoot;

    pthread_create(&threads[i], NULL, searchForThread, (void*)i);
  }

  //wait threads
  for (int i = 0; i < THREAD_NUM; ++i) {
    pthread_join(threads[i], NULL);
    threads[i] = 0;
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
      std::cout << *printingTarget;
      printingTarget++;
    }

    //print the others
    for (uint32_t i = 1; i < size; ++i) {
      Answer& answer = answers[i];
      if (printed.find(answer.patternID) == printed.end()) {
        printed.insert(answer.patternID);
        printingTarget = answer.startAdr;
        std::cout << '|';
        for (uint32_t j = 0; j < answer.length; ++j) {
          std::cout << *printingTarget;
          printingTarget++;
        }
      }
    }

  } else {
    // no answer
    std::cout << "-1";
  }
  std::cout << std::endl;

  globalStrQuery = nullptr;
  globalQueryLen = 0;
  globalIndex = 0;

  //return has no meaning.
  return 1;
}




/* int searchAllPatterns(struct Trie* trieRoot, char* strQuery)
 * {
 *   // return 0 if Trie is empty
 *   if (trieRoot == NULL)
 *     return 0;
 *
 *   struct Trie* curr = trieRoot;
 *
 *   bool firstPrint = true;
 *   bool hasAnswer = false;
 *   printed.clear();
 *
 *   char* str;
 *   while (*strQuery) {
 *     str = strQuery;
 *
 *     while (*str)
 *     {
 *       // go to next node
 *       curr = curr->chars[*str - 'a'];
 *
 *       // if string is invalid (reached end of path in Trie)
 *       if (curr == NULL) {
 *         curr = trieRoot;
 *         break;
 *       } else if (curr -> wordID) {
 *         if (printed.find(curr->wordID) != printed.end()) {
 *           // do not print if it was printed.
 *         } else {
 *           printed.insert(curr->wordID);
 *           // print start
 *           if (!firstPrint) {
 *             std::cout << '|';
 *           }
 *
 *           char* print = strQuery;
 *           while(print <= str) {
 *             std::cout << *print;
 *             print++;
 *           }
 *           firstPrint = false;
 *           hasAnswer = true;
 *         }
 *       }
 *
 *       // move to next chars
 *       str++;
 *     }
 *
 *     curr = trieRoot;
 *     strQuery++;
 *   }
 *
 *
 *   if (hasAnswer == false) {
 *     std::cout << "-1";
 *   }
 *
 *   std::cout << '\n';
 *   // if current node is a leaf and we have reached the
 *   // end of the string, return 1
 *   return curr->wordID;
 * }
 *  */

// returns 1 if given trieNode has any children
int childExist(struct Trie* curr)
{
  for (int i = 0; i < ALPHA_NUM; i++)
    if (curr->chars[i])
      return 1;	// child found

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


// Trie Implementation in C - Insertion, Searching and erase
int TestTrie()
{
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
