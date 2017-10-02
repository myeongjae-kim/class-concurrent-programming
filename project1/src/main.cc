#include "trie.h"

#include <iostream>

#define RESERVED_CAPACITY 1024


extern pthread_cond_t cond[THREAD_NUM];
extern pthread_mutex_t cond_mutex[THREAD_NUM];

extern pthread_t threads[THREAD_NUM];
extern thread_arg_t thread_args[THREAD_NUM];
extern bool thread_is_sleep[THREAD_NUM];

extern void* search_substring(void* arg);

extern bool finished;

int main(void)
{
  std::ios::sync_with_stdio(false);

  int number_of_strings = 0;
  std::cin >> number_of_strings;

  // reserve capacity for performance
  std::string str_buffer;
  str_buffer.reserve(RESERVED_CAPACITY);

  struct trie* root = create_trie_node();
  for (int i = 0; i < number_of_strings; ++i) {
    std::cin >> str_buffer;
    insert(&root, (char*)str_buffer.c_str());
  }

  // initializing cond and mutex
  for (int i = 0; i < THREAD_NUM; ++i) {
    pthread_cond_init(&cond[i], NULL);
    cond_mutex[i] = PTHREAD_MUTEX_INITIALIZER;
  }
  finished = false;

  for (long i = 0; i < THREAD_NUM; i++) {
    // create threads
    thread_is_sleep[i] = false;
    if (pthread_create(&threads[i], 0, search_substring, (void*)i) < 0) {
      std::cout << "thread create has been failed." << std::endl;
      return 0;
    }

    // wait for thread sleep
    while (thread_is_sleep[i] == false) {
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
        pthread_mutex_lock(&cond_mutex[i]);
        pthread_cond_broadcast(&cond[i]);
        pthread_mutex_unlock(&cond_mutex[i]);
      }
      break;
    }

    std::cin.get();

    // get argument
    getline(std::cin, str_buffer);
    switch (cmd) {
      case 'Q':
        search_all_patterns(root, (char*)str_buffer.c_str(), str_buffer.length());
        // setWasPrintedFalse(head);
        break;
      case 'A':
        insert(&root, (char*)str_buffer.c_str());
        break;
      case 'D':
        erase(&root, str_buffer.c_str());
        if (root == NULL) {
          root = create_trie_node();
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


  // Wait threads end
  for (int i = 0; i < THREAD_NUM; i++) {
    pthread_join(threads[i], NULL);
  }

  // free other memories
  erase_all(&root);
  return 0;
}
