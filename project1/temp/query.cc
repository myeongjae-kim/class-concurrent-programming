#include <main.h>

// in main.cc
extern std::set<BMString> patterns;

typedef struct {
  const BMString* BMStringPtr;
  const char* pos;
} Answer;

// global variable only used in this file.
static const std::string* queryPtr = nullptr;
static std::vector<Answer> resultVector;


std::string query(const std::string& query) {
  queryPtr = &query;

  //sequential
  for (auto &pat : patterns) {
    auto searchResult = pat.search(queryPtr->begin(), queryPtr->end());
    if (searchResult != queryPtr->end()) {
      Answer foundAnswer;

      foundAnswer.BMStringPtr = &pat;
      foundAnswer.pos = &(*searchResult);

      resultVector.push_back(foundAnswer);
    }
  }

  // sort
  std::sort(resultVector.begin(), resultVector.end(),
      [](Answer a1, Answer a2) {
      if (a1.pos < a2.pos) {
      return true;
      } else if (a1.pos > a2.pos) {
      return false;
      } else {
      // when position is same,
      // short string is front
      if (a1.BMStringPtr->length() < a2.BMStringPtr->length()) {
      return true;
      //TODO: 디버깅 끝나면 이부분 비교하지 말기
      } else if(a1.BMStringPtr->length() > a2.BMStringPtr->length()){
      return false;
      } else {
      // There is no case of same length.
      std::cout << "(query) There is no case of same length." << std::endl;
      exit(EXIT_FAILURE);
      }
      }
      });


  //make solution
  std::string result;
  for (auto &i : resultVector) {
    result += *(i.BMStringPtr) + "|";
  }
  resultVector.clear();

  if (result.length() != 0) {
    result.pop_back();
  } else {
    result = "-1";
  }

  return result;
}
