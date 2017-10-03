/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : trie.cc
 * Due date             : 2017-10-01
 * Compilation Standard : c++11 */

#include <cstdlib>
#include <cstdint>
#include <cstring>

#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_set>

#include "trie.h"

// Every words inserted to the trie structure has its own unique word ID.
// This variable is the uinque word ID generator.
uint32_t word_id_generator = 1;

// This function creates a trie node.
struct trie* create_trie_node() {
  // create new node
  struct trie* node = (struct trie*)calloc(1, sizeof(*node));
  return node;
}

// This function inserts a word to the trie structure iteratively.
void insert(struct trie* *trie_root, const char* word_exploring_ptr)
{
  struct trie* trie_node = *trie_root;
  while (*word_exploring_ptr)
  {
    // check wheter the trie already has a character of current word.
    if (trie_node->chars[*word_exploring_ptr - 'a'] == NULL){
      //if not, create
      trie_node->chars[*word_exploring_ptr - 'a'] = create_trie_node();
    }

    //go to next character node.
    trie_node = trie_node->chars[*word_exploring_ptr - 'a'];
    word_exploring_ptr++;
  }

  // Every word has its unique ID. It is used not to print duplicated result.
  trie_node->word_id = word_id_generator++;
}

// This function returns true if it has at least one child node.
bool child_exist(const struct trie* const trie_node) {
  //Check whether it has at least one child node.
  for (int i = 0; i < ALPHA_NUM; i++){
    if (trie_node->chars[i]){
      return true;
    }
  }

  return false;
}

// This function erases a word in the trie structure.
// It returns true if erasing is successful.
bool erase(struct trie* *trie_node, const char* word_exploring_ptr) {
  // 'Target string' is a word that will be removed.

  // base case #1: Null node.
  if (*trie_node == NULL){
    return false;
  }

  // base case #2: On the end of the target string
  if (*word_exploring_ptr == '\0' && (*trie_node)->word_id) {
    // If has no children
    if (!child_exist(*trie_node)) {
      // erasing is success.

      // remove and nullify.
      free(*trie_node);
      (*trie_node) = NULL;
      return true;
    } else {
      // when it is not a leaf node
      // do not remove. just makes wordID zero.
      (*trie_node)->word_id = 0;
      return false;
    }
  }

  // Below is recurion process.

  // Recursively find target node.
  // check whether it has a node of next character of the target string or not
  struct trie* *next_node = (*trie_node)->chars + *word_exploring_ptr - 'a';

  // if it has a next node,
  if (next_node) {

    // erase next character of target string recursively.
    bool erasing_is_successful = erase(next_node, word_exploring_ptr + 1);

    // If erase is success and we are on the middle of target string,
    // (the meaning of 'wordID == 0' is that we are not on the end of the target string)
    if (erasing_is_successful && (*trie_node)->word_id == 0) {

      // erase node if it has no children node.
      if (child_exist(*trie_node) == false) {
        free(*trie_node);
        (*trie_node) = NULL;
        return true;
      } else {
        return false;
      }
    }
  }

  return false;
}


// This function recursively free all allocated memories.
void erase_all(struct trie* *trie_node) {
  // If the node is exist
  if (*trie_node) {

    // find child node and free it
    for (int i = 0; i < ALPHA_NUM; ++i) {
      if ((*trie_node)->chars[i] != nullptr) {
        erase_all((*trie_node)->chars + i);
      }
    }

    // Current node does not have children.
    // Free current node.
    free(*trie_node);
    *trie_node = nullptr;
  }
}



// Variables for parallelizing.
// for conditional sleep and wake up threads.
pthread_cond_t cond[THREAD_NUM];
pthread_mutex_t cond_mutex[THREAD_NUM];

// Threads and thread arguments.
pthread_t threads[THREAD_NUM];
thread_arg_t thread_args[THREAD_NUM];

// It shows whether a thread is in sleep of not.
bool thread_is_sleep[THREAD_NUM];

// If it is true, threads will be terminated.
bool finished;


// This vector collects answers from localAnswers.
std::vector < answer_t > global_answers;

// Every thread has its own answer vector for avoid locking.
std::vector < answer_t > local_answers[THREAD_NUM];

// It stores already printed ID of words (wordID).
std::unordered_set <uint32_t> printed;


