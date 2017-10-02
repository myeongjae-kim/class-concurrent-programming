#include <iostream>

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include "trie.h"

#include <vector>
#include <unordered_set>

uint32_t patternID = 1;

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
    // check wheter trie already has a character of current word.
		if (trieNode->chars[*str - 'a'] == NULL){
			trieNode->chars[*str - 'a'] = createTrieNode();
    }

    //if not, create
		trieNode = trieNode->chars[*str - 'a'];

    //go to next character.
		str++;
	}

  // Every word has its unique ID. It is used not to print duplicated result.
	trieNode->wordID = patternID++;
}

// Variables for parallelizing.

// for sleep and wake for threads.
pthread_cond_t cond[THREAD_NUM];
pthread_mutex_t condMutex[THREAD_NUM];

// Threads and thread arguments.
pthread_t threads[THREAD_NUM];
ThreadArg threadArgs[THREAD_NUM];

// It shows whether a thread is in sleep of not.
bool threadIsSleep[THREAD_NUM];

// If it is true, threads will be terminated.
bool finished;


// This vector collects answers from localAnswers.
std::vector < Answer > answers;

// Every thread has its own answer vector for avoid locking.
std::vector < Answer > localAnswers[THREAD_NUM];

// It stores already printed ID of words (wordID).
std::unordered_set <uint32_t> printed;

void* searchSubstring(void* arg) {
  long tid = (long)arg;
  localAnswers[tid].clear();

  // Go to sleep when it is made.
  pthread_mutex_lock(&condMutex[tid]);
  threadIsSleep[tid] = true;
  pthread_cond_wait(&cond[tid], &condMutex[tid]);
  pthread_mutex_unlock(&condMutex[tid]);

  // If finished has 'true', the thread will be terminated.
  while (!finished) {
    // waken up!

    // If thread is not in sleep and it has query, search substring
    if (threadIsSleep[tid] == false && threadArgs[tid].strQuery) {
      ThreadArg data = threadArgs[tid];

      struct Trie* trieNode = data.trieRoot;
      Answer answerBuffer;

      uint32_t searchCount = 0;
      char* str;

      // iterate its own cake.           and break when it is on the end of the query.
      while (searchCount < data.searchLength && *data.strQuery) {

        // str is an iterating pointer that is exploring the trie.
        str = data.strQuery;

        // to the end of the query
        while (*str) {
          trieNode = trieNode->chars[*str - 'a'];

          if (trieNode == NULL) {
            // when the substring is not exist in the trie,
            // finish current search and go to next character of the query and search again.
            trieNode = data.trieRoot;
            break;
          } else if (trieNode -> wordID) {
            // When a word is exist,
            // Add information to answerBuffer
            answerBuffer.startAdr = data.strQuery;
            answerBuffer.length = (uint32_t)(str - data.strQuery) + 1;
            answerBuffer.patternID = trieNode->wordID;

            // and store in the localAnswer vector.
            localAnswers[tid].push_back(answerBuffer);
          }

          // go to next character of substring.
          str++;
        }

        // node reinitialization. Go to root.
        trieNode = data.trieRoot;

        // go to next substring
        data.strQuery++;

        // incrase iteration number.
        searchCount++;
      }
    }

    // search is finished.
    // go to sleep
    pthread_mutex_lock(&condMutex[tid]);
    threadIsSleep[tid] = true;
    pthread_cond_wait(&cond[tid], &condMutex[tid]);
    pthread_mutex_unlock(&condMutex[tid]);

    // Wake up!
    
  }


  return nullptr;
}

void searchAllPatterns(struct Trie* trieRoot, char* strQuery, uint32_t strLength)
{
  // return 0 if Trie is empty
  if (trieRoot == NULL){
    return;
  }

  // initialize data structures.
  printed.clear();
  answers.clear();


  char* endOfString = strQuery + strLength;

  // the number of iteration of each thread.
  uint32_t SEARCH_ITER_NUM = strLength / THREAD_NUM + 1;

  // prepare arguments for theads
  uint64_t tid = 0;
  for (uint64_t i = 0; i < THREAD_NUM && strQuery < endOfString; ++i) {
    tid = i % THREAD_NUM;
    threadArgs[tid].strQuery = strQuery;
    threadArgs[tid].trieRoot = trieRoot;
    threadArgs[tid].searchLength = SEARCH_ITER_NUM;

    strQuery += SEARCH_ITER_NUM;
  }

  // Run Threads
  // wake up threads
  // thread reinit for start
  for (uint32_t i = 0; i <= tid ; ++i) {
    threadIsSleep[i] = false;
  }

  for (int i = tid + 1; i <= THREAD_NUM; ++i) {
    threadArgs[i].strQuery = nullptr;
  }


  // Wake up all threads to work
  // Don't wake up threads that does not have arguments.
  for (uint32_t i = 0; i <= tid; ++i) {
    pthread_mutex_lock(&condMutex[i]);
    pthread_cond_signal(&cond[i]);
    pthread_mutex_unlock(&condMutex[i]);
  }


  // Wait for all threads to finish work
  while (1) {
    bool all_thread_done = true;
    for (uint32_t i = 0; i <= tid; i++) {
      if (threadIsSleep[i] == false) {
        all_thread_done = false;
        break;
      }
    }
    if (all_thread_done) {
      break;
    }
    pthread_yield();
  }

  // TODO:print
  // sort answer vector
  // check whether the answer was printed
  // if answer is not printed, print answer and add to printed set
  // clear printed set and answer vector


  //collect answers
  for (int i = 0; i < THREAD_NUM; ++i) {
    for (auto answer : localAnswers[i]) {
      answers.push_back(answer);
    }
    localAnswers[i].clear();
  }

  char* printingTarget;
  uint32_t size = answers.size();
  if (size != 0) {
    // answer exists
    // sort!
    std::sort(answers.begin(), answers.end(), [](Answer lhs, Answer rhs){
        // sort by substring's start location
        if (lhs.startAdr < rhs.startAdr) {
          return true;
        } else if (lhs.startAdr > rhs.startAdr) {
          return false;
        } else {
          // if substring's start location is same,
          // a short length precedes a long one.
          if (lhs.length < rhs.length) {
            return true;
            } else {
            return false;
            }
          }
        });

    //print first one
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

  return;
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

        // find next character recursively and erase it.
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
