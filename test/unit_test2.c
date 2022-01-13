// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#include <assert.h>
#include "../ant2.h"

static void check(struct ant *ant, const char *buf, antval_t expected) {
  antval_t res = ant_eval(ant, buf);
  printf("[%s] \t=> %ld %ld %d\n", buf, res, expected, ant->sp);
  if (res != expected || ant->sp != 1) exit(1);
}

int main(void) {
  struct ant ant;
  ant_init(&ant);
  printf("ant size: %d\n", (int) sizeof(ant));
  check(&ant, "1", 1);
  check(&ant, "1 2 +", 3);
  check(&ant, "7 =a 3 =b a", 7);
  check(&ant, "1 2 <", 1);
  check(&ant, "2 1 <", 0);
  check(&ant, "1 1 <", 0);
  check(&ant, "1 =a 1 @f 7 =a # a", 1);
  check(&ant, "1 =a 0 @f 7 =a # a", 7);
  check(&ant, "# 0 @b 1", 1);
  check(&ant, "0 =a 0 =i # a i + =a 1 i + =i i 10 < @b a", 45);
  return 0;
}
