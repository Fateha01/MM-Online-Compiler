%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
extern int yylineno;
int yylex(void);
void yyerror(const char *s);
extern FILE *yyin;
ASTNode *g_program = NULL;
%}

%union {
    int ival;
    double fval;
    char *sval;
    struct ASTNode *node;
}

%destructor { free($$); } <sval>
%destructor { ast_free($$); } <node>

%token <ival> INT_LIT
%token <fval> FLOAT_LIT
%token <sval> IDENT STRING_LIT
%token KW_INT KW_FLOAT KW_BOOL KW_STR KW_IF KW_ELSE KW_WHILE KW_FOR KW_IN KW_RANGE KW_PRINT KW_TRUE KW_FALSE
%token KW_AND KW_OR KW_NOT
%token ASSIGN COLON LPAREN RPAREN COMMA
%token PLUS MINUS STAR SLASH PERCENT
%token LT GT LE GE EQ NE
%token NEWLINE INDENT DEDENT

%type <node> program stmt_list stmt decl_stmt assign_stmt if_stmt while_stmt for_stmt print_stmt expr
%type <ival> type_spec

%right UMINUS
%left KW_OR
%left KW_AND
%right KW_NOT
%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left STAR SLASH PERCENT

%%

program:
    stmt_list { g_program = $1; $$ = NULL; }
    ;

stmt_list:
      /* empty */          { $$ = ast_new_stmt_list(); }
    | stmt_list stmt        { $$ = ast_append_stmt($1, $2); }
    ;

stmt:
      decl_stmt NEWLINE  { $$ = $1; }
    | assign_stmt NEWLINE { $$ = $1; }
    | print_stmt NEWLINE  { $$ = $1; }
    | if_stmt              { $$ = $1; }
    | while_stmt            { $$ = $1; }
    | for_stmt               { $$ = $1; }
    ;

type_spec:
      KW_INT    { $$ = TYPE_INT; }
    | KW_FLOAT  { $$ = TYPE_FLOAT; }
    | KW_BOOL   { $$ = TYPE_BOOL; }
    | KW_STR    { $$ = TYPE_STRING; }
    ;

decl_stmt:
      IDENT COLON type_spec               { $$ = ast_new_decl($3, $1, NULL, yylineno); free($1); }
    | IDENT COLON type_spec ASSIGN expr   { $$ = ast_new_decl($3, $1, $5, yylineno); free($1); }
    ;

assign_stmt:
    IDENT ASSIGN expr { $$ = ast_new_assign($1, $3, yylineno); free($1); }
    ;

print_stmt:
    KW_PRINT LPAREN expr RPAREN { $$ = ast_new_print($3, yylineno); }
    ;

if_stmt:
      KW_IF expr COLON NEWLINE INDENT stmt_list DEDENT
        { $$ = ast_new_if($2, ast_new_block($6), NULL, yylineno); }
    | KW_IF expr COLON NEWLINE INDENT stmt_list DEDENT KW_ELSE COLON NEWLINE INDENT stmt_list DEDENT
        { $$ = ast_new_if($2, ast_new_block($6), ast_new_block($12), yylineno); }
    ;

while_stmt:
    KW_WHILE expr COLON NEWLINE INDENT stmt_list DEDENT
        { $$ = ast_new_while($2, ast_new_block($6), yylineno); }
    ;

for_stmt:
    KW_FOR IDENT KW_IN KW_RANGE LPAREN expr COMMA expr RPAREN COLON NEWLINE INDENT stmt_list DEDENT
        {
            ASTNode *init = ast_new_decl(TYPE_INT, $2, $6, yylineno);
            ASTNode *cond = ast_new_binop(OP_LT, ast_new_ident($2, yylineno), $8, yylineno);
            ASTNode *step = ast_new_assign($2,
                ast_new_binop(OP_ADD, ast_new_ident($2, yylineno), ast_new_int_literal(1, yylineno), yylineno),
                yylineno);
            $$ = ast_new_for(init, cond, step, ast_new_block($13), yylineno);
            free($2);
        }
    ;

expr:
      expr PLUS expr   { $$ = ast_new_binop(OP_ADD, $1, $3, yylineno); }
    | expr MINUS expr  { $$ = ast_new_binop(OP_SUB, $1, $3, yylineno); }
    | expr STAR expr   { $$ = ast_new_binop(OP_MUL, $1, $3, yylineno); }
    | expr SLASH expr  { $$ = ast_new_binop(OP_DIV, $1, $3, yylineno); }
    | expr PERCENT expr{ $$ = ast_new_binop(OP_MOD, $1, $3, yylineno); }
    | expr LT expr     { $$ = ast_new_binop(OP_LT, $1, $3, yylineno); }
    | expr GT expr     { $$ = ast_new_binop(OP_GT, $1, $3, yylineno); }
    | expr LE expr     { $$ = ast_new_binop(OP_LE, $1, $3, yylineno); }
    | expr GE expr     { $$ = ast_new_binop(OP_GE, $1, $3, yylineno); }
    | expr EQ expr     { $$ = ast_new_binop(OP_EQ, $1, $3, yylineno); }
    | expr NE expr     { $$ = ast_new_binop(OP_NE, $1, $3, yylineno); }
    | expr KW_AND expr { $$ = ast_new_binop(OP_AND, $1, $3, yylineno); }
    | expr KW_OR expr  { $$ = ast_new_binop(OP_OR, $1, $3, yylineno); }
    | KW_NOT expr      { $$ = ast_new_unop(OP_NOT, $2, yylineno); }
    | MINUS expr %prec UMINUS { $$ = ast_new_unop(OP_NEG, $2, yylineno); }
    | LPAREN expr RPAREN { $$ = $2; }
    | IDENT            { $$ = ast_new_ident($1, yylineno); free($1); }
    | INT_LIT          { $$ = ast_new_int_literal($1, yylineno); }
    | FLOAT_LIT         { $$ = ast_new_float_literal($1, yylineno); }
    | STRING_LIT        { $$ = ast_new_string_literal($1, yylineno); free($1); }
    | KW_TRUE           { $$ = ast_new_bool_literal(1, yylineno); }
    | KW_FALSE          { $$ = ast_new_bool_literal(0, yylineno); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax Error (line %d): %s\n", yylineno, s);
}
