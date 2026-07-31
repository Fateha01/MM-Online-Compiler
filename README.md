# MM_compiler

Compiler construction lab project. Built 4 separate frontends (C, C++, Java, Python), each one goes through the full 6 phases:

1. Lexical Analysis (Flex)
2. Syntax Analysis (Bison, builds AST)
3. AST Construction (printed as indented tree)
4. Symbol Table (name, type, line of declaration)
5. Semantic Analysis (type checking, redeclaration/scope errors)
6. Intermediate Code (Three-Address Code)

Each language has its own lexer/parser so it actually parses that language's real syntax (braces vs Python's indentation, `print` vs `cout <<` vs `System.out.println` vs `print()`). Phases 3-6 are basically the same core logic reused across all four — didn't see the point in rewriting AST/symbol-table/TAC logic 4 times when the language-specific part is really just phases 1-2.


Name : Fateha Jannat Sumaya ID: 231-115-215

## Folder structure

```
MM_compiler/
├── c/          C frontend      -> mm_c
├── cpp/        C++ frontend    -> mm_cpp
├── java/       Java frontend   -> mm_java
├── python/     Python frontend -> mm_python
└── docs/
```

Same layout inside each:

```
<lang>/
├── src/
│   ├── lexer/lexer.l
│   ├── parser/parser.y
│   ├── ast/ast.c, ast.h
│   ├── symbol_table/
│   ├── semantic/
│   ├── codegen/
│   └── main.c
├── examples/
├── tests/
└── Makefile
```

## Build & run

```bash
cd c && make
cd cpp && make
cd java && make
cd python && make
```

Run:

```bash
./c/mm_c c/examples/demo.mmc
./cpp/mm_cpp cpp/examples/demo.mmcpp
./java/mm_java java/examples/demo.mmjava
./python/mm_python python/examples/demo.mmpy
```

Each run shows lex/parse status → AST → symbol table → semantic result → TAC. If semantic analysis fails it skips codegen and exits 1.

`make test` runs the demo, `make valgrind` runs it with `--leak-check=full`.

## Testing

Ran all 4 frontends against their test suites (25 files total — valid programs, undeclared variable, type mismatch, redeclaration, syntax error, lexical error, plus one bad-indentation case for Python) under:

```
valgrind --leak-check=full --error-exitcode=99
```

Result: `TOTAL=25 FAILED=0`. No leaks, no invalid read/write, no double-free on any of it.

## Syntax differences between the 4 languages

| | C | C++ | Java | Python |
|---|---|---|---|---|
| Blocks | `{ }` | `{ }` | `{ }` (inside `class Main { public static void main(String[] args) { ... } }`) | indentation, `:` + INDENT/DEDENT |
| Statement end | `;` | `;` | `;` | newline |
| Print | `print expr;` | `cout << expr;` | `System.out.println(expr);` | `print(expr)` |
| Declaration | `int x = 5;` | `int x = 5;` | `int x = 5;` | `x: int = 5` |
| Booleans | `true`/`false` | `true`/`false` | `true`/`false` | `True`/`False` |
| Logic ops | `&& \|\| !` | `&& \|\| !` | `&& \|\| !` | `and or not` |
| Types | `int float bool string` | same | `int float boolean String` | `int float bool str` |

All 4 support: declarations (with/without init), assignment, if/else, while, for, arithmetic, relational + logical ops, unary -/not, and one print statement.

## Python indentation lexer (the annoying part)

This was the one place where phase 1 wasn't trivial. Unlike the others, Python doesn't give you block boundaries as actual characters, so the lexer has to track an indent stack manually and synthesize INDENT/DEDENT/NEWLINE tokens itself (same basic idea CPython's own tokenizer uses).

A few things I had to handle:
- indentation comparisons only happen when a real token shows up next — otherwise blank lines / comment-only lines would mess up the stack
- Flex only lets you `return` one token per rule, so I queue tokens in a small ring buffer and wrote a thin `yylex()` wrapper around the generated `raw_yylex()` to drain it
- if a dedent doesn't match any level on the stack, it's a lexical error

Took a few tries to get right, kept getting phantom DEDENT tokens at EOF before I fixed the buffer draining logic.

## What's intentionally left out

- No functions, arrays, or classes (except Java's mandatory Main wrapper) — lab scope is declarations + control flow + expressions, which is enough to hit all 6 phases
- C++ only differs from C in the print statement here — a real C++ frontend would need templates, references, classes etc, way beyond this lab
- Type coercion only does int → float, everything else is a semantic error
- Python's `for i in range(a, b):` just gets lowered to a C-style `for(i=a; i<b; i=i+1)` in the AST, no actual iterator protocol
