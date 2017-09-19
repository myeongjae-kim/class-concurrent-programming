#ifndef SIGNAL_H
#define SIGNAL_H

#include <cstdint>
#include <string>
#include <BloomFilter.h>
#include <unordered_map>

#include <vector>

class Signal
{
private:
  uint32_t numberOfStrings;
  BloomFilter *bf;

                    // length, count
  std::unordered_map<uint32_t, uint32_t> lengthsOfStrings;

  typedef struct {
    std::string foundString;
    uint32_t pos;
  } AnswerWord;

  // This method returns a vector of AnswerWord.
  // The vector should not have duplicated value.
  std::vector<AnswerWord> findExistSubstring(const std::string&, const uint32_t);

public:
  Signal(int numberOfStrings);
  virtual ~Signal();

  void setNumberOfStrings(const int numberOfStrings);
  int getNumberOfStrings();

  std::string query(const std::string queryStr);
  void add(const std::string newStr);
  void addParallel(const std::string newStr);
  void del(const std::string toBeRemoved);

  bool isRegistered(const std::string str);

  void printLengthsOfStringsInBloomFilter();
};

#endif /* SIGNAL_H */
