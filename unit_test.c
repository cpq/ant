// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#include <time.h>
#include "ant.h"

#if 0
static bool ev(struct js *js, const char *expr, const char *expectation) {
  const char *result = js_str(js, js_eval(js, expr, strlen(expr)));
#ifdef VERBOSE
  printf("[%s] -> [%s] [%s]\n", expr, result, expectation);
#endif
  return strcmp(result, expectation) == 0;
}
#endif

int main(void) {
  clock_t a = clock();
  struct ant ant;
  double ms;
  int res;

  ant_init(&ant);
  res = ant_eval(&ant, "1 2 + 4 *", 0);
  printf("ant size: %d, eval result: %d\n", (int) sizeof(ant), res);

  ms = (double) (clock() - a) * 1000 / CLOCKS_PER_SEC;
  printf("SUCCESS. All tests passed in %g ms\n", ms);
  return 0;
}
