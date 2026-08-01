#ifndef CODEGEN_H
#define CODEGEN_H
#include "ast.h"
/* Generates Three-Address Code (TAC) to stdout and returns temp count used. */
void codegen_generate(ASTNode *program);
#endif
