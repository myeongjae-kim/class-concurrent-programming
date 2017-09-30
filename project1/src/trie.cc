//base source code: http://www.techiedelight.com/trie-implementation-insert-search-delete/

#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include "trie.h"

#include <unordered_set>


uint32_t patternID = 1;

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


// Iterative function to search a string in Trie. It returns 1
// if the string is found in the Trie, else it returns 0
int searchAllPatterns(struct Trie* trieRoot, char* strQuery)
{
	// return 0 if Trie is empty
	if (trieRoot == NULL)
		return 0;

	struct Trie* curr = trieRoot;

  bool firstPrint = true;
  bool hasAnswer = false;
  printed.clear();

  char* str;
  while (*strQuery) {
    str = strQuery;

    while (*str)
    {
      // go to next trieNode
      curr = curr->chars[*str - 'a'];

      // if string is invalid (reached end of path in Trie)
      if (curr == NULL) {
        curr = trieRoot;
        break;
      } else if (curr -> wordID) {
        if (printed.find(curr->wordID) != printed.end()) {
          // do not print if it was printed.
        } else {
          printed.insert(curr->wordID);
          // print start
          if (!firstPrint) {
            putchar('|');
          }

          char* print = strQuery;
          while(print <= str) {
            putchar(*print);
            print++;
          }
          firstPrint = false;
          hasAnswer = true;
        }
      }

      // move to next chars
      str++;
    }

    curr = trieRoot;
    strQuery++;
  }


  if (hasAnswer == false) {
    printf("-1");
  }

  putchar('\n');
  // if current trieNode is a leaf and we have reached the
  // end of the string, return 1
  return curr->wordID;
}


// returns 1 if given trieNode has any children
int haveChildren(struct Trie* curr)
{
  for (int i = 0; i < ALPHA_NUM; i++)
    if (curr->chars[i])
      return 1;	// child found

  return 0;
}

// Recursive function to delete a string in Trie.
int erase(struct Trie* *curr, char* str)
{
  // return if Trie is empty
  if (*curr == NULL)
    return 0;

  // if we have not reached the end of the string
  if (*str)
  {
    // recurse for the trieNode corresponding to next chars in
    // the string and if it returns 1, delete current trieNode
    // (if it is non-leaf)
    if (*curr != NULL && (*curr)->chars[*str - 'a'] != NULL &&
        erase(&((*curr)->chars[*str - 'a']), str + 1) &&
        (*curr)->wordID == 0)
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
  if (*str == '\0' && (*curr)->wordID)
  {
    // if current trieNode is a leaf trieNode and don't have any children
    if (!haveChildren(*curr))
    {
      free(*curr); // delete current trieNode
      (*curr) = NULL;
      return 1; // delete non-leaf parent trieNodes
    }

    // if current trieNode is a leaf trieNode and have children
    else
    {
      // mark current trieNode as non-leaf trieNode (DON'T DELETE IT)
      (*curr)->wordID = 0;
      return 0;	   // don't delete its parent trieNodes
    }
  }

  return 0;
}

/* void setWasPrintedFalse(struct Trie* trieRoot) {
 *   // do it recursively
 *
 *   // base case
 *   if (trieRoot == NULL) {
 *     return;
 *   }
 *
 *  if (trieRoot -> wordID) {
 *     trieRoot->wasPrinted = 0;
 *   }
 *
 *   for (uint8_t i = 0; i < ALPHA_NUM; ++i) {
 *     struct Trie* child = trieRoot->chars[i];
 *     if (child != NULL) {
 *       setWasPrintedFalse(child);
 *     }
 *   }
 * } */

// Trie Implementation in C - Insertion, Searching and erase
int TestTrie()
{
  struct Trie* trieRoot = createTrieNode();

  /*   insert(&trieRoot, "hello");
   *   printf("%d ", search(trieRoot, "hello"));   	// print 1
   *
   *   insert(&trieRoot, "helloworld");
   *   printf("%d ", search(trieRoot, "helloworld"));  // print 1
   *
   *   printf("%d ", search(trieRoot, "helll"));   	// print 0 (Not present)
   *
   *   insert(&trieRoot, "hell");
   *   printf("%d ", search(trieRoot, "hell"));		// print 1
   *
   *   insert(&trieRoot, "h");
   *   printf("%d \n", search(trieRoot, "h")); 		// print 1 + newline
   *
   *   erase(&trieRoot, "hello");
   *   printf("%d ", search(trieRoot, "hello"));   	// print 0 (hello deleted)
   *   printf("%d ", search(trieRoot, "helloworld"));  // print 1
   *   printf("%d \n", search(trieRoot, "hell"));  	// print 1 + newline
   *
   *   erase(&trieRoot, "h");
   *   printf("%d ", search(trieRoot, "h"));   		// print 0 (h deleted)
   *   printf("%d ", search(trieRoot, "hell"));		// print 1
   *   printf("%d\n", search(trieRoot, "helloworld")); // print 1 + newline
   *
   *   erase(&trieRoot, "helloworld");
   *   printf("%d ", search(trieRoot, "helloworld"));  // print 0
   *   printf("%d ", search(trieRoot, "hell"));		// print 1
   *
   *   erase(&trieRoot, "hell");
   *   printf("%d\n", search(trieRoot, "hell"));   	// print 0 + newline
   *
   *   if (trieRoot == NULL)
   *     printf("Trie empty!!\n");   			// Trie is empty now
   *
   *   printf("%d ", search(trieRoot, "hell"));		// print 0
   *  */

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
