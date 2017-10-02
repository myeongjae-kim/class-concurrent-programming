#define ALPHA_NUM 26

#include <cstdint>
#include <pthread.h>

// for threads
#define THREAD_NUM 42

typedef struct _ThreadArg {
  struct Trie* trieRoot; // this value could be a global variable.
  const char* substringLocation;
  uint32_t searchLength;
} ThreadArg;



struct Trie
{
  /* It is zero when character is not the end of a word. */
	int wordID;	

	struct Trie* chars[ALPHA_NUM];
};

typedef struct _Answer {
  const char* substringLocation;
  uint32_t length;
  uint32_t patternID;
} Answer;

struct Trie* createTrieNode();

void insert(struct Trie* *trieHead, char* str);
bool erase(struct Trie* *trieNode, const char* str);

// int search(struct Trie* trieHead, char* str);
void searchAllPatterns(struct Trie* trieRoot, const char* strQuery, const uint32_t strLength);

// int TestTrie();
