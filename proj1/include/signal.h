#ifndef SIGNAL_H
#define SIGNAL_H

#include <cstdint>
#include <string>

class Signal
{
private:
  uint32_t numberOfStrings;
  // Bloom Filter should be included.

public:
  Signal(int numberOfStrings);
  virtual ~Signal();

  void setNumberOfStrings(const int numberOfStrings);
  int getNumberOfStrings();

  std::string query(const std::string queryStr);
  void add(const std::string newStr);
  void addParallel(const std::string newStr);
  void del(const std::string toBeRemoved);
};

#endif /* SIGNAL_H */
