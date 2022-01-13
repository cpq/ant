// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#pragma once

#if defined(_MSC_VER) && _MSC_VER < 1700
#define inline __inline
#define vsnprintf _vsnprintf
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long antval_t;

struct ant {
  const char *buf, *pc, *eof;
  antval_t vars[2];                // Variables
  antval_t stack[10];              // Variables
  void (*ops[256])(struct ant *);  // Opcode handlers
  char err[20];                    // Error message
  int sp;
};

static inline void op_nop(struct ant *ant) {
  ant->pc++;
}

static inline void op_pop(struct ant *ant) {
  ant->pc++;
  ant->sp--;
}

static inline void op_num(struct ant *ant) {
  ant->stack[ant->sp++] = strtoul(ant->pc, (char **) &ant->pc, 0);
  // printf("%d->%ld\n", ant->sp - 1, ant->stack[ant->sp - 1]);
}

static inline void op_lt(struct ant *ant) {
  antval_t *v = &ant->stack[ant->sp - 2];
  v[0] = v[0] < v[1] ? 1 : 0;
  ant->sp--;
  ant->pc++;
}

static inline void op_plus(struct ant *ant) {
  antval_t *v = &ant->stack[ant->sp - 2];
  v[0] += v[1];
  ant->sp--;
  ant->pc++;
}

static inline void op_div(struct ant *ant) {
  antval_t *v = &ant->stack[ant->sp - 2];
  v[0] /= v[1];
  ant->sp--;
  ant->pc++;
}

static inline void op_var(struct ant *ant) {
  ant->stack[ant->sp++] = ant->vars[*(unsigned char *) ant->pc - 'a'];
  ant->pc++;
}

static inline void op_assign(struct ant *ant) {
  ant->pc++;
  ant->vars[*(unsigned char *) ant->pc - 'a'] = ant->stack[--ant->sp];
  ant->pc++;
}

static inline void op_jump(struct ant *ant) {
  int inc = ant->pc[1] == 'b' ? -1 : 1;
  const char *limit = inc > 0 ? ant->eof : ant->buf;
  ant->pc += 2;
  if (ant->stack[--ant->sp]) {
    while (ant->pc != limit && *ant->pc != '#') ant->pc += inc;
  }
}

static inline void ant_init(struct ant *ant) {
  size_t i, nops = sizeof(ant->ops) / sizeof(ant->ops[0]);
  memset(ant, 0, sizeof(*ant));
  for (i = 0; i < nops; i++) ant->ops[i] = op_nop;
  for (i = '0'; i < '9'; i++) ant->ops[i] = op_num;
  for (i = 'a'; i < 'z'; i++) ant->ops[i] = op_var;
  ant->ops[(int) ';'] = op_pop;
  ant->ops[(int) '+'] = op_plus;
  ant->ops[(int) '/'] = op_div;
  ant->ops[(int) '='] = op_assign;
  ant->ops[(int) '@'] = op_jump;
  ant->ops[(int) '<'] = op_lt;
}

static inline antval_t ant_eval(struct ant *ant, const char *str) {
  ant->pc = ant->buf = str;
  ant->eof = &ant->pc[strlen(str)];
  ant->err[0] = '\0';
  ant->sp = 0;
  while (ant->pc < ant->eof) ant->ops[*(unsigned char *) ant->pc](ant);
  return ant->stack[0];
}
