// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#include <assert.h>
#include "ant.h"

static void check(struct ant *ant, const char *buf, antval_t expected,
                  const char *errstr) {
  antval_t res = ant_eval(ant, buf);
  printf("'%s' = %ld, expected %ld (%s)\n", buf, res, expected, ant->err);
  if (res != expected || strcmp(ant->err, errstr) != 0) exit(1);
}

int main(void) {
  struct ant ant;
  ant_init(&ant);
  printf("ant size: %d\n", (int) sizeof(ant));
  // check(&ant, "", 0, "parse error");
  check(&ant, "", 0, "");
  check(&ant, "1", 1, "");
  check(&ant, "1 + 2", 3, "");
  check(&ant, "1 - 2", -1, "");
  check(&ant, "6/2", 3, "");
  check(&ant, "1 + 2 * 3", 7, "");
  check(&ant, "(1 + 2) * 3", 9, "");
  check(&ant, "(6 / (1 + 1)) * 3", 9, "");
  check(&ant, "a + 1", 1, "");
  check(&ant, "1;2", 2, "");
  check(&ant, "a = 7", 7, "");
  check(&ant, "a + 1", 8, "");
  check(&ant, "b = c = 17", 17, "");
  check(&ant, "c", 17, "");
  check(&ant, "b", 17, "");
  check(&ant, "b = b + 1", 18, "");
  check(&ant, "b -= c", 1, "");
  check(&ant, "c += b", 18, "");
  check(&ant, "a = 1; b = 2; a == b", 0, "");
  check(&ant, "a = 1; b = 2; a == 1", 1, "");
  check(&ant, "a = 1; b = 2; a < b", 1, "");
  check(&ant, "a = 1; b = 2; a > b", 0, "");
  check(&ant, "a=0; i=0; # a += i; i += 1; i < 10 @tb a", 45, "");
  return 0;
}
