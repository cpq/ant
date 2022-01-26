// Copyright (c) 2022 Cesanta Software Limited
// All rights reserved

#pragma once

#if defined(_MSC_VER) && _MSC_VER < 1700
#define inline __inline
#define vsnprintf _vsnprintf
typedef unsigned char uint8_t;
#else
#include <stdbool.h>
#include <stdint.h>
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
  int tok;                   // Parsed token
  antval_t val;              // Parsed value
  antval_t vars['z' - 'a'];  // Variables
  char err[20];              // Error message
};
#define ANT_INITIALIZER \
  { 0, 0, 0, 0, 0, {0}, "" }
enum { Inv, Eof, Num, Var, Inc, Dec, Eq };  // Tokens

static inline int ant_next(struct ant *ant) {
  if (ant->tok != Inv) {
    // Do nothing. A previously parsed token has not been consumed, return it
    // printf("not consumed...\n");
  } else if (ant->pc >= ant->eof) {
    ant->tok = Eof;
  } else {
    while (isspace(*(unsigned char *) ant->pc)) ant->pc++;
    // clang-format off
    switch (*ant->pc) {
      case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
      case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
      case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
      case 'v': case 'w': case 'x': case 'y': case 'z':
        ant->tok = Var;
        ant->val = *ant->pc++ - 'a';
        break;
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        ant->val = strtoul(ant->pc, (char **) &ant->pc, 0);
        ant->tok = Num;
        break;
      case '=':
        ant->tok = ant->pc[1] == '=' ? (int) Eq : (int) '=';
        ant->pc += ant->tok == Eq ? 2 : 1;
        break;
      case '+':
        ant->tok = ant->pc[1] == '=' ? (int) Inc : (int) '+';
        ant->pc += ant->tok == Inc ? 2 : 1;
        break;
      case '-':
        ant->tok = ant->pc[1] == '=' ? (int) Dec : (int) '-';
        ant->pc += ant->tok == Dec ? 2 : 1;
        break;
      default:
        ant->tok = *ant->pc++;
        break;
    }
  }
  // clang-format on
  // printf("TOK %d %c\n", ant->tok, ant->tok);
  return ant->tok;
}

static inline void ant_swallow(struct ant *ant) {
  ant->tok = Inv;
}

