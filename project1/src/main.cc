#include "trie.h"

#include <iostream>

#define RESERVED_CAPACITY 1024




extern pthread_cond_t cond[THREAD_NUM];
extern pthread_mutex_t condMutex[THREAD_NUM];

extern pthread_mutex_t vectorMutex;

extern pthread_t threads[THREAD_NUM];
extern ThreadArg threadArgs[THREAD_NUM];
extern bool threadIsSleep[THREAD_NUM];

extern void* searchSubstring(void* arg);

extern bool finished;


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

  // initializing cond and mutex
  for (int i = 0; i < THREAD_NUM; ++i) {
    pthread_cond_init(&cond[i], NULL);
    condMutex[i] = PTHREAD_MUTEX_INITIALIZER;
  }
  finished = false;

  for (long i = 0; i < THREAD_NUM; i++) {
    // create threads
    threadIsSleep[i] = false;
    if (pthread_create(&threads[i], 0, searchSubstring, (void*)i) < 0) {
      std::cout << "thread create has been failed." << std::endl;
      return 0;
    }

    // wait for thread sleep
    while (threadIsSleep[i] == false) {
      pthread_yield();
    }
  }


  std::cout << 'R' << std::endl;

  // To the end of stdin
  char cmd;
  while (1) {
    if ( !(std::cin >> cmd)) {
      finished = true;

      // Wake up all threads to terminate
      for (int i = 0; i < THREAD_NUM; ++i) {
        pthread_mutex_lock(&condMutex[i]);
        pthread_cond_broadcast(&cond[i]);
        pthread_mutex_unlock(&condMutex[i]);
      }
      break;
    }

    std::cin.get();

    // get argument
    getline(std::cin, strBuffer);
    switch (cmd) {
      case 'Q':



        searchAllPatterns(head, (char*)strBuffer.c_str(), strBuffer.length());
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
  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_join(threads[i], NULL);
  }

  return 0;
}
