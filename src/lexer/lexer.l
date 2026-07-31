/* ============================================================
 * lexer.l -- Lexical Analyzer 
 *
 * Recognizes: keywords, identifiers, int/float/bool literals,
 * operators, delimiters; discards whitespace and comments;
 * reports invalid tokens as lexical errors with line numbers.
 * ============================================================ */

%{
#include "../parser/parser.tab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int syntax_error_count;
%}

%option yylineno
%option noyywrap

%{
/* Populate Bison's yylloc (used for accurate @N.first_line line numbers
 * in parser.y, e.g. for if/while statements) for every token matched. */
#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;
%}

DIGIT   [0-9]
LETTER  [a-zA-Z_]

%%

"int"    { return INT_TYPE; }
"float"  { return FLOAT_TYPE; }
"bool"   { return BOOL_TYPE; }
"if"     { return IF; }
"else"   { return ELSE; }
"while"  { return WHILE; }
"print"  { return PRINT; }
"true"   { return TRUE_TOK; }
"false"  { return FALSE_TOK; }

"<="     { return LE; }
">="     { return GE; }
"=="     { return EQ; }
"!="     { return NE; }
"&&"     { return AND; }
"||"     { return OR; }

[-+*/%<>(){};=!]   { return yytext[0]; }

{DIGIT}+"."{DIGIT}+   { yylval.fval = atof(yytext); return FLOAT_LITERAL; }
{DIGIT}+              { yylval.ival = atoi(yytext); return INT_LITERAL; }

{LETTER}({LETTER}|{DIGIT})*   { yylval.sval = strdup(yytext); return IDENTIFIER; }

"//".*                        { /* single-line comment: discard */ }
"/*"([^*]|\*+[^*/])*\*+"/"    { /* block comment: discard */ }

[ \t\r\n]+   { /* whitespace: discard */ }

.   {
        fprintf(stderr, "Lexical Error at line %d: unexpected character '%s'\n", yylineno, yytext);
        syntax_error_count++;
    }

%%

