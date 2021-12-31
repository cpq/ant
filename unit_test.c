#include "ant.h"
#include "stdio.h"

int main(void) {
  struct ant ant;
  ant_init(&ant);
  int res = ant_eval(&ant, "1 2 + 4 *", 0);
  printf("ant size: %d, eval result: %d\n", (int) sizeof(ant), res);
  return 0;
}
