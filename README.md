

# MM_Compiler

### Compiler Design Lab Project

**Developed by:** Fateha Jannat Sumaya
**Student ID:** 231-115-215

---

## About This Project

During our Compiler Design course, we learned that every compiler processes a program through several phases before producing output. To understand these phases practically, I developed **MM_Compiler**.

Instead of building only one compiler, I implemented **four different language frontends**:

* C
* C++
* Java
* Python

Each frontend accepts the syntax of its own language while following the same compiler workflow internally.

---

## What This Compiler Can Do

The compiler performs the following tasks one by one:

* Reads the source program
* Breaks the source code into tokens
* Checks whether the program follows the grammar
* Builds an Abstract Syntax Tree (AST)
* Creates a Symbol Table
* Detects semantic errors
* Generates Three Address Code (TAC)

---

## Compiler Workflow

```text
Source Program
      │
      ▼
Lexical Analysis
      │
      ▼
Syntax Analysis
      │
      ▼
Abstract Syntax Tree
      │
      ▼
Symbol Table
      │
      ▼
Semantic Analysis
      │
      ▼
Three Address Code (TAC)
```

---

## Folder Layout

```text
MM_compiler
│
├── c
├── cpp
├── java
├── python
└── docs
```

Inside every language folder:

```text
src/
├── lexer
├── parser
├── ast
├── symbol_table
├── semantic
├── codegen
└── main.c
```

---

## Language Differences

Although all four compilers perform the same six phases, each language has its own syntax.

For example:

* C, C++, and Java use braces (`{}`) to define blocks.
* Python uses indentation.
* C++ prints using `cout <<`.
* Java uses `System.out.println()`.
* Python uses `print()`.

Because of these differences, every frontend has its own lexer and parser.

---

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
./cpp/mm_cpp cpp/examples/demo.mmcpp
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

## Error Checking

The compiler can detect several common programming errors, including:

* Invalid tokens
* Syntax errors
* Undeclared variables
* Variable redeclaration
* Type mismatch
* Invalid Python indentation

If a semantic error is found, TAC generation is skipped.

---

## Python Frontend

Among the four frontends, Python required the most different lexer because Python uses indentation instead of braces.

The lexer keeps track of indentation levels and generates:

* INDENT
* DEDENT
* NEWLINE

This allows Python programs to be parsed correctly.

---

## Current Scope

This project focuses on the core concepts of compiler construction.

Supported features include:

* Variable declaration
* Assignment
* Arithmetic expressions
* Relational expressions
* Logical expressions
* if-else
* while loop
* for loop
* print statement

The project does not include advanced language features such as functions, arrays, objects, templates, or exception handling.

---

## Final Remarks

The main goal of this project was not to build a complete compiler, but to understand how different programming languages pass through the same compilation process.

Working on separate frontends for C, C++, Java, and Python helped me compare their syntax while using a common compilation pipeline for AST construction, semantic analysis, symbol table generation, and intermediate code generation.


## Error Checking

The compiler can detect several common programming errors, including:

* Invalid tokens
* Syntax errors
* Undeclared variables
* Variable redeclaration
* Type mismatch
* Invalid Python indentation

If a semantic error is found, TAC generation is skipped.

---

## Python Frontend

Among the four frontends, Python required the most different lexer because Python uses indentation instead of braces.

The lexer keeps track of indentation levels and generates:

* INDENT
* DEDENT
* NEWLINE

This allows Python programs to be parsed correctly.

---

## Current Scope

This project focuses on the core concepts of compiler construction.

Supported features include:

* Variable declaration
* Assignment
* Arithmetic expressions
* Relational expressions
* Logical expressions
* if-else
* while loop
* for loop
* print statement

The project does not include advanced language features such as functions, arrays, objects, templates, or exception handling.

---

## Final Remarks

The main goal of this project was not to build a complete compiler, but to understand how different programming languages pass through the same compilation process.

Working on separate frontends for C, C++, Java, and Python helped me compare their syntax while using a common compilation pipeline for AST construction, semantic analysis, symbol table generation, and intermediate code generation.
