%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"

#define MAX_INDENT 64
static int indent_stack[MAX_INDENT];
static int indent_top = 0;      /* indent_stack[0] = 0 always */
static int tok_queue[64];
static int tq_head = 0, tq_tail = 0;
static int line_has_content = 0;

static int indent_dirty = 0;
static int pending_col = 0;

static void resolve_indent(void) {
    if (!indent_dirty) return;
    indent_dirty = 0;
    if (pending_col > indent_stack[indent_top]) {
        if (indent_top + 1 >= MAX_INDENT) {
            fprintf(stderr, "Lexical Error (line %d): indentation too deep\n", yylineno);
        } else {
            indent_stack[++indent_top] = pending_col;
            tok_queue[tq_tail++] = INDENT;
        }
    } else {
        while (indent_top > 0 && pending_col < indent_stack[indent_top]) {
            indent_top--;
            tok_queue[tq_tail++] = DEDENT;
        }
        if (indent_stack[indent_top] != pending_col) {
            fprintf(stderr, "Lexical Error (line %d): inconsistent indentation\n", yylineno);
        }
    }
}

#define YY_DECL int raw_yylex(void)

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
        case KW_STR: return "KW_STR";
        case KW_IF: return "KW_IF";
        case KW_ELSE: return "KW_ELSE";
        case KW_WHILE: return "KW_WHILE";
        case KW_FOR: return "KW_FOR";
        case KW_IN: return "KW_IN";
        case KW_RANGE: return "KW_RANGE";
        case KW_PRINT: return "KW_PRINT";
        case KW_TRUE: return "KW_TRUE";
        case KW_FALSE: return "KW_FALSE";
        case KW_AND: return "KW_AND";
        case KW_OR: return "KW_OR";
        case KW_NOT: return "KW_NOT";
        case ASSIGN: return "ASSIGN";
        case COLON: return "COLON";
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
        case NEWLINE: return "NEWLINE";
        case INDENT: return "INDENT";
        case DEDENT: return "DEDENT";
        default: return "UNKNOWN";
    }
}
%}
%option noyywrap
%option yylineno

%%

\n[ \t]* {
    if (line_has_content) { tok_queue[tq_tail++] = NEWLINE; line_has_content = 0; }
    pending_col = yyleng - 1;
    indent_dirty = 1;
    if (tq_head < tq_tail) return tok_queue[tq_head++];
}

<<EOF>> {
    if (line_has_content) { tok_queue[tq_tail++] = NEWLINE; line_has_content = 0; }
    indent_dirty = 0;
    while (indent_top > 0) { indent_top--; tok_queue[tq_tail++] = DEDENT; }
    if (tq_head < tq_tail) return tok_queue[tq_head++];
    return 0;
}

"#"[^\n]*        { /* comment */ }
[ \t]+           { /* intra-line whitespace */ }

"if"             { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_IF; return tok_queue[tq_head++]; }
"else"           { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_ELSE; return tok_queue[tq_head++]; }
"while"          { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_WHILE; return tok_queue[tq_head++]; }
"for"            { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_FOR; return tok_queue[tq_head++]; }
"in"             { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_IN; return tok_queue[tq_head++]; }
"range"          { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_RANGE; return tok_queue[tq_head++]; }
"print"          { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_PRINT; return tok_queue[tq_head++]; }
"and"            { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_AND; return tok_queue[tq_head++]; }
"or"             { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_OR; return tok_queue[tq_head++]; }
"not"            { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_NOT; return tok_queue[tq_head++]; }
"True"           { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_TRUE; return tok_queue[tq_head++]; }
"False"          { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_FALSE; return tok_queue[tq_head++]; }
"int"            { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_INT; return tok_queue[tq_head++]; }
"float"          { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_FLOAT; return tok_queue[tq_head++]; }
"bool"           { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_BOOL; return tok_queue[tq_head++]; }
"str"            { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=KW_STR; return tok_queue[tq_head++]; }

":"              { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=COLON; return tok_queue[tq_head++]; }
"="              { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=ASSIGN; return tok_queue[tq_head++]; }
"("              { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=LPAREN; return tok_queue[tq_head++]; }
")"              { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=RPAREN; return tok_queue[tq_head++]; }
","              { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=COMMA; return tok_queue[tq_head++]; }
"+"              { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=PLUS; return tok_queue[tq_head++]; }
"-"              { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=MINUS; return tok_queue[tq_head++]; }
"*"              { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=STAR; return tok_queue[tq_head++]; }
"/"              { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=SLASH; return tok_queue[tq_head++]; }
"%"              { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=PERCENT; return tok_queue[tq_head++]; }
"<="             { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=LE; return tok_queue[tq_head++]; }
">="             { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=GE; return tok_queue[tq_head++]; }
"=="             { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=EQ; return tok_queue[tq_head++]; }
"!="             { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=NE; return tok_queue[tq_head++]; }
"<"              { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=LT; return tok_queue[tq_head++]; }
">"              { line_has_content=1; resolve_indent(); tok_queue[tq_tail++]=GT; return tok_queue[tq_head++]; }

[0-9]+\.[0-9]+   { line_has_content=1; resolve_indent(); yylval.fval=atof(yytext); tok_queue[tq_tail++]=FLOAT_LIT; return tok_queue[tq_head++]; }
[0-9]+            { line_has_content=1; resolve_indent(); yylval.ival=atoi(yytext); tok_queue[tq_tail++]=INT_LIT; return tok_queue[tq_head++]; }
\"([^\"\\]|\\.)*\" { line_has_content=1; resolve_indent(); yytext[strlen(yytext)-1]='\0'; yylval.sval=strdup(yytext+1); tok_queue[tq_tail++]=STRING_LIT; return tok_queue[tq_head++]; }
[a-zA-Z_][a-zA-Z0-9_]* { line_has_content=1; resolve_indent(); yylval.sval=strdup(yytext); tok_queue[tq_tail++]=IDENT; return tok_queue[tq_head++]; }

.                { fprintf(stderr, "Lexical Error (line %d): unexpected character '%s'\n", yylineno, yytext); }
%%

int yylex(void) {
    int tok;
    if (tq_head < tq_tail) tok = tok_queue[tq_head++];
    else { tq_head = tq_tail = 0; tok = raw_yylex(); }
    if (g_trace_lex && tok != 0) {
        const char *lex = yytext;
        if (tok == NEWLINE) lex = "\\n";
        else if (tok == INDENT) lex = "<indent>";
        else if (tok == DEDENT) lex = "<dedent>";
        printf("%-16s %-14s %d\n", token_name(tok), lex, yylineno);
    }
    return tok;
}