// This function searches words at substrings of the query
// and stores answers to local_answer vector.
// This function is an argument of pthread_create().
void* search_substring(void* arg) {
  const long tid = (const long)arg;
  answer_t answer_buffer;
  local_answers[tid].clear();

  // Go to sleep right after it is made.
  pthread_mutex_lock(&cond_mutex[tid]);
  thread_is_sleep[tid] = true;
  pthread_cond_wait(&cond[tid], &cond_mutex[tid]);
  pthread_mutex_unlock(&cond_mutex[tid]);

  // If finished is 'true', threads will be terminated.
  while (!finished) {
    // waken up!

    // Get data from thread arguments.
    // Use local variable to avoid point referencing overhead of arguments.
    thread_arg_t data = thread_args[tid];

    // This variable explores the trie structure
    const struct trie* trie_node = data.trie_root;

    // This variable explores the query.
    const char* query_exploring_ptr;
    for (
        // initialize counter
        uint32_t search_count = 0; 

        // condition:
        // Search counter is smaller than search length,
        // and substring's first character is not on the end of the query.
        search_count < data.search_length && *data.substring_location;

        // increasing variable for every iteration.
        search_count++, data.substring_location++){

      /* Here is in the for loop */
      // Set the ptr to the start of substring of the query
      query_exploring_ptr = data.substring_location;

      // to the end of the query
      while (*query_exploring_ptr) {
        trie_node = trie_node->chars[*query_exploring_ptr - 'a'];

        if (trie_node == NULL) {
          // when the substring is not exist in the trie,
          // finish current search and go to next character of the query and search again.
          trie_node = data.trie_root;
          break;
        } else if (trie_node -> word_id) {
          // When a word is exist,
          // Add information to answerBuffer
          answer_buffer.substring_location = data.substring_location;
          answer_buffer.length = (uint32_t)(query_exploring_ptr - data.substring_location) + 1;
          answer_buffer.word_id = trie_node->word_id;

          // and store it in the localAnswer vector.
          local_answers[tid].push_back(answer_buffer);
        }

        // go to next character of substring.
        query_exploring_ptr++;
      }

      // node reinitialization. Go to root.
      trie_node = data.trie_root;
    }

    // search is finished.
    // go to sleep
    pthread_mutex_lock(&cond_mutex[tid]);
    thread_is_sleep[tid] = true;
    pthread_cond_wait(&cond[tid], &cond_mutex[tid]);
    pthread_mutex_unlock(&cond_mutex[tid]);
    // Wake up!
  }

  // Thread is terminated.
  return nullptr;
}

// This function prints all of found words.
void search_all_patterns(struct trie* trie_root, 
                          const char* query, 
                          const uint32_t query_length) {
  // return 0 if Trie is empty
  if (trie_root == NULL){
    return;
  }

  // initialize data structures.
  printed.clear();
  global_answers.clear();


  const char* end_of_query = query + query_length;
  const char* substring_location = query;

  // the number of iteration of each thread.
  const uint32_t search_itertion_number = query_length / THREAD_NUM + 1;

  // Do not wake up threads that does not have its own cake.
  // This happens when a string size is smaller that the number of threads.

  // prepare arguments for theads

  uint32_t tid = 0;
  for (tid = 0; tid < THREAD_NUM && substring_location < end_of_query; ++tid) {
    // If substringLocation is same or bigger than end_of_query,
    //  there is no substring for threads.
    // This happens when a size of query is smaller than the number of threads.
    
    thread_args[tid].substring_location = substring_location;
    thread_args[tid].trie_root = trie_root;
    thread_args[tid].search_length = search_itertion_number;

    substring_location += search_itertion_number;
  }

  // Wake up threads
  
  // Thread reinit for wake up
  for (uint32_t i = 0; i < tid ; ++i) {
    thread_is_sleep[i] = false;
  }

  // Don't wake up threads that does not have arguments.
  for (uint32_t i = 0; i < tid; ++i) {
    pthread_mutex_lock(&cond_mutex[i]);
    pthread_cond_signal(&cond[i]);
    pthread_mutex_unlock(&cond_mutex[i]);
  }


  // Wait for all threads to finish work
  // Belows waiting codes are TA's code.

  // Iterate until all of threads are in sleep.
  while (1) {
    bool all_thread_done = true;
    for (uint32_t i = 0; i < tid; i++) {
      if (thread_is_sleep[i] == false) {
        // Not sleeping thread is found.
        all_thread_done = false;
        break;
      }
    }
    if (all_thread_done) {
      break;
    }

    // Yield cpu resources to running thread.
    pthread_yield();
  }
  // The end of TA's code.


  // Below is printing procedures.

  // Psuedo code
  // 1. Collect answers from local_answers vector and store answers to global_answer vector.
  // 2. Sort global_answer vector
  // 3. Iterate sorted global_answer and print the answers.
  // 4. Check whether the answer was printed or not.
  // 5. If answer is not printed, print answer and add its wordID to printed set


  // 1. Collect answers from local_answers vector and store answers to global_answer vector.
  for (int i = 0; i < THREAD_NUM; ++i) {
    for (auto& answer : local_answers[i]) {
      global_answers.push_back(answer);
    }
    // reinitialize local answers when all answers are moved to global answers vector.
    local_answers[i].clear();
  }

  uint32_t size = global_answers.size();
  if (size != 0) {
    // answer exists

    // 2. Sort global_answer vector
    // A lambda function is used as a callback function for sorting.
    std::sort(global_answers.begin(), global_answers.end(), [](answer_t lhs, answer_t rhs){

        // sort by substring's start location
        if (lhs.substring_location < rhs.substring_location) {
          return true;
        } else if (lhs.substring_location > rhs.substring_location) {
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


    // 3. Iterate sorted global_answer and print the answers.

    // The difference of printing first one and others is that
    // '|' is printed of not.

    // Print first one
    const answer_t& first_answer = global_answers[0];
    printed.insert(first_answer.word_id);

    const char* printing_target = first_answer.substring_location;
    for (uint32_t i = 0; i < first_answer.length; ++i) {
      std::cout << *printing_target;

      // go to next character
      printing_target++;
    }

    // Print the others
    // Iterate global_answers vector.
    for (uint32_t i = 1; i < size; ++i) {
      const answer_t& answer = global_answers[i];
      if (printed.find(answer.word_id) == printed.end()) {
        printed.insert(answer.word_id);
        printing_target = answer.substring_location;
        std::cout << '|';
        for (uint32_t j = 0; j < answer.length; ++j) {
          std::cout << *printing_target;

          // go to next character
          printing_target++;
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

