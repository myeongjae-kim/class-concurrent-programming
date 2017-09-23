#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <vector>
#include <boost/algorithm/searching/boyer_moore.hpp>

class Signal
{
private:

public:
  Signal(int numberOfStrings);
  virtual ~Signal();

  std::unordered_set<std::string> inputWords;

  typedef struct {
    std::string foundString;
    uint64_t *pos;
  } Answer;
  std::vector<Answer> globalResult;

  std::string query(const std::string queryStr);
  void add(const std::string &newWord);
  void addParallel(const std::string newStr);
  void del(const std::string &toBeRemoved);

  bool isRegistered(const std::string &str);
};
