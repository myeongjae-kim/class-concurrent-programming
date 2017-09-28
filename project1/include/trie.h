#define ALPHA_NUM 26
#define THREAD_NUM 36

#include <cstdint>
#include <pthread.h>

struct Trie
{
  /* It is zero when character is not the end of a word. */
	int wordID;	

	struct Trie* chars[ALPHA_NUM];
};

typedef struct _Answer {
  char* startAdr;
  uint32_t length;
  uint32_t patternID;
} Answer;

struct Trie* createTrieNode();

void insert(struct Trie* *trieHead, char* str);
int erase(struct Trie* *trieNode, char* str);

int search(struct Trie* trieHead, char* str);
int searchAllPatterns(struct Trie* trieHead, char* strQuery);

int TestTrie();
