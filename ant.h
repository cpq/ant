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
  const char *pc, *eof;
  int tok;                   // Parsed token
  antval_t val;              // Parsed value
  antval_t vars['z' - 'a'];  // Variables
  char err[20];              // Error message
};

enum { Inv, Eof, Num, Var };  // Tokens

static inline void ant_init(struct ant *ant) {
  memset(ant, 0, sizeof(*ant));
}

static inline int ant_next(struct ant *ant) {
  if (ant->tok != Inv) {
    // Do nothing. A previously parsed token has not been consumed, return it
    // printf("not consumed...\n");
  } else if (ant->pc >= ant->eof) {
    ant->tok = Eof;
  } else {
    while (isspace(*(unsigned char *) ant->pc)) ant->pc++;
    if (*ant->pc >= 'a' && *ant->pc <= 'z') {
      ant->tok = Var;
      ant->val = *ant->pc++ - 'a';
    } else if (isdigit(*(unsigned char *) ant->pc)) {
      ant->val = strtoul(ant->pc, (char **) &ant->pc, 0);
      ant->tok = Num;
    } else {
      ant->tok = *ant->pc++;
    }
  }
  // printf("TOK %d %c\n", ant->tok, ant->tok);
  return ant->tok;
}

static inline int ant_isnext(struct ant *ant, int t1, int t2) {
  int tok = ant_next(ant);
  if (tok == t1 || tok == t2) {
    // printf("consuming... %d %d %d\n", tok, t1, t2);
    ant->tok = Inv;
    return tok;
  } else {
    // printf("nope... %d %d %d\n", tok, t1, t2);
    return Inv;
  }
}

static inline void ant_err(struct ant *ant, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(ant->err, sizeof(ant->err), fmt, ap);
  va_end(ap);
  ant->err[sizeof(ant->err) - 1] = '\0';
  ant->tok = Eof;
  ant->pc = ant->eof;
}

static inline int ant_checktok(struct ant *ant, int t1, int t2) {
  int tok = ant_isnext(ant, t1, t2);
  if (tok == Inv) ant_err(ant, "%s", "parse error");
  return tok;
}

static inline antval_t ant_num_or_var(struct ant *ant) {
  int tok = ant_next(ant);
  ant->tok = Inv;
  if (tok == Var) {
    ant->val = ant->vars[ant->val];
  } else if (tok == Num) {
  } else {
    ant_err(ant, "%s", "parse error");
  }
  // printf(">>> [%s] %d (%s)\n", ant->pc, ant->tok, ant->err);
  return ant->val;
}

static antval_t ant_expr(struct ant *ant);
static inline antval_t ant_parenthesis(struct ant *ant) {
  if (ant_isnext(ant, '(', Inv) != Inv) {
    antval_t res = ant_expr(ant);
    ant_checktok(ant, ')', Inv);
    // printf("--> [%s] %d\n", ant->pc, ant->err);
    return res;
  } else {
    // printf("==>\n");
    return ant_num_or_var(ant);
  }
}

static inline antval_t ant_mul_or_div(struct ant *ant) {
  antval_t res = ant_parenthesis(ant);
  int tok = ant_isnext(ant, '*', '/');
  if (tok != Inv) {
    antval_t x = ant_mul_or_div(ant);
    res = tok == '*' ? res * x : res / x;
  }
  return res;
}

static antval_t ant_add_or_sub(struct ant *ant);
static inline antval_t ant_add_or_sub2(struct ant *ant, antval_t res) {
  int tok = ant_isnext(ant, '+', '-');
  // printf("==> ADDSUB: %d [%s]\n", tok, ant->pc);
  if (tok != Inv) {
    antval_t x = ant_add_or_sub(ant);
    res = tok == '+' ? res + x : res - x;
  }
  return res;
}

static inline antval_t ant_add_or_sub(struct ant *ant) {
  return ant_add_or_sub2(ant, ant_mul_or_div(ant));
}

static inline antval_t ant_assignment(struct ant *ant) {
  if (ant_isnext(ant, Var, Inv) != Inv) {
    int varindex = ant->val;
    if (ant_isnext(ant, '=', Inv) != Inv) {
      return ant->vars[varindex] = ant_expr(ant);
    } else {
      return ant_add_or_sub2(ant, ant->vars[varindex]);
    }
  } else {
    return ant_add_or_sub(ant);
  }
}

static inline antval_t ant_expr(struct ant *ant) {
  return ant_assignment(ant);
}

static inline void ant_stmt_list(struct ant *ant, int etok) {
  int tok;
  while ((tok = ant_next(ant)) != etok || tok != Eof) {
    if (tok == ';') {
      ant->tok = Inv;
      continue;
    } else {
      ant->val = ant_expr(ant);
    }
  }
}

static inline antval_t ant_eval(struct ant *ant, const char *str) {
  ant->pc = str;
  ant->eof = &ant->pc[strlen(str)];
  ant->err[0] = '\0';
  ant->tok = Inv;
  ant_stmt_list(ant, Eof);
  return ant->val;
  // return ant_expr(ant);
}
