// define character size
// currently Trie supports lowercase English characters (a - z)
#define CHAR_SIZE 26

// A Trie node
struct Trie
{
	int isLeaf;	// 1 when node is a leaf node
  int wasPrinted;
	struct Trie* character[CHAR_SIZE];
};

struct Trie* getNewTrieNode();

void insert(struct Trie* *head, char* str);
int deletion(struct Trie* *curr, char* str);

int search(struct Trie* head, char* str);
int searchAllPatterns(struct Trie* head, char* strQuery);
void setWasPrintedFalse(struct Trie* head);

int TestTrie();
