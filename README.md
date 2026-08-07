# MM_compiler

# Name : Fateha Jannat Sumaya 
# ID : 231-115-215

A compiler front-end I built for my Compiler Construction lab. It has four
separate frontends — C, C++, Java, Python — and each one runs the full
compiler pipeline on its own:

1. **Lexical analysis** — Flex
2. **Syntax analysis** — Bison, builds the AST
3. **Semantic analysis** — type checking, scope/redeclaration checks (builds the symbol table)
4. **Intermediate code generation** — three-address code (TAC)
5. **Code optimization** — copy propagation + dead-temporary elimination
6. **Target code generation** — final output

Each language has its own lexer/parser, so it reads that language's real
syntax — braces vs. Python's indentation, `print` vs `cout <<` vs
`System.out.println` vs `print()`. Phases 3–6 (semantic check, TAC,
optimization, target code) share one implementation, copied into every
language folder — so each frontend still builds and runs completely by
itself.

## Layout

```
MM_compiler/
├── c/          C frontend       (mm_c)
├── cpp/        C++ frontend     (mm_cpp)
├── java/       Java frontend    (mm_java)
└── python/     Python frontend  (mm_python)
```

Every `<lang>/` folder is the same:

```
<lang>/
├── src/
│   ├── lexer/lexer.l         Phase 1
│   ├── parser/parser.y       Phase 2
│   ├── ast/                  (built in phase 2, printed in phase 3 — not a standalone phase)
│   ├── symbol_table/         (used in phase 3 — not a standalone phase)
│   ├── semantic/             Phase 3
│   ├── codegen/              Phase 4
│   ├── optimizer/            Phase 5
│   ├── targetcode/           Phase 6
│   └── main.c                 runs all 6 phases in order
├── examples/                  sample programs
├── tests/                     valid + error cases
└── Makefile
```

## Build

Linux/WSL:
```bash
cd c    && make      # ./c/mm_c
cd cpp  && make       # ./cpp/mm_cpp
cd java && make       # ./java/mm_java
cd python && make     # ./python/mm_python
```

Windows (MinGW):
```bat
mingw32-make clean
mingw32-make
```

## Run

```bash
cd c
./mm_c examples/demo.mmc
./mm_c examples/ex_expr.mmc
./mm_c examples/ex_for.mmc
./mm_c examples/ex_ifelse.mmc
./mm_c examples/ex_while.mmc

cd cpp
./mm_cpp examples/demo.mmcpp
./mm_cpp examples/ex_expr.mmcpp
./mm_cpp examples/ex_for.mmcpp
./mm_cpp examples/ex_ifelse.mmcpp
./mm_cpp examples/ex_while.mmcpp

cd java
./mm_java examples/demo.mmjava
./mm_java examples/ex_expr.mmjava
./mm_java examples/ex_for.mmjava
./mm_java examples/ex_ifelse.mmjava
./mm_java examples/ex_while.mmjava


cd python
./mm_python examples/demo.mmpy
./mm_python examples/comments_blank_lines.mmpy
./mm_python examples/ex_expr.mmpy
./mm_python examples/ex_for.mmpy
./mm_python examples/ex_ifelse.mmpy
./mm_python examples/ex_while.mmpy
```
Each run shows all six phases in order: token stream, parse tree (AST),
symbol table + semantic result, TAC, optimized TAC, then target code. If
semantic analysis fails, codegen is skipped and it exits with status 1.

`make test` runs the demo. `make valgrind` runs it under
`valgrind --leak-check=full`.

## Testing

All 4 frontends, 25 test files total (valid programs + undeclared-variable,
type-mismatch, redeclaration, syntax-error, lexical-error, and Python's
inconsistent-indentation case), checked with
`valgrind --leak-check=full --error-exitcode=99`:

```
TOTAL=25  FAILED=0
```

No leaks, no invalid reads/writes, no double-frees — valid and error cases
both.

## Syntax at a glance

| | C | C++ | Java | Python |
|---|---|---|---|---|
| Blocks | `{ }` | `{ }` | `{ }` (inside `class Main { public static void main(String[] args) { ... } }`) | indentation (`:` + INDENT/DEDENT) |
| Terminator | `;` | `;` | `;` | newline |
| Print | `print expr;` | `cout << expr;` | `System.out.println(expr);` | `print(expr)` |
| Declaration | `int x = 5;` | `int x = 5;` | `int x = 5;` | `x: int = 5` (PEP 526) |
| Booleans | `true` / `false` | `true` / `false` | `true` / `false` | `True` / `False` |
| Logical ops | `&& \|\| !` | `&& \|\| !` | `&& \|\| !` | `and or not` |
| Types | `int float bool string` | same | `int float boolean String` | `int float bool str` |

All four support: variable declarations (with or without a value),
assignment, `if`/`else`, `while`, `for`, math (`+ - * / %`), comparisons
(`< > <= >= == !=`) and logic ops, unary `-`/`not`, and one `print`
statement.

## Python's indentation lexer

Python is the only frontend where Phase 1 does real work, not just token
matching. `src/lexer/lexer.l` keeps an indentation stack and creates
`INDENT` / `DEDENT` / `NEWLINE` tokens by hand — there's no such thing at
the character level (CPython's own tokenizer does the same). How it works:

- indentation is only checked once a real token appears, so blank lines
  and comment-only lines don't affect it
- Flex can only return one token per rule, but sometimes more than one
  token needs to fire (like two DEDENTs in a row), so a small queue holds
  them and a wrapper function drains it
- a dedent that doesn't match any open indent level is reported as a
  lexical error

## What's left out :

- No functions, arrays, or classes — just Java's required `Main` wrapper.
  Declarations, control flow, and expressions are enough to cover all six
  phases.
- C++ differs from C only in the print statement (`cout <<`); a real C++
  frontend would also need templates, references, classes, etc.
- Type coercion only covers `int → float`; anything else is a semantic
  error.
- Python's `for i in range(a, b):` turns into a plain
  `for(i=a; i<b; i=i+1)` in the AST — no real iterator protocol.