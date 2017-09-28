//base source code: http://www.techiedelight.com/trie-implementation-insert-search-delete/

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

// Function that returns a new Trie node
struct Trie* getNewTrieNode()
{
	struct Trie* node = (struct Trie*)malloc(sizeof(struct Trie));
	node->isLeaf = 0;

	for (int i = 0; i < CHAR_SIZE; i++)
		node->character[i] = NULL;

	return node;
}

// Iterative function to insert a string in Trie.
void insert(struct Trie* *head, char* str)
{
	// start from root node
	struct Trie* curr = *head;
	while (*str)
	{
		// create a new node if path doesn't exists
		if (curr->character[*str - 'a'] == NULL)
			curr->character[*str - 'a'] = getNewTrieNode();

		// go to next node
		curr = curr->character[*str - 'a'];

		// move to next character
		str++;
	}

	// mark current node as leaf
	// curr->isLeaf = 1;
	curr->isLeaf = patternID++;
}

// Iterative function to search a string in Trie. It returns 1
// if the string is found in the Trie, else it returns 0
int search(struct Trie* head, char* str)
{
	// return 0 if Trie is empty
	if (head == NULL)
		return 0;

	struct Trie* curr = head;
	while (*str)
	{
		// go to next node
		curr = curr->character[*str - 'a'];

		// if string is invalid (reached end of path in Trie)
		if (curr == NULL)
			return 0;

		// move to next character
		str++;
	}

	// if current node is a leaf and we have reached the
	// end of the string, return 1
	return curr->isLeaf;
}

// Iterative function to search a string in Trie. It returns 1
// if the string is found in the Trie, else it returns 0
int searchAllPatterns(struct Trie* head, char* strQuery)
{
	// return 0 if Trie is empty
	if (head == NULL)
		return 0;

	struct Trie* curr = head;

  Answer answerBuffer;
  printed.clear();
  answers.clear();

  char* str;
  while (*strQuery) {
    str = strQuery;

    while (*str)
    {
      // go to next node
      curr = curr->character[*str - 'a'];

      // if string is invalid (reached end of path in Trie)
      if (curr == NULL) {
        curr = head;
        break;
      } else if (curr -> isLeaf) {
        answerBuffer.startAdr = strQuery;
        answerBuffer.length = (uint32_t)(str - strQuery) + 1;
        answerBuffer.patternID = curr->isLeaf;

        // TODO: parallelize by using index.
        answers.push_back(answerBuffer);
      }

      // move to next character
      str++;
    }

    strQuery++;
  }

  // TODO:print
  // sort answer vector
  // check whether the answer was printed
  // if answer is not printed, print answer and add to printed set
  // clear printed set and answer vector


  char* currentChar;
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
    currentChar = tempAnswer.startAdr;
    for (uint32_t i = 0; i < tempAnswer.length; ++i) {
      putchar(*currentChar);
      currentChar++;
    }

    //print the others
    for (uint32_t i = 1; i < size; ++i) {
      Answer& answer = answers[i];
      if (printed.find(answer.patternID) == printed.end()) {
        printed.insert(answer.patternID);
        currentChar = answer.startAdr;
        putchar('|');
        for (uint32_t j = 0; j < answer.length; ++j) {
          putchar(*currentChar);
          currentChar++;
        }
      }
    }

  } else {
    // no answer
    printf("-1");
  }
  putchar('\n');


  // if current node is a leaf and we have reached the
  // end of the string, return 1
  return curr->isLeaf;
}


// returns 1 if given node has any children
int haveChildren(struct Trie* curr)
{
  for (int i = 0; i < CHAR_SIZE; i++)
    if (curr->character[i])
      return 1;	// child found

  return 0;
}

// Recursive function to delete a string in Trie.
int deletion(struct Trie* *curr, char* str)
{
  // return if Trie is empty
  if (*curr == NULL)
    return 0;

  // if we have not reached the end of the string
  if (*str)
  {
    // recurse for the node corresponding to next character in
    // the string and if it returns 1, delete current node
    // (if it is non-leaf)
    if (*curr != NULL && (*curr)->character[*str - 'a'] != NULL &&
        deletion(&((*curr)->character[*str - 'a']), str + 1) &&
        (*curr)->isLeaf == 0)
    {
      if (!haveChildren(*curr))
      {
        free(*curr);
        (*curr) = NULL;
        return 1;
      }
      else {
        return 0;
      }
    }
  }

  // if we have reached the end of the string
  if (*str == '\0' && (*curr)->isLeaf)
  {
    // if current node is a leaf node and don't have any children
    if (!haveChildren(*curr))
    {
      free(*curr); // delete current node
      (*curr) = NULL;
      return 1; // delete non-leaf parent nodes
    }

    // if current node is a leaf node and have children
    else
    {
      // mark current node as non-leaf node (DON'T DELETE IT)
      (*curr)->isLeaf = 0;
      return 0;	   // don't delete its parent nodes
    }
  }

  return 0;
}

/* void setWasPrintedFalse(struct Trie* head) {
 *   // do it recursively
 *
 *   // base case
 *   if (head == NULL) {
 *     return;
 *   }
 *
 *  if (head -> isLeaf) {
 *     head->wasPrinted = 0;
 *   }
 *
 *   for (uint8_t i = 0; i < CHAR_SIZE; ++i) {
 *     struct Trie* child = head->character[i];
 *     if (child != NULL) {
 *       setWasPrintedFalse(child);
 *     }
 *   }
 * } */

// Trie Implementation in C - Insertion, Searching and Deletion
int TestTrie()
{
  struct Trie* head = getNewTrieNode();

  /*   insert(&head, "hello");
   *   printf("%d ", search(head, "hello"));   	// print 1
   *
   *   insert(&head, "helloworld");
   *   printf("%d ", search(head, "helloworld"));  // print 1
   *
   *   printf("%d ", search(head, "helll"));   	// print 0 (Not present)
   *
   *   insert(&head, "hell");
   *   printf("%d ", search(head, "hell"));		// print 1
   *
   *   insert(&head, "h");
   *   printf("%d \n", search(head, "h")); 		// print 1 + newline
   *
   *   deletion(&head, "hello");
   *   printf("%d ", search(head, "hello"));   	// print 0 (hello deleted)
   *   printf("%d ", search(head, "helloworld"));  // print 1
   *   printf("%d \n", search(head, "hell"));  	// print 1 + newline
   *
   *   deletion(&head, "h");
   *   printf("%d ", search(head, "h"));   		// print 0 (h deleted)
   *   printf("%d ", search(head, "hell"));		// print 1
   *   printf("%d\n", search(head, "helloworld")); // print 1 + newline
   *
   *   deletion(&head, "helloworld");
   *   printf("%d ", search(head, "helloworld"));  // print 0
   *   printf("%d ", search(head, "hell"));		// print 1
   *
   *   deletion(&head, "hell");
   *   printf("%d\n", search(head, "hell"));   	// print 0 + newline
   *
   *   if (head == NULL)
   *     printf("Trie empty!!\n");   			// Trie is empty now
   *
   *   printf("%d ", search(head, "hell"));		// print 0
   *  */

  insert(&head, (char*)"app");
  insert(&head, (char*)"apple");
  insert(&head, (char*)"pineapple");
  insert(&head, (char*)"leap");

  searchAllPatterns(head, (char*)"apple");
  // setWasPrintedFalse(head);
  searchAllPatterns(head, (char*)"pineapple");
  // setWasPrintedFalse(head);
  // searchAllPatterns(head, "penpineappleapplepen");


  return 0;
}
