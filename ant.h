// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#pragma once

#include <stddef.h>
#include <stdio.h>
#include <string.h>

struct ant {
  int vars['z' - 'a'];
  int stack[10];
  void (*ops[256])(struct ant *);
  int sp;
  const char *code;
  size_t len;
  size_t pos;
};

static inline void op_nop(struct ant *ant) {
  ant->pos++;
}

static inline void op_pop(struct ant *ant) {
  ant->sp--;
  ant->pos++;
}

static inline void op_num(struct ant *ant) {
  int num = 0;
  while (ant->pos < ant->len && ant->code[ant->pos] >= '0' &&
         ant->code[ant->pos] <= '9') {
    num *= 10;
    num += ant->code[ant->pos++] - '0';
  }
  ant->stack[ant->sp++] = num;
}

static inline void op_plus(struct ant *ant) {
  int *v = &ant->stack[ant->sp - 2];
  v[0] += v[1];
  ant->sp--;
  ant->pos++;
}

static inline void op_mul(struct ant *ant) {
  int *v = &ant->stack[ant->sp - 2];
  v[0] *= v[1];
  ant->sp--;
  ant->pos++;
}

static inline void ant_init(struct ant *ant) {
  size_t i, nops = sizeof(ant->ops) / sizeof(ant->ops[0]);
  memset(ant, 0, sizeof(*ant));
  for (i = 0; i < nops; i++) ant->ops[i] = op_nop;
  for (i = '0'; i < '9'; i++) ant->ops[i] = op_num;
  ant->ops[';'] = op_pop;
  ant->ops['+'] = op_plus;
  ant->ops['*'] = op_mul;
}

static inline int ant_eval(struct ant *ant, const char *script, size_t len) {
  ant->code = script;
  ant->len = len == 0 ? strlen(script) : len;
  ant->pos = 0;
  while (ant->pos < ant->len) {
    ant->ops[((unsigned char *) ant->code)[ant->pos]](ant);
    printf("'%.*s' %d %d %d,%d,%d,%d,%d\n", (int) ant->len, script,
           (int) ant->pos, ant->sp, ant->stack[0], ant->stack[1], ant->stack[2],
           ant->stack[3], ant->stack[4]);
  }
  return ant->stack[0];
}
