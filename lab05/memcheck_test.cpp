#include <stdio.h>
#include <stdlib.h>

int main(void)
{
  int p, t;
  char *pc;
  if (p == 5) {
    t = p + 1;
  }

  printf("%d is not initialized\n", p);

  pc = (char *)malloc(sizeof(char)* 10);

  return 0;
}
