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
        case KW_TRUE: return "KW_TRUE";
        case KW_FALSE: return "KW_FALSE";
        case KW_CLASS: return "KW_CLASS";
        case KW_PUBLIC: return "KW_PUBLIC";
        case KW_STATIC: return "KW_STATIC";
        case KW_VOID: return "KW_VOID";
        case KW_MAIN: return "KW_MAIN";
        case KW_STRING_ARR: return "KW_STRING_ARR";
        case SYS_OUT_PRINTLN: return "SYS_OUT_PRINTLN";
        case ASSIGN: return "ASSIGN";
        case SEMI: return "SEMI";
        case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case COMMA: return "COMMA";
        case DOT: return "DOT";
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
\n                               { /* newline */ }

"class"                          { return KW_CLASS; }
"public"                         { return KW_PUBLIC; }
"static"                         { return KW_STATIC; }
"void"                           { return KW_VOID; }
"main"                           { return KW_MAIN; }
"String[]"                       { return KW_STRING_ARR; }
"System.out.println"            { return SYS_OUT_PRINTLN; }

"int"                            { return KW_INT; }
"float"                          { return KW_FLOAT; }
"boolean"                        { return KW_BOOL; }
"String"                         { return KW_STRING; }
"if"                             { return KW_IF; }
"else"                           { return KW_ELSE; }
"while"                          { return KW_WHILE; }
"for"                            { return KW_FOR; }
"true"                           { return KW_TRUE; }
"false"                          { return KW_FALSE; }

"="                              { return ASSIGN; }
";"                              { return SEMI; }
"{"                              { return LBRACE; }
"}"                              { return RBRACE; }
"("                              { return LPAREN; }
")"                              { return RPAREN; }
","                              { return COMMA; }
"."                              { return DOT; }
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
