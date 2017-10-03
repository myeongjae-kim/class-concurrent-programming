/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : main.cc
 * Due date             : 2017-10-01
 * Compilation Standard : c++11 */


/* A simple design document.
 *
 * Every input is stored at a trie data structure.
 *
 * For each query, all of its substring is searched
 *   to find words in the trie data structure.
 *
 *   e.g.) When a query is 'applepen'
 *
 *   applepen
 *   pplepen
 *   plepen
 *   lepen
 *   epen
 *   pen
 *   en
 *   n
 *
 *   Eight substring is searched.
 *
 *   In the worst case, the required time is O(m^2)
 *   (m is a length of a query) 
 *
 *   The deatiled documents are in the wiki.
 *
 *   */


#include "trie.h"

#include <iostream>

#define RESERVED_CAPACITY 1024

int main(void)
{
  // Disable IO sync with libc.
  // This makes huge speed up.
  std::ios::sync_with_stdio(false);

  // get the number of the initial inputs.
  int number_of_strings = 0;
  std::cin >> number_of_strings;

  // reserve capacity for performance
  std::string str_buffer;
  str_buffer.reserve(RESERVED_CAPACITY);

  // Create the trie.
  struct trie* root = create_trie_node();
  // Get inputs from stdin and insert it to the trie.
  for (int i = 0; i < number_of_strings; ++i) {
    std::cin >> str_buffer;
    insert(&root, (char*)str_buffer.c_str());
  }

  // initializing conds and mutexes.
  for (int i = 0; i < THREAD_NUM; ++i) {
    pthread_cond_init(&cond[i], NULL);
    cond_mutex[i] = PTHREAD_MUTEX_INITIALIZER;
  }

  // Set finished false.
  // It will be true at the end of workloads.
  finished = false;

  // create threads
  for (long i = 0; i < THREAD_NUM; i++) {
    thread_is_sleep[i] = false;
    if (pthread_create(&threads[i], 0, search_substring, (void*)i) < 0) {
      std::cout << "(main) thread creation has been failed." << std::endl;
      return 0;
    }

    // wait for sleep
    while (thread_is_sleep[i] == false) {
      pthread_yield();
    }
  }

  // Initialization finished.
  // Get workloads.
  std::cout << 'R' << std::endl;

  // To the end of stdin
  char cmd;
  while (1) {

    if ( !(std::cin >> cmd) ) {
      // below instruction are executed when 'EOF' is inputted.

      // turn on 'finished' to destory all threads.
      finished = true;

      // Wake up all threads to be terminated
      for (int i = 0; i < THREAD_NUM; ++i) {
        pthread_mutex_lock(&cond_mutex[i]);
        pthread_cond_broadcast(&cond[i]);
        pthread_mutex_unlock(&cond_mutex[i]);
      }
      break;
    }

    // Remove space bar between a command and a argument.
    std::cin.get();

    // get argument
    getline(std::cin, str_buffer);
    switch (cmd) {
      case 'Q':
        // Do searching
        search_all_patterns(root, str_buffer.c_str(), str_buffer.length());
        break;
      case 'A':
        // Add new word to the trie.
        insert(&root, str_buffer.c_str());
        break;
      case 'D':
        // Delete a word in the trie.
        erase(&root, str_buffer.c_str());

        // If root is removed, recreate it.
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
