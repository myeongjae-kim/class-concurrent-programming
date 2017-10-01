#include <iostream>

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include "trie.h"

#include <vector>
#include <unordered_set>


uint32_t patternID = 1;
std::vector < Answer > answers;
std::vector < Answer > localAnswers[THREAD_NUM];

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

/* int search(struct Trie* trieRoot, char* str)
 * {
 *   if (trieRoot == NULL){
 *     return 0;
 *   }
 *
 *   struct Trie* trieNode = trieRoot;
 *   while (*str)
 *   {
 *     trieNode = trieNode->chars[*str - 'a'];
 *
 *     if (trieNode == NULL) {
 *       return 0;
 *     }
 *
 *     str++;
 *   }
 *
 *   return trieNode->wordID;
 * } */

// struct for parallelizing

pthread_cond_t cond;
pthread_mutex_t condMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t vectorMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_t threads[THREAD_NUM];
ThreadArg threadArgs[THREAD_NUM];
bool threadIsSleep[THREAD_NUM];

bool finished;


void* searchSubstring(void* arg) {
  long tid = (long)arg;

  pthread_mutex_lock(&condMutex);
  threadIsSleep[tid] = true;
  pthread_cond_wait(&cond, &condMutex);
  pthread_mutex_unlock(&condMutex);


  while (!finished) {
    localAnswers[tid].clear();
    if (threadIsSleep[tid] == false && threadArgs[tid].strQuery) {
      ThreadArg data = threadArgs[uint64_t(tid)];

      struct Trie* trieNode = data.trieRoot;
      Answer answerBuffer;

      std::string query(data.strQuery);

      uint32_t searchCount = 0;
      char* str;
      while (searchCount < data.searchLength && *data.strQuery) {
        str = data.strQuery;

        while (*str) {
          trieNode = trieNode->chars[*str - 'a'];

          if (trieNode == NULL) {
            trieNode = data.trieRoot;
            break;
          } else if (trieNode -> wordID) {
            answerBuffer.startAdr = data.strQuery;
            answerBuffer.length = (uint32_t)(str - data.strQuery) + 1;
            answerBuffer.patternID = trieNode->wordID;

            localAnswers[tid].push_back(answerBuffer);
          }

          str++;
        }

        trieNode = data.trieRoot;
        data.strQuery++;
        searchCount++;
      }
    }



sleep:
    pthread_mutex_lock(&condMutex);
    threadIsSleep[tid] = true;
    pthread_cond_wait(&cond, &condMutex);
    // Waked up
    pthread_mutex_unlock(&condMutex);

    if (threadIsSleep[tid] == true) {
      if (finished) {
        break;
      } else {
        goto sleep;
      }
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

  printed.clear();
  answers.clear();

  // prepare arguments for theads

  uint64_t tid = THREAD_NUM - 1;
  uint32_t numberOfThreadRun = (strlen(strQuery) / SEARCH_ITER_NUM) + 1;
  for (uint64_t i = 0; i < numberOfThreadRun; ++i) {
    tid = i % THREAD_NUM;
    threadArgs[tid].strQuery = strQuery;
    threadArgs[tid].trieRoot = trieRoot;
    threadArgs[tid].searchLength = SEARCH_ITER_NUM;

    // full of threads. Run!
    if (tid == THREAD_NUM - 1) {

      // wake up threads
      // thread reinit for start
      for (int i = 0; i < THREAD_NUM; ++i) {
        threadIsSleep[i] = false;
      }

      // Wake up all threads to work
      pthread_mutex_lock(&condMutex);
      pthread_cond_broadcast(&cond);
      pthread_mutex_unlock(&condMutex);


      // Wait for all threads to finish work
      while (1) {
        bool all_thread_done = true;
        for (uint32_t i = 0; i < THREAD_NUM; i++) {
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

    }

    strQuery += SEARCH_ITER_NUM;
  }

  // there is left threads. Run them.
  if (tid != THREAD_NUM - 1) {

    // wake up threads
    // thread reinit for start
    for (uint32_t i = 0; i < THREAD_NUM; ++i) {
      threadIsSleep[i] = false;
    }

    for (int i = tid + 1; i < THREAD_NUM; ++i) {
      threadArgs[i].strQuery = nullptr;
    }


    // Wake up all threads to work
    pthread_mutex_lock(&condMutex);
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&condMutex);


    // Wait for all threads to finish work
    while (1) {
      bool all_thread_done = true;
      for (uint32_t i = 0; i < THREAD_NUM; i++) {
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
  }

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

  //return has no meaning.
  return 1;
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