static inline int ant_isnext(struct ant *ant, int t1, int t2) {
  int tok = ant_next(ant);
  if (tok == t1 || tok == t2) {
    // printf("consuming... %d %d %d\n", tok, t1, t2);
    ant_swallow(ant);
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
  ant_swallow(ant);
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

static inline antval_t ant_eq_more_less2(struct ant *ant, antval_t res) {
  int tok = ant_next(ant);
  if (tok == Eq) {
    ant_swallow(ant);
    return res == ant_expr(ant) ? 1 : 0;
  } else if (tok == '<') {
    ant_swallow(ant);
    return res < ant_expr(ant) ? 1 : 0;
  } else if (tok == '>') {
    ant_swallow(ant);
    return res > ant_expr(ant) ? 1 : 0;
  } else {
    return ant_add_or_sub2(ant, res);
  }
}

static inline antval_t ant_eq_more_less(struct ant *ant) {
  return ant_eq_more_less2(ant, ant_add_or_sub(ant));
}

static inline antval_t ant_assignment(struct ant *ant) {
  if (ant_isnext(ant, Var, Inv) != Inv) {
    int varindex = ant->val, tok = ant_next(ant);
    if (tok == '=') {
      ant_swallow(ant);
      return ant->vars[varindex] = ant_expr(ant);
    } else if (tok == Inc) {
      ant_swallow(ant);
      return ant->vars[varindex] += ant_expr(ant);
    } else if (tok == Dec) {
      ant_swallow(ant);
      return ant->vars[varindex] -= ant_expr(ant);
    } else {
      return ant_eq_more_less2(ant, ant->vars[varindex]);
    }
  } else {
    return ant_eq_more_less(ant);
  }
}

static inline antval_t ant_expr(struct ant *ant) {
  return ant_assignment(ant);
}

static inline void ant_jump(struct ant *ant) {
  int inc = *ant->pc++ == 'b' ? -1 : 1;
  if (ant_expr(ant)) {
    const char *limit = inc > 0 ? ant->eof : ant->buf;
    while (ant->pc != limit && *ant->pc != '#') ant->pc += inc;
    if (*ant->pc == '#') ant->pc++;  // Skip label
  }
}

static inline void ant_stmt_list(struct ant *ant, int etok) {
  int tok;
  while ((tok = ant_next(ant)) != etok || tok != Eof) {
    if (tok == ';') {
      ant_swallow(ant);
      continue;
    } else if (tok == '#') {
      ant_swallow(ant);
    } else if (tok == '@') {
      ant_swallow(ant);
      ant_jump(ant);
    } else {
      ant->val = ant_expr(ant);
    }
  }
}

#if 0
static inline int ant_copy(struct ant *ant, const char *str) {
  int i = 0, j = 0;
  while (str[i] != 0) {
    if (isspace(*(unsigned char *) &str[i])) {
      i++;
    } else {
      ant->code[j++] = str[i++];
    }
  }
  ant->code[j] = '\0';
  return j;
}
#endif

static inline antval_t ant_eval(struct ant *ant, const char *str) {
  // int n = ant_copy(ant, str);
  ant->pc = ant->buf = str;
  ant->eof = &ant->pc[strlen(str)];
  ant->err[0] = '\0';
  ant->tok = Inv;
  ant_stmt_list(ant, Eof);
  return ant->val;
}

/////////////////////////////////////////////// ANT 2
struct ant2 {
  const char *buf, *pc, *eof;
  antval_t vars['z' - 'a'];  // Variables
  antval_t stack[10];        // Stack
  int sp;                    // Stack pointer
};

#define ANT2_INITIALIZER \
  { 0, 0, 0, {0}, {0}, 0 }

static inline antval_t ant2_eval(struct ant2 *ant, const char *str) {
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

/////////////////////////////////////////////// ANT 3
struct ant3 {
  antval_t imm[10];          // Immediate values
  antval_t vars['z' - 'a'];  // Variables
  antval_t stack[10];        // Stack
  int sp;
};

enum {
  // OP       params               Description
  Done,       //                    The end
  IncVar,     // var_idx            Increment variable by var index
  Assign,     // var_idx imm_idx    Assign value to a variable
  PushVar,    // var_idx            Push variable to stack
  PopVar,     // var_idx            Pop to variable from stack
  PushImm,    // var_idx            Push immediate to stack
  Plus,       //                    Add on-stack values
  Div,        //                    Divide on-stack values
  Pop,        //                    Pop value from stack
  CmpVarImm,  // var_idx imm_idx    Push comparison (var - imm) result
  Jump,       // offset             Jump if stack top is non zero
};

static inline antval_t ant3_eval(struct ant3 *ant, const unsigned char *pc) {
  const unsigned char *saved = pc;
  ant->sp = 0;
  while (*pc) {
    antval_t *v;
    switch (*pc++) {
      case IncVar:
        ant->vars[*pc++]++;
        break;
      case PushVar:
        ant->stack[ant->sp++] = ant->vars[*pc++];
        break;
      case PopVar:
        ant->vars[*pc++] = ant->stack[--ant->sp];
        break;
      case PushImm:
        ant->stack[ant->sp++] = ant->imm[*pc++];
        break;
      case Assign:
        ant->vars[pc[0]] = ant->imm[pc[1]];
        pc += 2;
        break;
      case Plus:
        v = &ant->stack[ant->sp-- - 2];
        v[0] += v[1];
        break;
      case Div:
        v = &ant->stack[ant->sp-- - 2];
        v[0] /= v[1];
        break;
      case CmpVarImm: {
        antval_t var = ant->vars[*pc++], imm = ant->imm[*pc++];
        // printf("CMP %ld %ld\n", var, imm);
        ant->stack[ant->sp++] = var - imm;
        break;
      }
      case Jump: {
        unsigned char offset = *pc++;
        if (ant->stack[--ant->sp]) pc = saved + offset;
        break;
      }
      default:
        break;
    }
  }
  return ant->stack[0];
}

// Using computed goto. Available on GCC and Clang
#if defined(__GNUC__) || defined(__clang__)
static inline antval_t ant3_eval2(struct ant3 *ant, const unsigned char *pc) {
  void *tab[] = {&&Done, &&IncVar, &&Assign, &&PushVar,   &&PopVar, &&PushImm,
                 &&Plus, &&Div,    &&Pop,    &&CmpVarImm, &&Jump};
  const unsigned char *saved = pc;
  antval_t *v;
  ant->sp = 0;
  goto *tab[*pc++];
IncVar:
  // printf("INC\n");
  ant->vars[*pc++]++;
  goto *tab[*pc++];
PushVar:
  // printf("PushV\n");
  ant->stack[ant->sp++] = ant->vars[*pc++];
  goto *tab[*pc++];
PopVar:
  // printf("PopV\n");
  ant->vars[*pc++] = ant->stack[--ant->sp];
  goto *tab[*pc++];
PushImm:
  // printf("PushImm\n");
  ant->stack[ant->sp++] = ant->imm[*pc++];
  goto *tab[*pc++];
Assign:
  // printf("Assign\n");
  ant->vars[pc[0]] = ant->imm[pc[1]];
  pc += 2;
  goto *tab[*pc++];
Plus:
  // printf("Plus\n");
  v = &ant->stack[ant->sp-- - 2];
  v[0] += v[1];
  goto *tab[*pc++];
Div:
  // printf("Div\n");
  v = &ant->stack[ant->sp-- - 2];
  v[0] /= v[1];
  goto *tab[*pc++];
Pop:
  // printf("Pop\n");
  goto *tab[*pc++];
CmpVarImm : {
  antval_t var = ant->vars[*pc++], imm = ant->imm[*pc++];
  // printf("CMP %ld %ld\n", var, imm);
  ant->stack[ant->sp++] = var - imm;
  goto *tab[*pc++];
}
Jump : {
  unsigned char offset = *pc++;
  // printf("Jump\n");
  if (ant->stack[--ant->sp]) pc = saved + offset;
  goto *tab[*pc++];
}
Done:
  // printf("Done\n");
  return ant->stack[0];
}
#endif  // __GNUC__ or __clang__

/////////////////////////////////////////////// ANT 4
struct ant4 {
  const char *s;  // Source code. Required by compiler
  antval_t val;   // Parsed value. Required by compiler
  int tok;

  uint8_t *ssym, *esym;        // Symbol table
  uint8_t *scode, *ecode;      // Code
  antval_t *sstk, *estk, *sp;  // Stack
};

enum { AEOF, AINC, APLUS, AMUL, APUSH, APOP };
enum { TINV, TEOF, TINC, TPLUS, TMUL, TNUM, TVAR };

static inline int ant4_next(struct ant4 *ant) {
  if (ant->tok != TINV) return ant->tok;
  if (*ant->s == 0) return TEOF;
  while (*ant->s == ' ') ant->s++;
  // clang-format off
  switch(*ant->s++) {
#if 0
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':
      ant->tok = Var;
      ant->val = *ant->pc++ - 'a';
      break;
#endif
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      ant->val = strtoul(ant->s -1, (char **) &ant->s, 0);
      ant->tok = TNUM;
      break;
    case '+': ant->tok = TPLUS; break;
    case '*': ant->tok = TMUL; break;
    default: ant->tok = TEOF; break;
  }
  // clang-format on
  // printf("tok : %d\n", ant->tok);
  return ant->tok;
}

static inline void ant4_num(struct ant4 *ant) {
  if (ant4_next(ant) == TNUM) {
    ant->tok = TINV;
    *ant->estk-- = ant->val;
    *ant->ecode++ = APUSH;
    *ant->ecode++ = ant->sstk - ant->estk - 1;
    // printf("---> %ld %d\n", ant->estk[1], ant->ecode[-1]);
  }
}

static inline void ant4_mul_div(struct ant4 *ant) {
  ant4_num(ant);
  if (ant4_next(ant) == TMUL) {
    ant->tok = TINV;
    ant4_mul_div(ant);
    *ant->ecode++ = AMUL;
  }
}

static inline void ant4_add_sub(struct ant4 *ant) {
  ant4_mul_div(ant);
  if (ant4_next(ant) == TPLUS) {
    ant->tok = TINV;
    ant4_add_sub(ant);
    *ant->ecode++ = APLUS;
  }
}

static inline void ant4_compile(struct ant4 *ant) {
  ant->tok = TINV;
  ant4_add_sub(ant);
  *ant->ecode++ = 0;
}

static inline void ant4_stk(antval_t *sp, antval_t *esp, const char *msg) {
  printf("%s", msg);
  while (sp > esp) printf("%ld ", *sp--);
  putchar('\n');
}

static inline antval_t ant4_exec(struct ant4 *ant) {
  antval_t *sp = ant->estk, *esp = ant->sstk;
  uint8_t *pc = ant->scode;
  sp[0] = 0;
  // ant4_stk(esp, sp, "BEFORE EXEC ");
  while (*pc) {
    switch (*pc++) {
      case APOP:
        sp[0] = esp[-*pc++];
        sp++;
        // ant4_stk(esp, sp, "POP ");
        break;
      case APUSH:
        sp[0] = esp[-*pc++];
        sp--;
        // ant4_stk(esp, sp, "PUSH ");
        break;
      case APLUS:
        sp[2] += sp[1];
        sp++;
        // ant4_stk(esp, sp, "PLUS ");
        break;
      case AMUL:
        sp[2] *= sp[1];
        sp++;
        // ant4_stk(esp, sp, "MUL ");
        break;
      default:
        break;
    }
  }
  return ant->estk[0];
}

static inline antval_t ant4_eval(struct ant4 *ant, const char *str) {
  // printf("=========================== EVAL %s\n", str);
  ant->s = str;
  ant4_compile(ant);
  return ant4_exec(ant);
}

static size_t ant_roundup(size_t size, size_t align) {
  return (size + align - 1) / align * align;
}

static inline struct ant4 *ant4_create(void *buf, size_t len) {
  struct ant4 *ant = (struct ant4 *) buf;
  size_t align = sizeof(antval_t);
  if (len < sizeof(*ant) + 2 * sizeof(antval_t)) return NULL;
  ant->ssym = ant->esym =
      &((uint8_t *) buf)[ant_roundup(len - align - 1, align)];
  ant->scode = ant->ecode = (uint8_t *) (ant + 1);
  ant->sstk = ant->estk = (antval_t *) ant->ssym;
  return ant;
}
