#ifndef SIGNAL_H
#define SIGNAL_H

#include <cstdint>
#include <string>
#include <BloomFilter.h>
#include <unordered_map>

class Signal
{
private:
  uint32_t numberOfStrings;
  BloomFilter *bf;

  std::unordered_map<int, int> lengthsOfStrings;

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
