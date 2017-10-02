/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : trie.h
 * Due date             : 2017-10-01
 * Compilation Standard : c++11 */

#ifndef TRIE_H
#define TRIE_H

// The number of characters the trie handles.
#define ALPHA_NUM 26

// The number of threads
#define THREAD_NUM 42

// #define DBG

#include <cstdint>
#include <pthread.h>

#ifdef DBG
#include <cassert>
#endif


// The structure of the answer.
// An answer does not have its own string.
// It dereferences a query for memory efficiency.
// Each answer needs only 16bytes.
typedef struct _answer {
  const char* substring_location;
  uint32_t length;
  uint32_t pattern_id;
} answer_t;



// Trie data structure
struct trie
{
  /* It is zero when character is not the end of a word. */
	int word_id;	

	struct trie* chars[ALPHA_NUM];
};

// This function creates a trie node.
// (Memory Allocation)
struct trie* create_trie_node();

// This function destory whole trie strucutre.
// (Memory deallocation)
void erase_all(struct trie* *trie_node);

// This function inserts a word to the trie structure iteratively.
void insert(struct trie* *trie_head, const char* str);

// This function erases a word in the trie structure.
// It returns true if erasing is successful.
bool erase(struct trie* *trie_node, const char* str);




// This function prints all of found words.
void search_all_patterns(struct trie* trie_root,
    const char* str_query,
    const uint32_t str_length);






// Thread global variables.

// Thread arguments have three information.
// 'trie_root' is a root of trie data structure.
// 'substring_location' is a substring of the query.
// 'search_length' is the number of substrings that a thread should search.
typedef struct _thread_arg {
  struct trie* trie_root; // this value could be a global variable.
  const char* substring_location;
  uint32_t search_length;
} thread_arg_t;

// Below variables are declared in trie.cc

// Variables for parallelizing.
// for conditional sleep and wake up threads.
extern pthread_cond_t cond[THREAD_NUM];
extern pthread_mutex_t cond_mutex[THREAD_NUM];

// Threads and thread arguments.
extern pthread_t threads[THREAD_NUM];
extern thread_arg_t thread_args[THREAD_NUM];

// It shows whether a thread is in sleep of not.
extern bool thread_is_sleep[THREAD_NUM];

// If it is true, threads will be terminated.
extern bool finished;

// This function searches words at substrings of the query
// and stores answers to local_answer vector.
extern void* search_substring(void* arg);

#endif
