# Ant - a scripting engine

Ant is an ultra fast and ultra small embedded scripting engine
that targets performance, small size and embedding simplicity:

- Single C header engine with very simple API
- Interpreted directly with near-native performance

# Syntax

- Infix notation
- Arithmetics: `+`, `-`, `*`, `/`
- Single-letter variables from `a` to `z`
- Assignments `a = 0; b = 2;`
- Increments and decrements `a += 2; b += a * (c + d);`
- Labels for jumps: `#`
- Jumps: `@f expr` jumps forward, `@b expr` jumps backward if `expr` is true.
  Jumps are performed to the nearest `#` label
- Loops and conditionals are implemented using labels and jumps, e.g.
  `if (a > 0) i++; ...` -> `@tf a < 0 i += 1 # ...`

# Notes

Below are major places where a scripting engine slows down in comparison
with the native code:

1. Expression parsing. To alleviate this slowness, either
   - a postfix notation should be used, e.g. `1 2 + 3 *` instead of `(1 + 2) * 3`
   - an extremely fast infix expression parser
2. Variable lookup: `a = 123` or `a = b + c`. Compiled code assigns some
  memory locations for each variable and references that memory directly.
  A scripting engine performs a variable lookup every time a variable
  gets referenced. To speed
  up variable lookups, the following tactics can be used:
   - using only single-letter variable names, for example from `a` to `z`.
  This way, ant reserves 26 variables in total, and a variable name gives
  a direct index: `vars[*pc - 'a']`
   - using explicit variable indices, e.g. `17 v` which gives `vars[17]`.
  For example, JavaScript's `let foo = 123` can become ant's `123 0 v =`
  if a postfix notation is used, which in turn executes `vars[0] = 123`
3. Marshalling arguments to FFI calls. To alleviate this, a scripting engine
  can push arguments to machine's stack directly and therefore avoid any
  extra marshalling layer, e.g. `"world" "hello, %s\n" P` where `P` references
  stdlib's `printf`, and strings `"world"` and `"hello, %s"` push respective
  pointers to the machine's stack. This is not portable however, since
  some architectures might not use stack for arguments
4. Floating-point versus integer math
5. Conditionals. Consider JavaScript's `if (cond) { body }`, where `cond` is
  false. Then, we should jump over the `body` whilst not executing it.
  If there is no compilation step, we don't know where `body` ends and therefore
  we should "execute" the `body` without evaluating it.
  The solution is to use labels and jump commands, just like in the
  assembly code.
