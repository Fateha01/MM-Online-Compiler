#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"

SymbolTable *symtab_new(void) {
    SymbolTable *t = malloc(sizeof(SymbolTable));
    t->head = NULL;
    return t;
}
int symtab_declare(SymbolTable *t, const char *name, DataType type, int line) {
    if (symtab_lookup(t, name)) return 0; /* already declared */
    Symbol *s = malloc(sizeof(Symbol));
    s->name = strdup(name); s->type = type; s->line = line;
    s->next = t->head; t->head = s;
    return 1;
}
Symbol *symtab_lookup(SymbolTable *t, const char *name) {
    for (Symbol *s = t->head; s; s = s->next)
        if (strcmp(s->name, name) == 0) return s;
    return NULL;
}
void symtab_print(SymbolTable *t) {
    printf("=== Symbol Table ===\n");
    printf("%-15s %-10s %s\n", "Name", "Type", "Declared@Line");
    for (Symbol *s = t->head; s; s = s->next)
        printf("%-15s %-10s %d\n", s->name, datatype_to_string(s->type), s->line);
}
void symtab_free(SymbolTable *t) {
    Symbol *s = t->head;
    while (s) { Symbol *n = s->next; free(s->name); free(s); s = n; }
    free(t);
}
