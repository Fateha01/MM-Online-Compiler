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
%token KW_INT KW_FLOAT KW_BOOL KW_STRING KW_IF KW_ELSE KW_WHILE KW_FOR KW_TRUE KW_FALSE
%token KW_CLASS KW_PUBLIC KW_STATIC KW_VOID KW_MAIN KW_STRING_ARR
%token SYS_OUT_PRINTLN
%token ASSIGN SEMI LBRACE RBRACE LPAREN RPAREN COMMA DOT
%token PLUS MINUS STAR SLASH PERCENT
%token LT GT LE GE EQ NE AND OR NOT

%type <node> program stmt_list stmt decl_stmt assign_stmt if_stmt while_stmt for_stmt print_stmt block expr for_init assign_stmt_noeol
%type <ival> type_spec

%right NOT UMINUS
%left OR
%left AND
%left EQ NE
%left LT GT LE GE
%left PLUS MINUS
%left STAR SLASH PERCENT

%%

program:
    KW_CLASS IDENT LBRACE KW_PUBLIC KW_STATIC KW_VOID KW_MAIN LPAREN KW_STRING_ARR IDENT RPAREN block RBRACE
        { g_program = $12; $$ = NULL; free($2); free($10); }
    ;

stmt_list:
      /* empty */          { $$ = ast_new_stmt_list(); }
    | stmt_list stmt        { $$ = ast_append_stmt($1, $2); }
    ;

stmt:
      decl_stmt   { $$ = $1; }
    | assign_stmt { $$ = $1; }
    | if_stmt     { $$ = $1; }
    | while_stmt  { $$ = $1; }
    | for_stmt    { $$ = $1; }
    | print_stmt  { $$ = $1; }
    | block       { $$ = $1; }
    ;

type_spec:
      KW_INT    { $$ = TYPE_INT; }
    | KW_FLOAT  { $$ = TYPE_FLOAT; }
    | KW_BOOL   { $$ = TYPE_BOOL; }
    | KW_STRING { $$ = TYPE_STRING; }
    ;

decl_stmt:
      type_spec IDENT SEMI                  { $$ = ast_new_decl($1, $2, NULL, yylineno); free($2); }
    | type_spec IDENT ASSIGN expr SEMI      { $$ = ast_new_decl($1, $2, $4, yylineno); free($2); }
    ;

assign_stmt:
    IDENT ASSIGN expr SEMI { $$ = ast_new_assign($1, $3, yylineno); free($1); }
    ;

if_stmt:
      KW_IF LPAREN expr RPAREN stmt %prec UMINUS
        { $$ = ast_new_if($3, $5, NULL, yylineno); }
    | KW_IF LPAREN expr RPAREN stmt KW_ELSE stmt
        { $$ = ast_new_if($3, $5, $7, yylineno); }
    ;

while_stmt:
    KW_WHILE LPAREN expr RPAREN stmt { $$ = ast_new_while($3, $5, yylineno); }
    ;

for_init:
      decl_stmt { $$ = $1; }
    | assign_stmt { $$ = $1; }
    ;

for_stmt:
    KW_FOR LPAREN for_init expr SEMI assign_stmt_noeol RPAREN stmt
        { $$ = ast_new_for($3, $4, $6, $8, yylineno); }
    ;

assign_stmt_noeol:
    IDENT ASSIGN expr { $$ = ast_new_assign($1, $3, yylineno); free($1); }
    ;

print_stmt:
    SYS_OUT_PRINTLN LPAREN expr RPAREN SEMI { $$ = ast_new_print($3, yylineno); }
    ;

block:
    LBRACE stmt_list RBRACE { $$ = ast_new_block($2); }
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
    | expr AND expr    { $$ = ast_new_binop(OP_AND, $1, $3, yylineno); }
    | expr OR expr     { $$ = ast_new_binop(OP_OR, $1, $3, yylineno); }
    | NOT expr         { $$ = ast_new_unop(OP_NOT, $2, yylineno); }
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
