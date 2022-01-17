// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#include <assert.h>
#include "../ant.h"

static void check(struct ant *ant, const char *buf, antval_t expected,
                  const char *errstr) {
  antval_t res = ant_eval(ant, buf);
  printf("'%s' = %ld, expected %ld (%s)\n", buf, res, expected, ant->err);
  if (res != expected || strcmp(ant->err, errstr) != 0) exit(1);
}

static void test_ant(void) {
  struct ant ant = ANT_INITIALIZER;
  // ant_init(&ant);
  printf("ant size: %d\n", (int) sizeof(ant));
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
  check(&ant, "a=0; i=0; # a += i; i += 1; @b i<10; a", 45, "");
}

static void check2(struct ant2 *ant, const char *buf, antval_t expected) {
  antval_t res = ant2_eval(ant, buf);
  printf("[%s] \t=> %ld %ld %d\n", buf, res, expected, ant->sp);
  if (res != expected || ant->sp != 1) exit(1);
}

static void test_ant2(void) {
  struct ant2 ant = ANT2_INITIALIZER;
  printf("ant2 size: %d\n", (int) sizeof(ant));
  check2(&ant, "1", 1);
  check2(&ant, "1 2 +", 3);
  check2(&ant, "7 =a 3 =b a", 7);
  check2(&ant, "1 2 <", 1);
  check2(&ant, "2 1 <", 0);
  check2(&ant, "1 1 <", 0);
  check2(&ant, "1 =a 1 @f 7 =a # a", 1);
  check2(&ant, "1 =a 0 @f 7 =a # a", 7);
  check2(&ant, "# 0 @b 1", 1);
  check2(&ant, "0 =a 0 =i # a i + =a 1 i + =i i 10 < @b a", 45);
  check2(&ant, "0 =a 0 =i # a i + =a Ii i 10 < @b a", 45);
  check2(&ant, "0=a 0=i 1000=d  # ai+i3/+=a Ii id< @b a", 665667);
}

static void check3(struct ant3 *ant, const unsigned char *pc, antval_t exp) {
  antval_t res = ant3_eval(ant, pc);
  printf("%ld %ld %d\n", res, exp, ant->sp);
  if (res != exp) exit(1);
}

static void test_ant3(void) {
  {
    struct ant3 ant = {{3, 1000}, {0}, {0}, 0};
    unsigned char code[] = {IncVar, 0, PushVar, 0, Done};
    check3(&ant, code, 1);
  }
  {
    struct ant3 ant = {{3, 1000}, {0}, {0}, 0};
    unsigned char code[] = {PushImm, 0, Done};
    check3(&ant, code, 3);
  }
  {
    struct ant3 ant = {{3, 1000}, {0}, {0}, 0};
    unsigned char code[] = {IncVar, 1, PushVar, 1, PopVar,  0, CmpVarImm,
                            1,      1, Jump,    0, PushVar, 0, Done};
    check3(&ant, code, 1000);
  }
  {
    struct ant3 ant = {{3, 1000}, {0}, {0}, 0};
    unsigned char code[] = {PushVar, 0,       PushVar, 1,         Plus, PushVar,
                            1,       PushImm, 0,       Div,       Plus, PopVar,
                            0,       IncVar,  1,       CmpVarImm, 1,    1,
                            Jump,    0,       PushVar, 0,         Done};
    check3(&ant, code, 665667);
  }
}

static void check4(const char *buf, antval_t expected) {
  char tmp[200];
  struct ant4 *ant = ant4_create(tmp, sizeof(tmp));
  antval_t res = ant4_eval(ant, buf);
  printf("[%s] \t=> %ld %ld\n", buf, res, expected);
  if (res != expected) exit(1);
}

static void test_ant4(void) {
  // printf("%s %d\n", __func__, (int) ((char *) ant->estk - (char *) ant));
  check4("", 0);
  check4("0", 0);
  check4("17", 17);
  check4("1 + 2", 3);
  check4("1 + 2 + 3", 6);
  check4("1 + 2 + 3 * 4", 15);
}

int main(void) {
  test_ant();
  test_ant2();
  test_ant3();
  test_ant4();
  return 0;
}
