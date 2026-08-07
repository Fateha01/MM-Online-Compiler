#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include "ast.h"
typedef struct Symbol {
    char *name;
    DataType type;
    int line;
    struct Symbol *next;
} Symbol;
typedef struct SymbolTable {
    Symbol *head;
} SymbolTable;
SymbolTable *symtab_new(void);
int symtab_declare(SymbolTable *t, const char *name, DataType type, int line);
Symbol *symtab_lookup(SymbolTable *t, const char *name);
void symtab_print(SymbolTable *t);
void symtab_free(SymbolTable *t);
#endif
