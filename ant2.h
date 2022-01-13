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
  antval_t vars['z' - 'a'];  // Variables
  antval_t stack[10];        // Stack
  int sp;                    // Stack pointer
};

static inline void ant_init(struct ant *ant) {
  memset(ant, 0, sizeof(*ant));
}

static inline antval_t ant_eval(struct ant *ant, const char *str) {
  ant->pc = ant->buf = str;
  ant->eof = &ant->pc[strlen(str)];
  ant->sp = 0;
  while (ant->pc < ant->eof) {
    antval_t *v;
    // clang-format off
    switch (*ant->pc++) {
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
      case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
      case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
      case 'v': case 'w': case 'x': case 'y': case 'z':
        ant->stack[ant->sp++] = ant->vars[ant->pc[-1] - 'a'];
        break;
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        ant->stack[ant->sp++] = strtoul(&ant->pc[-1], (char **) &ant->pc, 0);
        break;
      case '=':
        ant->vars[*ant->pc++ - 'a'] = ant->stack[--ant->sp];
        break;
      case '+': 
        v = &ant->stack[ant->sp-- - 2];
        v[0] += v[1];
        break;
      case '*':
        v = &ant->stack[ant->sp-- - 2];
        v[0] *= v[1];
        break;
      case '/':
        v = &ant->stack[ant->sp-- - 2];
        v[0] /= v[1];
        break;
      case '@': {
        int inc = *ant->pc++ == 'b' ? -1 : 1;
        const char *limit = inc > 0 ? ant->eof : ant->buf;
        //printf("JMP %d %ld\n", ant->sp, ant->stack[ant->sp -1]);
        if (ant->stack[--ant->sp]) {
          while (ant->pc != limit && *ant->pc != '#') ant->pc += inc;
        }
        break;
      }
      case 'I':
        ant->vars[*ant->pc++ - 'a']++;
        break;
      case '<':
        v = &ant->stack[ant->sp-- - 2];
        v[0] = v[0] < v[1] ? 1 : 0;
        break;
      case '>':
        v = &ant->stack[ant->sp-- - 2];
        v[0] = v[0] > v[1] ? 1 : 0;
        break;
      case ';':
        ant->sp--;
        break;
      default: break;
    }
    // clang-format on
  }
  return ant->stack[0];
}
