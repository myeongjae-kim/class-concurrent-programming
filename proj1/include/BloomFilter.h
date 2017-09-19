#include <string>
#include <cstdint>

typedef uint64_t (*hashfunc_t)(const std::string str);

class BloomFilter
{
public:
  BloomFilter (const uint64_t numberOfStrings, const double errProb);

  void insert(const std::string str);
  bool lookup(const std::string str);
  void remove(const std::string str);

  virtual ~BloomFilter ();

private:
  uint64_t getIndex(const int hashFuncReturn);
  void reHashing();


  // It is not a bit for implementing delete.
  uint8_t *filterArray;

  /* s : number of strings,
   * b : bits per an object,
   * k : number of hash function */
  uint64_t maxNumberOfElements;   // s
  uint64_t filterSize;            // b * n of elements
  uint64_t numberOfHashFunctions; // k

  hashfunc_t *hashFuncs;

};

uint64_t Hash1 (const std::string str);
uint64_t Hash2 (const std::string str);
uint64_t Hash3 (const std::string str);
uint64_t Hash4 (const std::string str);
uint64_t Hash5 (const std::string str);
uint64_t Hash6 (const std::string str);
uint64_t Hash7 (const std::string str);
uint64_t Hash8 (const std::string str);
uint64_t Hash9 (const std::string str);
uint64_t Hash10 (const std::string str);

