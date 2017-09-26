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

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* searchSubstr(void* BMStringPtrArg) {
  const BMString* BMStringPtr = (const BMString*) BMStringPtrArg;
  Answer foundAnswer;

  auto searchResult = BMStringPtr->search(queryPtr->begin(), queryPtr->end());

  if (searchResult != queryPtr->end()) {
    foundAnswer.BMStringPtr = BMStringPtr;
    foundAnswer.pos = &(*searchResult);

    pthread_mutex_lock(&mutex);
    resultVector.push_back(foundAnswer);
    pthread_mutex_unlock(&mutex);
  }

  return nullptr;
}

std::string query(const std::string& query) {
  queryPtr = &query;

  //sequential
/*   for (auto &pat : patterns) {
 *     auto searchResult = pat.search(queryPtr->begin(), queryPtr->end());
 *     if (searchResult != queryPtr->end()) {
 *       Answer foundAnswer;
 *
 *       foundAnswer.BMStringPtr = &pat;
 *       foundAnswer.pos = &(*searchResult);
 *
 *       resultVector.push_back(foundAnswer);
 *     }
 *   } */


  pthread_t *threads = (pthread_t*)malloc(patterns.size() * sizeof(pthread_t));
  int count = 0;
  for (auto &pat : patterns) {
    pthread_create(&threads[count++], NULL, searchSubstr, (void*)&pat);
  }

  // count is the number of threads
  for (int i = 0; i < count; ++i) {
    pthread_join(threads[i], NULL);
  }
  free(threads);
  threads = nullptr;


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
  queryPtr = nullptr;

  if (result.length() != 0) {
    result.pop_back();
  } else {
    result = "-1";
  }

  return result;
}
