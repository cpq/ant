// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#include <assert.h>
#include "../ant2.h"

static void check(struct ant *ant, const char *buf, antval_t expected,
                  const char *errstr) {
  antval_t res = ant_eval(ant, buf);
  printf("'%s' = %ld, expected %ld (%s) %d\n", buf, res, expected, ant->err,
         ant->sp);
  if (res != expected || strcmp(ant->err, errstr) != 0) exit(1);
}

int main(void) {
  struct ant ant;
  ant_init(&ant);
  printf("ant size: %d\n", (int) sizeof(ant));
  check(&ant, "", 0, "");
  check(&ant, "1", 1, "");
  check(&ant, "1 2 +", 3, "");
  check(&ant, "7 =a 3 =b a", 7, "");
  check(&ant, "0 =a 0 =b # a b + =a 1 b + =b b 10 < @b a", 45, "");
#if 0
  check(&ant, "c += b", 18, "");
  check(&ant, "a = 1; b = 2; a == b", 0, "");
  check(&ant, "a = 1; b = 2; a == 1", 1, "");
  check(&ant, "a = 1; b = 2; a < b", 1, "");
  check(&ant, "a = 1; b = 2; a > b", 0, "");
  check(&ant, "a=0; i=0; # a += i; i += 1; @b i<10; a", 45, "");
#endif
  return 0;
}
