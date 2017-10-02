#include <iostream>

#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include "trie.h"

#include <vector>
#include <unordered_set>

// Every words inserted to the trie structure has its own unique ID.
// This variable is the uinque ID generator.
uint32_t patternID = 1;

// This function creates a trie node.
struct Trie* createTrieNode() {
  // create new node
	struct Trie* node = (struct Trie*)calloc(1, sizeof(*node));
	return node;
}

// This function inserts a word to the trie structure iteratively.
void insert(struct Trie* *trieRoot, char* str)
{
	struct Trie* trieNode = *trieRoot;
	while (*str)
	{
    // check wheter the trie already has a character of current word.
		if (trieNode->chars[*str - 'a'] == NULL){
      //if not, create
			trieNode->chars[*str - 'a'] = createTrieNode();
    }

    //go to next character node.
		trieNode = trieNode->chars[*str - 'a'];
		str++;
	}

  // Every word has its unique ID. It is used not to print duplicated result.
	trieNode->wordID = patternID++;
}

// Variables for parallelizing.

// for conditional sleep and wake up threads.
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


// This is a function that a thread executes.
void* searchSubstring(void* arg) {
  const long tid = (const long)arg;
  Answer answerBuffer;
  localAnswers[tid].clear();

  // Go to sleep right after it is made.
  pthread_mutex_lock(&condMutex[tid]);
  threadIsSleep[tid] = true;
  pthread_cond_wait(&cond[tid], &condMutex[tid]);
  pthread_mutex_unlock(&condMutex[tid]);

  // If finished is 'true', threads will be terminated.
  while (!finished) {
    // waken up!

    // Get data from thread arguments.
    // Use local variable to avoid point referencing overhead of arguments.
    ThreadArg data = threadArgs[tid];

    // This variable explores the trie structure
    const struct Trie* trieNode = data.trieRoot;

    // This variable explores the query.
    const char* exploringQueryPointer;
    for (
        // initialize counter
        uint32_t searchCount = 0; 

        // condition:
        // Search counter is smaller than search length,
        // and substring's first character is not on the end of the query.
        searchCount < data.searchLength && *data.substringLocation;

        // increasing variable for every iteration.
        searchCount++, data.substringLocation++){

      /* Here is in the for loop */
      // Set the pointer to the start of substring of the query
      exploringQueryPointer = data.substringLocation;

      // to the end of the query
      while (*exploringQueryPointer) {
        trieNode = trieNode->chars[*exploringQueryPointer - 'a'];

        if (trieNode == NULL) {
          // when the substring is not exist in the trie,
          // finish current search and go to next character of the query and search again.
          trieNode = data.trieRoot;
          break;
        } else if (trieNode -> wordID) {
          // When a word is exist,
          // Add information to answerBuffer
          answerBuffer.substringLocation = data.substringLocation;
          answerBuffer.length = (uint32_t)(exploringQueryPointer - data.substringLocation) + 1;
          answerBuffer.patternID = trieNode->wordID;

          // and store it in the localAnswer vector.
          localAnswers[tid].push_back(answerBuffer);
        }

        // go to next character of substring.
        exploringQueryPointer++;
      }

      // node reinitialization. Go to root.
      trieNode = data.trieRoot;
    }

    // search is finished.
    // go to sleep
    pthread_mutex_lock(&condMutex[tid]);
    threadIsSleep[tid] = true;
    pthread_cond_wait(&cond[tid], &condMutex[tid]);
    pthread_mutex_unlock(&condMutex[tid]);
    // Wake up!
  }

  // Thread is terminated.
  return nullptr;
}

