#include <Signal.h>
#include <iostream>
#include <unordered_set>
#include <algorithm>

#include <main.h>

#include <boost/algorithm/searching/boyer_moore.hpp>

std::vector<Answer>* globalResultPointer = nullptr;
const std::string* queryPointer = nullptr;

Signal::Signal(const int n) {
  inputWords.reserve(n);
  globalResult.reserve(1024);

  globalResultPointer = &globalResult;
}

Signal::~Signal() {
}

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* searchSubstring(void* wordPointerArg) {
  Answer answerBuffer;
  std::string* wordPointer = (std::string*)wordPointerArg;

  auto searchResult = boost::algorithm::boyer_moore_search(queryPointer->begin(), queryPointer->end(),
      wordPointer->begin(), wordPointer->end());

  if (queryPointer->end() != searchResult)
  {
    // found
    answerBuffer.pos = (uint64_t*)&(*searchResult);
    answerBuffer.foundString = *wordPointer;

    //locking
    pthread_mutex_lock(&mutex);
    //TODO:use concurrent safe data structure to speed up
    globalResultPointer->push_back(answerBuffer);
    pthread_mutex_unlock(&mutex);
  } else {
    // Not Found
    // Do Nothing
  }
  return nullptr;
}

std::string Signal::query(const std::string queryStr) {
  Answer answerBuffer;
  queryPointer = &queryStr;
  
  pthread_t *threads = (pthread_t*)malloc(inputWords.size() * sizeof(pthread_t));
  int count = 0;
  for (auto &word : inputWords) {
    pthread_create(&threads[count++], NULL, searchSubstring, (void *)&word);
  }

  // count is the number of threads
  for (int i = 0; i < count; ++i) {
    pthread_join(threads[i], NULL);
  }

  free(threads);
  threads = nullptr;

  std::sort(globalResult.begin(), globalResult.end(),
      [](Answer a1, Answer a2) {
      if (a1.pos < a2.pos) {
      return true;
      } else if (a1.pos > a2.pos) {
      return false;
      } else {
      // when position is same,
      // short string is front
      if (a1.foundString.length() < a2.foundString.length()) {
      return true;
      //TODO: 디버깅 끝나면 이부분 비교하지 말기
      } else if(a1.foundString.length() > a2.foundString.length()){
      return false;
      } else {
      // There is no case of same length.
      ERROR_MSG("(query) There is no case of same length.");
      exit(EXIT_FAILURE);
      }
      }
      });

  std::string rtValue("");
  for (auto &i : globalResult) {
    if (i.foundString.length() != 0) {
      rtValue += i.foundString + "|";
    }
  }

  globalResult.clear();

  if (rtValue.length() == 0) {
    return "-1";
  } else {
    rtValue.pop_back();
    return rtValue;
  }
}

void Signal::add(const std::string &newWord) {
#ifdef DBG
  std::cout << "(add) " << newWord << std::endl;
#endif
  inputWords.insert(newWord);
}

void Signal::addParallel(const std::string newStr) {
  std::cout << "(addParallel) " << newStr << std::endl;
  std::cout << "(addParallel) Add a word to data structure parallel" << std::endl;
  std::cout << "(addParallel) Not yet implemented" << std::endl;
}


void Signal::del(const std::string &toBeRemoved) {
#ifdef DBG
  std::cout << "(del) " << toBeRemoved << std::endl;
#endif
  inputWords.erase(toBeRemoved);
}


bool Signal::isRegistered(const std::string &word) {
  return inputWords.find(word) != inputWords.end();
}
