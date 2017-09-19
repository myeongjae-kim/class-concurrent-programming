#include <Signal.h>
#include <iostream>
#include <unordered_set>
#include <algorithm>

#include <main.h>

Signal::Signal(const int n) {
  setNumberOfStrings(n);

  try {
    bf = new BloomFilter(n, 0.01);
  }catch(std::bad_alloc& e) {
    std::cerr << "(BloomFilter creation) " << e.what() << '\n'; 
    exit(-1);
  }
}

Signal::~Signal() {
  delete bf;
}

void Signal::setNumberOfStrings(const int n) {
  numberOfStrings = n;
}

int Signal::getNumberOfStrings() {
  return numberOfStrings;
}

//TODO: FIXIT. No result...
std::string Signal::query(const std::string queryStr) {
  std::vector<Signal::AnswerWord> globalResult;

  // TODO: below should be parallelized.
  for (auto &i : lengthsOfStrings) {
    if (i.second != 0) { // i.second is a count.

#ifdef DBG
      std::cout << "(query) finding substring length : " << i.first << std::endl;
#endif

      std::vector<Signal::AnswerWord>&& localResult = findExistSubstring(queryStr, i.first); // i.first is a substring length.

      for (auto word : localResult) {
        globalResult.push_back(word);
      }
    }
  }

  // sort by pos
  std::sort(globalResult.begin(), globalResult.end(),
      [](Signal::AnswerWord word1, Signal::AnswerWord word2) {
      return word1.pos < word2.pos;
      });

  // concatenate results.
  std::string concatenated;
  for (auto &i : globalResult) {
    concatenated += i.foundString + "|";
  }

  // remove last '|'
  if (concatenated.length() != 0) {
    *(--concatenated.end()) = '\0';
  }

  return concatenated;
}

void Signal::add(const std::string newStr) {
#ifdef DBG
  std::cout << "(add) " << newStr << std::endl;
#endif
  if (bf->lookup(newStr)) {
    // Do nothing. It is already exist
  } else {
    // insert
    bf->insert(newStr);

    // For finding substring in query,
    // We should remember lengths of strings in the bloom filter.
    uint32_t length = newStr.length();

    if (lengthsOfStrings.find(length) == lengthsOfStrings.end()) {
      // length is already exist
      __sync_fetch_and_add(&lengthsOfStrings[length], 1);
    } else {
      // length is not exist
      lengthsOfStrings[length] = 1;
    }

  }
}

void Signal::addParallel(const std::string newStr) {
  std::cout << "(addParallel) " << newStr << std::endl;
  std::cout << "(addParallel) Add a word to data structure parallel" << std::endl;
  std::cout << "(addParallel) Not yet implemented" << std::endl;
}


void Signal::del(const std::string toBeRemoved) {
#ifdef DBG
  std::cout << "(del) " << toBeRemoved << std::endl;
#endif

  if (bf->lookup(toBeRemoved)) {
    bf->remove(toBeRemoved);

    // For finding substring in query,
    // We should remember lengths of strings in the bloom filter.
    __sync_fetch_and_sub(&lengthsOfStrings[toBeRemoved.length()], 1);
  } else {
    // Do nothing. It is not exist
  }
}


bool Signal::isRegistered(const std::string str) {
  return bf->lookup(str) ? true : false;
}

void Signal::printLengthsOfStringsInBloomFilter() {
  for (auto i : lengthsOfStrings) {
    printf("length:%d, count:%d\n", i.first, i.second);
  }
}


//TODO: FIXIT
std::vector<Signal::AnswerWord> Signal::findExistSubstring(const std::string& query, const uint32_t subStringLength) {
  uint32_t queryLength = query.length();
  uint32_t startIdx = 0;

  std::vector<AnswerWord> result;
  result.clear();

  std::unordered_set<std::string> foundedStrings;
  AnswerWord answerBuffer;

  // TODO: How can I parallelize below process?
  while(queryLength <= startIdx + subStringLength) {
    std::string&& subStr = query.substr(startIdx, subStringLength);
    if(bf->lookup(subStr)) {
      // answer is found
      if (foundedStrings.find(subStr) == foundedStrings.end()) {
        // answer is first found
        std::cout << "(findExistSubstring) first found!" << std::endl;

        foundedStrings.insert(subStr);

        // insert to the result
        answerBuffer.foundString = subStr;
        answerBuffer.pos = startIdx;
        result.push_back(answerBuffer);
      } else {
        // answer was already found
        // do nothing
      }
    }

    subStr.clear();
    startIdx++;
  }

  // The result has no duplicated value and it has been sorted by position.

  return result;
}