void searchAllPatterns(struct Trie* trieRoot, const char* strQuery, const uint32_t strLength)
{
  // return 0 if Trie is empty
  if (trieRoot == NULL){
    return;
  }

  // initialize data structures.
  printed.clear();
  answers.clear();


  const char* end_of_query = strQuery + strLength;
  const char* substringLocation = strQuery;

  // the number of iteration of each thread.
  const uint32_t search_itertion_number = strLength / THREAD_NUM + 1;

  // Do not wake up threads that does not have its own cake.
  // This happens when a string size is smaller that the number of threads.

  // prepare arguments for theads

  uint32_t tid = 0;
  for (uint32_t i = 0; i < THREAD_NUM && substringLocation < end_of_query; ++i) {
    // If substringLocation is same or bigger than end_of_query,
    // there is no substring for threads that its 'tid' is bigger thant current 'tid'.
    
    tid = i % THREAD_NUM;
    threadArgs[tid].substringLocation = substringLocation;
    threadArgs[tid].trieRoot = trieRoot;
    threadArgs[tid].searchLength = search_itertion_number;

    substringLocation += search_itertion_number;
  }

  // Run Threads

  // Wake up threads
  // Thread reinit for start
  for (uint32_t i = 0; i <= tid ; ++i) {
    threadIsSleep[i] = false;
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
    // reinitialize local answers when all answers are moved to global answers vector.
    localAnswers[i].clear();
  }

  const char* printingTarget;
  uint32_t size = answers.size();
  if (size != 0) {
    // answer exists
    // sort!

    // A lambda function is used as a callback function for sorting.
    std::sort(answers.begin(), answers.end(), [](Answer lhs, Answer rhs){
        // sort by substring's start location
        if (lhs.substringLocation < rhs.substringLocation) {
        return true;
        } else if (lhs.substringLocation > rhs.substringLocation) {
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


    // The difference of printing first one and others is that
    // '|' is printed of not.

    // Print first one
    const Answer& firstAnswer = answers[0];
    printed.insert(firstAnswer.patternID);
    printingTarget = firstAnswer.substringLocation;
    for (uint32_t i = 0; i < firstAnswer.length; ++i) {
      std::cout << *printingTarget;

      // go to next character
      printingTarget++;
    }

    // Print the others
    for (uint32_t i = 1; i < size; ++i) {
      const Answer& answer = answers[i];
      if (printed.find(answer.patternID) == printed.end()) {
        printed.insert(answer.patternID);
        printingTarget = answer.substringLocation;
        std::cout << '|';
        for (uint32_t j = 0; j < answer.length; ++j) {
          std::cout << *printingTarget;

          // go to next character
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


// This function returns true if it has at least one child node.
bool childExist(const struct Trie* const trieNode) {
  //Check whether it has at least one child node.
  for (int i = 0; i < ALPHA_NUM; i++){
    if (trieNode->chars[i]){
      return true;
    }
  }

  return false;
}

// This function erases a word in the trie structure. It returns true if erasing is successful.
bool erase(struct Trie* *trieNode, const char* str) {
  // 'Target string' is a word that will be removed.

  // base case #1: Null node.
  if (*trieNode == NULL){
    return false;
  }

  // base case #2: On the end of the target string
  if (*str == '\0' && (*trieNode)->wordID) {
    // If has no children
    if (!childExist(*trieNode)) {
      // erasing is success.

      // remove and nullify.
      free(*trieNode);
      (*trieNode) = NULL;
      return true;
    } else {
      // when it is not a leaf node
      // do not remove. just makes wordID zero.
      (*trieNode)->wordID = 0;
      return false;
    }
  }

  // Below is recurion process.

  // Recursively find target node.
  // check whether it has a node of next character of the target string or not
  struct Trie* *nextNode = (*trieNode)->chars + *str - 'a';

  // if it has a next node,
  if (nextNode) {

    // erase next character of target string recursively.
    bool erasingIsSuccess = erase(nextNode, str + 1);

    // If erase is success and we are on the middle of target string,
    // (the meaning of 'wordID == 0' is that we are not on the end of the target string)
    if ( erasingIsSuccess && (*trieNode)->wordID == 0) {

      // erase node if it has no children node.
      if (childExist(*trieNode) == false) {
        free(*trieNode);
        (*trieNode) = NULL;
        return true;
      } else {
        return false;
      }
    }
  }

  return false;
}
