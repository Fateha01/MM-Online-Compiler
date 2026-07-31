# MM_compiler

Compiler Construction Lab project. Front-end for a small imperative
language — lexer, parser/AST, semantic analysis, three-address code gen.
Language has int/float/bool, arithmetic/relational/logical exprs,
if/if-else, while, print, nested block scoping. Built with Flex + Bison.

Repo folder is `MM_compiler`, binary is still `mc-compiler` (didn't bother
renaming it everywhere in the Makefile etc, wasn't worth the risk this
close to submission).

Team: Fateha Jannat Sumaya, Id 231-115-215

## Requirements

Need flex, bison, gcc, make. gdb too if you're gonna use the debug config
in VS Code.

**Linux / WSL:**
```
sudo apt update
sudo apt install flex bison gcc make gdb
```

**Windows without WSL:** install MSYS2 (msys2.org, default install path is
fine). Open the MSYS2 shortcut and run `pacman -Syu` — it'll probably
close on you and ask you to run it again, just do that till it says
nothing left to update. Then open the **UCRT64** shortcut (different one!)
and run:
```
pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain flex bison
```
Check with `gcc --version` / `flex --version` / `bison --version` / `make --version`.

VS Code is already set up to use UCRT64 automatically (see `.vscode/`), no
PATH editing needed.

## Build & run

```
make
./mc-compiler tests/valid_program.mc
```

`make` runs bison on `parser.y` → `parser.tab.c/h`, flex on `lexer.l` →
`lex.yy.c`, compiles + links everything into `mc-compiler` (or
`mc-compiler.exe` on Windows, Makefile figures out which).

Running it prints parse status → semantic status → AST → TAC, in that
order. Errors (lexical/syntax/semantic) get reported with a line number
and it stops there — won't try to generate TAC if semantic analysis
failed.

`make test` runs every `.mc` file in `tests/` and you diff against the
matching `.expected.txt`.

`make clean` wipes the build dir + binary + generated flex/bison files
(these aren't committed, see .gitignore).

## VS Code

Opening the folder gives you build/run/debug already wired up:
- Ctrl+Shift+B = build
- Run Task → "Run all tests" = make test
- Run Task → "Run on current file"
- Debug panel → "Debug mc-compiler (gdb)" — asks which .mc file to run,
  defaults to valid_program.mc, set breakpoints in semantic.c/codegen.c
  and step through it

It'll probably prompt you to install the C/C++ extension on first open,
say yes to that.

On WSL open the folder from inside your Ubuntu terminal (`code .`), not
from Windows explorer, or the toolchain won't be on PATH. On MSYS2 it
should just work as long as it's installed at the default path — if you
put it somewhere else you'll need to fix the msys64 path in
`.vscode/settings.json` and `tasks.json`.

fyi `lexer.l` includes `parser.tab.h` which doesn't exist till you build
once, so you'll get a red squiggly there before the first `make`. normal,
ignore it.

I tested the Linux/WSL build properly — all test cases + valgrind + gdb,
no leaks, no issues. Didn't have a Windows machine around to test the
MSYS2 steps on so can't 100% promise those work, should be fine though
since it's just the normal MSYS2 install process.

## Demo (bonus GUI thing)

Made a little GUI too, it's in `demo/`. one page, dropdown to pick a
language. if you pick Mini Language it actually runs the real
mc-compiler binary from src/, everything else in the dropdown (C, C++,
Python, Java, JS) just calls the normal compilers for those, put it in
there mostly cause it looked cooler with more options lol. only the Mini
Language one actually matters for grading obviously.

to run it:
```
cd demo/backend
python3 server.py
```
or just double click run_demo.bat (windows) / run run_demo.sh, those
build the compiler first then start the server for you. if you run
server.py directly it skips the build step so make sure you ran `make`
at least once already

it's just a local server using python's stdlib, no installs needed,
opens your browser on its own, works offline once python's there.
needs python3 obviously (on msys2: `pacman -S mingw-w64-ucrt-x86_64-python`
if you don't have it already). the other languages only show up if you
actually have those compilers installed on your machine — not required,
whole demo thing is optional anyway.

## Structure

```
project-root/
├── .vscode/          # vs code stuff
├── docs/report.md    # the writeup
├── src/
│   ├── lexer/lexer.l
│   ├── parser/parser.y
│   ├── ast/
│   ├── semantic/
│   ├── symbol_table/
│   ├── codegen/
│   └── main.c
├── demo/             # bonus GUI, see below
├── tests/            # .mc files + expected output
├── examples/
├── Makefile
└── README.md
```

## Implemented

Lexer handles keywords, identifiers, int/float/bool literals, all the
required operators/delimiters, strips comments, reports bad characters as
lexical errors with line numbers.

Parser covers the full grammar (docs/report.md 3.3), builds the AST, has
basic panic-mode recovery for syntax errors.

AST prints as an indented tree for every construct.

Symbol table does nested/block scoping — tracks name, type, scope depth,
declared line for everything, prints the full table once a program passes
semantic analysis clean.

Semantic analysis catches all 6 required categories: undeclared variable,
redeclaration, scope violation, type mismatch, invalid assignment, invalid
expressions. Each gets its own line-numbered error message.

TAC generation covers arithmetic (correct precedence), relational/logical
expressions, if/if-else/while via labels and jumps, print.
