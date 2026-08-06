# MM_compiler

# Name : Fateha Jannat Sumaya 
# ID : 231-115-215


A 6-phase compiler front-end project with **four independent language frontends** —
**C, C++, Java, and Python** — each implementing the full classical compiler pipeline:

```
Phase 1: Lexical Analysis   (Flex)
Phase 2: Syntax Analysis    (Bison, builds the AST)
Phase 3: AST Construction   (printed as an indented tree)
Phase 4: Symbol Table       (name -> type, line of declaration)
Phase 5: Semantic Analysis  (type checking, scope/redeclaration checks)
Phase 6: Intermediate Code  (Three-Address Code / TAC generation)
```

Each language has its **own lexer and parser** written to accept that language's
authentic surface syntax, so lexical analysis is genuinely different per language
(braces vs. Python indentation, `print` vs `cout <<` vs `System.out.println` vs
`print()`). Phases 3–6 (AST, symbol table, semantic analysis, TAC codegen) share
one battle-tested implementation, copied into each language's directory so every
frontend is a fully self-contained, independently buildable compiler — the same
way real compilers share a common IR across different frontends.

## Structure

```
MM_compiler/
├── c/          C frontend       (mm_c)
├── cpp/        C++ frontend     (mm_cpp)
├── java/       Java frontend    (mm_java)
├── python/     Python frontend  (mm_python)
└── docs/       this file + notes
```

Each language folder is identical in structure:

```
<lang>/
├── src/
│   ├── lexer/lexer.l         Phase 1 — Flex rules
│   ├── parser/parser.y       Phase 2 — Bison grammar (builds AST)
│   ├── ast/ast.c, ast.h      Phase 3 — AST node types + printer
│   ├── symbol_table/         Phase 4 — symbol table
│   ├── semantic/             Phase 5 — type checking
│   ├── codegen/              Phase 6 — TAC generator
│   └── main.c                 driver: runs all 6 phases in order
├── examples/                  sample source programs
├── tests/                     valid + error test cases
└── Makefile
```

## Build & run

Each frontend builds independently:

```bash
cd c && make          # produces ./c/mm_c
cd cpp && make         # produces ./cpp/mm_cpp
cd java && make        # produces ./java/mm_java
cd python && make      # produces ./python/mm_python
```

Run on any source file:

```bash
./mm_c examples/ex_ifelse.mmc
./mm_c examples/ex_while.mmc
./mm_c examples/ex_for.mmc
./mm_c examples/ex_expr.mmc


mingw32-make clean
mingw32-make
.\mm_cpp.exe examples\demo.mmcpp
cd ..\java
mingw32-make clean
mingw32-make
.\mm_java.exe examples\demo.mmjava


./mm_java examples/ex_ifelse.mmjava
./mm_java examples/ex_while.mmjava
./mm_java examples/ex_for.mmjava
./mm_java examples/ex_expr.mmjava

./mm_python examples/ex_ifelse.mmpy
./mm_python examples/ex_while.mmpy
./mm_python examples/ex_for.mmpy
./mm_python examples/ex_expr.mmpy
```

Each run prints all 6 phases in order: lex+parse status, the AST, the symbol
table, semantic-analysis result, and the generated TAC. If semantic analysis
fails, code generation is skipped and the tool exits with status 1.

`make test` runs the bundled demo. `make valgrind` runs it under Valgrind
(`--leak-check=full`).

## Verification

All 4 frontends × their full test suites (25 test files total: valid programs,
undeclared-variable, type-mismatch, redeclaration, syntax-error, lexical-error,
and — for Python — an inconsistent-indentation case) were run under
`valgrind --leak-check=full --error-exitcode=99` end-to-end:

```
TOTAL=25 FAILED=0
```

Zero leaks, zero invalid reads/writes, zero double-frees across every language
and every test case (valid and error paths alike).

## Per-language syntax notes

| | C | C++ | Java | Python |
|---|---|---|---|---|
| Block delimiters | `{ }` | `{ }` | `{ }` (wrapped in `class Main { public static void main(String[] args) { ... } }`) | indentation (`:` + INDENT/DEDENT) |
| Statement terminator | `;` | `;` | `;` | newline |
| Print | `print expr;` | `cout << expr;` | `System.out.println(expr);` | `print(expr)` |
| Declaration | `int x = 5;` | `int x = 5;` | `int x = 5;` | `x: int = 5` (PEP 526 variable annotation) |
| Boolean literals | `true` / `false` | `true` / `false` | `true` / `false` | `True` / `False` |
| Logical ops | `&& \|\| !` | `&& \|\| !` | `&& \|\| !` | `and or not` |
| Types | `int float bool string` | same | `int float boolean String` | `int float bool str` |

All four support: variable declarations (with optional initializer),
assignment, `if`/`else`, `while`, `for`, arithmetic (`+ - * / %`), relational
(`< > <= >= == !=`) and logical operators, unary `-`/`not`, and a single
`print`-style output statement.

## Notable implementation detail: Python's indentation lexer

Python is the only frontend where Phase 1 has to do real work beyond token
matching: it tracks an indentation stack in `src/lexer/lexer.l` and manually
synthesizes `INDENT`/`DEDENT`/`NEWLINE` tokens (there is no such thing at the
character level — Python's own CPython tokenizer does the same). The
implementation:

- defers indentation comparison until the next *real* token is seen, so blank
  lines and comment-only lines never affect the indent stack;
- queues multiple tokens per Flex rule invocation (Flex can only `return` one
  token per action) via a small ring buffer, drained by a thin `yylex()`
  wrapper around the Flex-generated `raw_yylex()`;
- reports a lexical error on inconsistent indentation (a dedent that doesn't
  match any enclosing indentation level).

## Known simplifications 

- No functions/methods, arrays, or classes beyond Java's mandatory `Main`
  wrapper — the subset targets declarations, control flow, and expressions,
  which is enough to exercise all 6 phases meaningfully.
- C++'s grammar differs from C's only in the print statement (`cout <<`) and
  is otherwise the same subset; a production C++ frontend would additionally
  handle templates, references, classes, etc., which is out of scope here.
- Type coercion is limited to `int → float`; all other mismatches are
  semantic errors.
- Python's `for i in range(a, b):` is lowered directly to a C-style
  `for(i=a; i<b; i=i+1)` in the AST — no real iterator protocol.
