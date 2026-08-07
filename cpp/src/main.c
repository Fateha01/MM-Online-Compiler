#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symbol_table.h"
#include "semantic.h"
#include "codegen.h"
#include "optimizer.h"
#include "targetcode.h"

extern FILE *yyin;
extern int yyparse(void);
extern ASTNode *g_program;
extern int g_trace_lex;

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source-file> [--ast] [--symtab]\n", argv[0]);
        return 1;
    }
    yyin = fopen(argv[1], "r");
    if (!yyin) { perror("fopen"); return 1; }

    int show_ast = 0, show_symtab = 0;
    for (int i = 2; i < argc; i++) {
        if (!strcmp(argv[i], "--ast")) show_ast = 1;
        if (!strcmp(argv[i], "--symtab")) show_symtab = 1;
    }
    (void)show_ast; (void)show_symtab;

    /* ---------------- Phase 1: Lexical Analysis ---------------- */
    printf("=== Phase 1: Lexical Analysis ===\n");
    printf("%-16s %-14s %s\n", "TOKEN", "LEXEME", "LINE");
    printf("------------------------------------------\n");
    g_trace_lex = 1;
    int parse_status = yyparse();
    g_trace_lex = 0;
    printf("\n");

    /* ---------------- Phase 2: Syntax Analysis ---------------- */
    if (parse_status != 0) {
        fprintf(stderr, "=== Phase 2: Syntax Analysis ===\nParsing failed.\n");
        fclose(yyin);
        return 1;
    }
    printf("=== Phase 2: Syntax Analysis ===\n");
    printf("Parse successful. Parse tree constructed.\n\n");
    printf("-- Parse Tree --\n");
    ast_print_tree(g_program, 0);
  
    printf("\n");

    /* ---------------- Phase 3: Semantic Analysis ---------------- */
    printf("=== Phase 3: Semantic Analysis ===\n");
    printf("-- Symbol Table --\n");
    SymbolTable *table = symtab_new();
    int ok = semantic_analyze(g_program, table);
    symtab_print(table);
    printf("\n");
    if (ok) {
        printf("No semantic errors found. All type checks passed.\n\n");
        printf("-- Type-Annotated Parse Tree --\n");
        ast_print_tree(g_program, 1);
        printf("\n");
    } else {
        fprintf(stderr, "Semantic analysis failed (see errors above). Aborting before code generation.\n");
        symtab_free(table);
        ast_free(g_program);
        fclose(yyin);
        return 1;
    }

    /* ---------------- Phase 4: Intermediate Code Generator ---------------- */
    printf("=== Phase 4: Intermediate Code Generator ===\n");
    int tac_count = 0;
    char **tac = codegen_generate(g_program, &tac_count);
    for (int i = 0; i < tac_count; i++) printf("%s\n", tac[i]);
    printf("\n");

    /* ---------------- Phase 5: Code Optimization ---------------- */
    printf("=== Phase 5: Code Optimization ===\n");
    int opt_count = 0, removed = 0;
    char **opt = optimize_tac(tac, tac_count, &opt_count, &removed);
    printf("(copy propagation + dead-temporary elimination: %d instruction(s) removed)\n", removed);
    for (int i = 0; i < opt_count; i++) printf("%s\n", opt[i]);
    printf("\n");

    /* ---------------- Phase 6: Target Code Generation ---------------- */
    printf("=== Phase 6: Target Code Generation ===\n");
    generate_target_code(opt, opt_count);
    printf("\n");

    codegen_free_lines(tac, tac_count);
    codegen_free_lines(opt, opt_count);
    symtab_free(table);
    ast_free(g_program);
    fclose(yyin);
    return 0;
}
