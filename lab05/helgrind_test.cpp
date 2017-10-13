#include <pthread.h>

int var = 0;

void *thread_func(void *arg)
{
  var++;
  return NULL;
}

int main(void)
{
  pthread_t child;
  pthread_create(&child, NULL, thread_func, NULL);
  var++;
  pthread_join(child, NULL);
  return 0;
}
