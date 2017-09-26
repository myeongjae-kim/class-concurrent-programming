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

static pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;

void* searchSubstr(void* BMStringPtrArg) {
  const BMString* BMStringPtr = (const BMString*) BMStringPtrArg;
  Answer foundAnswer;

  auto searchResult = BMStringPtr->search(queryPtr->begin(), queryPtr->end());

  if (searchResult != queryPtr->end()) {
    foundAnswer.BMStringPtr = BMStringPtr;
    foundAnswer.pos = &(*searchResult);

    pthread_mutex_lock(&result_mutex);
    resultVector.push_back(foundAnswer);
    pthread_mutex_unlock(&result_mutex);
  }

  return nullptr;
}

// for make threads sleep and wakeup
BMString* qry_thread_arg[NUM_THREAD];

bool finished = 0;

pthread_cond_t qry_cond;
pthread_mutex_t qry_mutex = PTHREAD_MUTEX_INITIALIZER;


void* condSearchSubstr(void* arg) {
  long tid = (long)arg;

  // init
  qry_thread_arg[tid] = nullptr;

  // wait
  pthread_mutex_lock(&qry_mutex);

#ifdef DBG
  std::cout << "(thread " << tid << ") is go to sleep" << std::endl;
#endif


  pthread_cond_wait(&qry_cond, &qry_mutex);
  pthread_mutex_unlock(&qry_mutex);

#ifdef DBG
  std::cout << "(thread " << tid << ") is waken up" << std::endl;
#endif

  while (!finished) {
    // do work
    if (qry_thread_arg[tid] != nullptr) {
      Answer foundAnswer;

      auto searchResult = qry_thread_arg[tid]->search(queryPtr->begin(), queryPtr->end());

      if (searchResult != queryPtr->end()) {
        foundAnswer.BMStringPtr = qry_thread_arg[tid];
        foundAnswer.pos = &(*searchResult);

        pthread_mutex_lock(&result_mutex);
        resultVector.push_back(foundAnswer);
        pthread_mutex_unlock(&result_mutex);
      }
    }


    pthread_mutex_lock(&qry_mutex);
    //reinit
    qry_thread_arg[tid] = nullptr;

    // send result
    pthread_cond_wait(&qry_cond, &qry_mutex);
    pthread_mutex_unlock(&qry_mutex);
  }

  return nullptr;
}

// end




std::string query(const std::string& query) {
  queryPtr = &query;

  int tid = 0;
  for (auto &pat : patterns) {
    qry_thread_arg[tid] = (BMString*)&pat;
    tid++;
    tid %= NUM_THREAD;

    // full of argument
    if (tid == 0) {

      // thread wake up
      pthread_mutex_lock(&qry_mutex);
      pthread_cond_broadcast(&qry_cond);
      pthread_mutex_unlock(&qry_mutex);

      // and wait to the end
      for (long i = 0; i < NUM_THREAD; ++i) {
        while (qry_thread_arg[i] != nullptr) {
          // pthread_yield();
        }
      }
    }
  }

  if (tid != 0) {
    // wake up left threads
    pthread_mutex_lock(&qry_mutex);
    pthread_cond_broadcast(&qry_cond);
    pthread_mutex_unlock(&qry_mutex);

    // and wait to the end
    for (long i = 0; i < NUM_THREAD; ++i) {
      while (qry_thread_arg[i] != nullptr) {
        // pthread_yield();
      }
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
  queryPtr = nullptr;

  if (result.length() != 0) {
    result.pop_back();
  } else {
    result = "-1";
  }

  return result;
}
