#define ALPHA_NUM 26

#include <cstdint>
#include <pthread.h>

// for threads
#define THREAD_NUM 42

typedef struct _thread_arg {
  struct trie* trie_root; // this value could be a global variable.
  const char* substring_location;
  uint32_t search_length;
} thread_arg_t;



struct trie
{
  /* It is zero when character is not the end of a word. */
	int word_id;	

	struct trie* chars[ALPHA_NUM];
};

typedef struct _answer {
  const char* substring_location;
  uint32_t length;
  uint32_t pattern_id;
} answer_t;

struct trie* create_trie_node();

void insert(struct trie* *trie_head, char* str);
bool erase(struct trie* *trie_node, const char* str);

// int search(struct Trie* trieHead, char* str);
void search_all_patterns(struct trie* trie_root, const char* str_query, const uint32_t str_length);

// int TestTrie();
void erase_all(struct trie* *trie_node);
