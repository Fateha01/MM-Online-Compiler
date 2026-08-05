#ifndef CODEGEN_H
#define CODEGEN_H
#include "ast.h"

/* Phase 4: Intermediate Code Generator.
   Generates Three-Address Code (TAC) into an in-memory line array
   (does NOT print anything). Caller owns the returned array;
   free it with codegen_free_lines(). *out_count receives the
   number of lines produced. */
char **codegen_generate(ASTNode *program, int *out_count);
void codegen_free_lines(char **lines, int count);

#endif
