/** Metadata
 * Author               : Kim, Myeong Jae
 * File Name            : main.cc
 * Due date             : 2017-11-6
 * Compilation Standard : c++11 */

#include <iostream>
#include <pthread.h>
#include <unistd.h>


#include "snapshot.h"
#include "stamped_value.h"
#include "WFSnapshot.h"

WFSnapshot_t *ss;

bool end = false;
int64_t* num_of_update;
int64_t num_of_threads;

void* thread_func(void* arg) {
  int64_t tid = (int64_t)arg;
  int64_t value = 0;
  while (end == false) {
    ss->update(++value, tid);
    num_of_update[tid]++;
    /* std::cout << "[tid: " << tid << "] "
     *   << "# of update == " << num_of_update[tid] << std::endl; */
  }
  return nullptr;
}


int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <# of threads>" << std::endl;
    exit(1);
  }

  num_of_threads = atoi(argv[1]);
  if (num_of_threads == 0) {
    std::cout << "<# of threads> should be numeric value and bigger than  0" << std::endl;
    exit(1);
  }

  ss = new WFSnapshot_t(num_of_threads, 0);

  num_of_update =
    (int64_t*)calloc(num_of_threads, sizeof(*num_of_update));


  pthread_t* threads = (pthread_t*)malloc(num_of_threads * 
      sizeof(*threads));

  for (int64_t i = 0; i < num_of_threads; ++i) {
    if (pthread_create(&threads[i], nullptr, thread_func, (void*)i) < 0) {
      fprintf(stderr, "(main) thread creating error!\n");
      exit(1);
    }
  }

  sleep(60);

  end = true;
  int64_t sum_of_update_num = 0;
  for (int i = 0; i < num_of_threads; ++i) {
    pthread_join(threads[i], nullptr);
    sum_of_update_num += num_of_update[i];
  }
  
  std::cout << "update : " << sum_of_update_num << std::endl;

  free(threads);
  free(num_of_update);
  delete ss;
  return 0;
}
