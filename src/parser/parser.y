/* ============================================================
 * parser.y -- Syntax Analyzer  + AST construction
 *
 * Implements the CFG for the language defined in Section 5 of
 * the manual. Operator precedence/associativity is declared
 * explicitly below rather than encoded in extra grammar layers,
 * which keeps the grammar short and is the idiomatic Bison way
 * to resolve arithmetic/logical expression ambiguity.
 *
 * No shift/reduce conflicts: `bison -v` reports zero for this grammar.
 * The classic "dangling else" ambiguity (if(a) if(b) s1 else s2) does
 * NOT arise here because if/while bodies must be a brace-delimited
 * `block`, not a bare statement -- so there is only one way to write
 * a nested if-else. See docs/report.md (Parser Design section).
 * ============================================================ */

%code requires {
#include "../ast/ast.h"
}

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../ast/ast.h"

int yylex(void);
void yyerror(const char *s);
extern int yylineno;

ASTNode *ast_root = NULL;
int syntax_error_count = 0;
%}

%union {
    int ival;
    double fval;
    char *sval;
    ASTNode *node;
}

%define parse.error verbose
%locations

%token <ival> INT_LITERAL
%token <fval> FLOAT_LITERAL
%token <sval> IDENTIFIER
%destructor { free($$); } IDENTIFIER
/* Scoped to `expr` specifically (NOT a blanket `<node>` type destructor).
 * A blanket <node> destructor also matches `program` -- and Bison's
 * generated accept path has a documented quirk where the "don't reclaim
 * the result of the rule that triggered YYACCEPT" protection can miss
 * the start symbol when the fast $end-shift-to-YYFINAL path is taken,
 * so it ends up destroying `ast_root` itself right after a successful
 * parse (verified with gdb: crash inside semantic_analyze on a freed
 * tree). `expr` is the only nonterminal actually at risk of being
 * stranded on the stack by our `stmt_list: stmt_list error ';'`
 * recovery rule (e.g. the value side of an assignment that never got
 * used because the statement was missing its ';'), so that's the only
 * one that needs a destructor. */
%destructor { ast_free($$); } expr
%token INT_TYPE FLOAT_TYPE BOOL_TYPE
%token IF ELSE WHILE PRINT TRUE_TOK FALSE_TOK
%token LE GE EQ NE AND OR

%type <node> program stmt_list stmt decl_stmt assign_stmt if_stmt while_stmt print_stmt block expr
%type <ival> type_spec

%right '='
%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%right '!' UMINUS

%%

program:
      stmt_list { ast_root = $1; $$ = $1; }
    ;

stmt_list:
      stmt_list stmt      { $$ = ast_append_stmt($1, $2); }
    | stmt_list error ';' { yyerrok; $$ = $1; }
    | /* empty */         { $$ = ast_new_stmt_list(); }
    ;

stmt:
      decl_stmt
    | assign_stmt
    | if_stmt
    | while_stmt
    | print_stmt
    | block
    ;

decl_stmt:
    type_spec IDENTIFIER ';' { $$ = ast_new_decl($1, $2, yylineno); }
    ;

type_spec:
      INT_TYPE   { $$ = TYPE_INT; }
    | FLOAT_TYPE { $$ = TYPE_FLOAT; }
    | BOOL_TYPE  { $$ = TYPE_BOOL; }
    ;

assign_stmt:
    IDENTIFIER '=' expr ';' { $$ = ast_new_assign($1, $3, yylineno); }
    ;

/* NOTE on line numbers: plain yylineno reflects the lexer's position at
 * the moment an action runs. Because Bison performs default reductions
 * without pulling a fresh lookahead token, that moment can lag behind
 * for short rules like "type IDENTIFIER ';'" -- but never across a line
 * boundary, so plain yylineno is fine there. if_stmt/while_stmt are
 * different: they swallow an entire nested block before their action
 * fires, so by then yylineno has drifted to wherever the block ended.
 * We use Bison's %locations tracking (see @1 below) instead, which
 * records the line of the IF/WHILE token itself without needing a
 * mid-rule action. (A mid-rule action was tried first and rejected: it
 * splits the two if_stmt alternatives into different intermediate
 * states before Bison can tell them apart, which *introduces* a new
 * shift/reduce conflict -- confirmed by a "rule useless in parser"
 * warning during testing -- even though the plain grammar below has
 * none.) */
if_stmt:
      IF '(' expr ')' block ELSE block { $$ = ast_new_if($3, $5, $7, @1.first_line); }
    | IF '(' expr ')' block            { $$ = ast_new_if($3, $5, NULL, @1.first_line); }
    ;

while_stmt:
    WHILE '(' expr ')' block { $$ = ast_new_while($3, $5, @1.first_line); }
    ;

print_stmt:
    PRINT expr ';' { $$ = ast_new_print($2, yylineno); }
    ;

block:
    '{' stmt_list '}' { $$ = ast_new_block($2); }
    ;

expr:
      expr OR expr   { $$ = ast_new_binop(OP_OR,  $1, $3, yylineno); }
    | expr AND expr  { $$ = ast_new_binop(OP_AND, $1, $3, yylineno); }
    | expr EQ expr   { $$ = ast_new_binop(OP_EQ,  $1, $3, yylineno); }
    | expr NE expr   { $$ = ast_new_binop(OP_NE,  $1, $3, yylineno); }
    | expr '<' expr  { $$ = ast_new_binop(OP_LT,  $1, $3, yylineno); }
    | expr '>' expr  { $$ = ast_new_binop(OP_GT,  $1, $3, yylineno); }
    | expr LE expr   { $$ = ast_new_binop(OP_LE,  $1, $3, yylineno); }
    | expr GE expr   { $$ = ast_new_binop(OP_GE,  $1, $3, yylineno); }
    | expr '+' expr  { $$ = ast_new_binop(OP_ADD, $1, $3, yylineno); }
    | expr '-' expr  { $$ = ast_new_binop(OP_SUB, $1, $3, yylineno); }
    | expr '*' expr  { $$ = ast_new_binop(OP_MUL, $1, $3, yylineno); }
    | expr '/' expr  { $$ = ast_new_binop(OP_DIV, $1, $3, yylineno); }
    | expr '%' expr  { $$ = ast_new_binop(OP_MOD, $1, $3, yylineno); }
    | '!' expr               { $$ = ast_new_unop(OP_NOT, $2, yylineno); }
    | '-' expr %prec UMINUS  { $$ = ast_new_unop(OP_NEG, $2, yylineno); }
    | '(' expr ')'   { $$ = $2; }
    | IDENTIFIER     { $$ = ast_new_ident($1, yylineno); }
    | INT_LITERAL    { $$ = ast_new_int_literal($1, yylineno); }
    | FLOAT_LITERAL  { $$ = ast_new_float_literal($1, yylineno); }
    | TRUE_TOK       { $$ = ast_new_bool_literal(1, yylineno); }
    | FALSE_TOK      { $$ = ast_new_bool_literal(0, yylineno); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Syntax Error at line %d: %s\n", yylineno, s);
    syntax_error_count++;
}
