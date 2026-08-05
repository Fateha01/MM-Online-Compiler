#ifndef SEMANTIC_H
#define SEMANTIC_H
#include "ast.h"
#include "symbol_table.h"
/* Returns 1 if no errors, 0 if any semantic error was found (also printed to stderr). */
int semantic_analyze(ASTNode *program, SymbolTable *table);
#endif
