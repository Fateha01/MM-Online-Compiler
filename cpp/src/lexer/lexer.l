%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"

int g_trace_lex = 0;

static const char *token_name(int tok) {
    switch (tok) {
        case INT_LIT: return "INT_LIT";
        case FLOAT_LIT: return "FLOAT_LIT";
        case IDENT: return "IDENT";
        case STRING_LIT: return "STRING_LIT";
        case KW_INT: return "KW_INT";
        case KW_FLOAT: return "KW_FLOAT";
        case KW_BOOL: return "KW_BOOL";
        case KW_STRING: return "KW_STRING";
        case KW_IF: return "KW_IF";
        case KW_ELSE: return "KW_ELSE";
        case KW_WHILE: return "KW_WHILE";
        case KW_FOR: return "KW_FOR";
        case COUT: return "COUT";
        case KW_TRUE: return "KW_TRUE";
        case KW_FALSE: return "KW_FALSE";
        case ASSIGN: return "ASSIGN";
        case SEMI: return "SEMI";
        case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case COMMA: return "COMMA";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case STAR: return "STAR";
        case SLASH: return "SLASH";
        case PERCENT: return "PERCENT";
        case LT: return "LT";
        case GT: return "GT";
        case LE: return "LE";
        case GE: return "GE";
        case EQ: return "EQ";
        case NE: return "NE";
        case AND: return "AND";
        case OR: return "OR";
        case NOT: return "NOT";
        case SHL: return "SHL";
        default: return "UNKNOWN";
    }
}

#define YY_DECL int raw_yylex(void)
%}
%option yylineno noyywrap

%%
"//".*                          { /* line comment */ }
"/*"([^*]|\*+[^*/])*\*+"/"       { /* block comment */ }
[ \t\r]+                        { /* skip whitespace */ }
\n                               { /* newline, yylineno auto-incremented */ }

"int"                            { return KW_INT; }
"float"                          { return KW_FLOAT; }
"bool"                           { return KW_BOOL; }
"string"                         { return KW_STRING; }
"if"                             { return KW_IF; }
"else"                           { return KW_ELSE; }
"while"                          { return KW_WHILE; }
"for"                            { return KW_FOR; }
"cout"                           { return COUT; }
"true"                           { return KW_TRUE; }
"false"                          { return KW_FALSE; }

"="                              { return ASSIGN; }
";"                              { return SEMI; }
"{"                              { return LBRACE; }
"}"                              { return RBRACE; }
"("                              { return LPAREN; }
")"                              { return RPAREN; }
","                              { return COMMA; }
"+"                              { return PLUS; }
"-"                              { return MINUS; }
"*"                              { return STAR; }
"/"                              { return SLASH; }
"%"                              { return PERCENT; }
"<="                             { return LE; }
">="                             { return GE; }
"=="                             { return EQ; }
"!="                             { return NE; }
"<"                              { return LT; }
">"                              { return GT; }
"&&"                             { return AND; }
"||"                             { return OR; }
"!"                              { return NOT; }
"<<"                             { return SHL; }

[0-9]+\.[0-9]+                  { yylval.fval = atof(yytext); return FLOAT_LIT; }
[0-9]+                           { yylval.ival = atoi(yytext); return INT_LIT; }
\"([^\"\\]|\\.)*\"              { yytext[strlen(yytext)-1] = '\0'; yylval.sval = strdup(yytext+1); return STRING_LIT; }
[a-zA-Z_][a-zA-Z0-9_]*           { yylval.sval = strdup(yytext); return IDENT; }

.                                { fprintf(stderr, "Lexical Error (line %d): unexpected character '%s'\n", yylineno, yytext); }
%%

int yylex(void) {
    int tok = raw_yylex();
    if (g_trace_lex && tok != 0) {
        printf("%-16s %-14s %d\n", token_name(tok), yytext, yylineno);
    }
    return tok;
}
