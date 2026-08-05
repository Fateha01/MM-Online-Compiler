#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "codegen.h"

static int temp_count = 0;
static int label_count = 0;

/* dynamic line buffer (module-local, reset each call) */
static char **g_lines = NULL;
static int g_count = 0;
static int g_cap = 0;

static void emit(const char *fmt, ...) {
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);

    if (g_count == g_cap) {
        g_cap = g_cap ? g_cap * 2 : 32;
        g_lines = realloc(g_lines, g_cap * sizeof(char *));
    }
    g_lines[g_count++] = strdup(buf);
}

static char *new_temp(void) {
    char *buf = malloc(16);
    snprintf(buf, 16, "t%d", temp_count++);
    return buf;
}
static char *new_label(void) {
    char *buf = malloc(16);
    snprintf(buf, 16, "L%d", label_count++);
    return buf;
}

/* returns a heap string holding the "place" (temp name, var name, or literal text) of the expr */
static char *gen_expr(ASTNode *node) {
    char buf[64];
    switch (node->type) {
        case NODE_INT_LITERAL: snprintf(buf, sizeof buf, "%d", node->data.int_literal.value); return strdup(buf);
        case NODE_FLOAT_LITERAL: snprintf(buf, sizeof buf, "%g", node->data.float_literal.value); return strdup(buf);
        case NODE_BOOL_LITERAL: return strdup(node->data.bool_literal.value ? "true" : "false");
        case NODE_STRING_LITERAL: snprintf(buf, sizeof buf, "\"%s\"", node->data.string_literal.value); return strdup(buf);
        case NODE_IDENT: return strdup(node->data.ident.name);
        case NODE_UNOP: {
            char *o = gen_expr(node->data.unop.operand);
            char *t = new_temp();
            emit("%s = %s %s", t, optype_to_string(node->data.unop.op), o);
            free(o);
            return t;
        }
        case NODE_BINOP: {
            char *l = gen_expr(node->data.binop.left);
            char *r = gen_expr(node->data.binop.right);
            char *t = new_temp();
            emit("%s = %s %s %s", t, l, optype_to_string(node->data.binop.op), r);
            free(l); free(r);
            return t;
        }
        default: return strdup("?");
    }
}

static void gen_stmt(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case NODE_STMT_LIST:
            for (int i = 0; i < node->data.stmt_list.count; i++) gen_stmt(node->data.stmt_list.stmts[i]);
            break;
        case NODE_DECL:
            if (node->data.decl.init) {
                char *v = gen_expr(node->data.decl.init);
                emit("%s = %s", node->data.decl.name, v);
                free(v);
            }
            break;
        case NODE_ASSIGN: {
            char *v = gen_expr(node->data.assign.value);
            emit("%s = %s", node->data.assign.name, v);
            free(v);
            break;
        }
        case NODE_IF: {
            char *c = gen_expr(node->data.if_stmt.cond);
            char *Lelse = new_label();
            char *Lend = new_label();
            emit("ifFalse %s goto %s", c, Lelse);
            free(c);
            gen_stmt(node->data.if_stmt.then_branch);
            emit("goto %s", Lend);
            emit("%s:", Lelse);
            if (node->data.if_stmt.else_branch) gen_stmt(node->data.if_stmt.else_branch);
            emit("%s:", Lend);
            free(Lelse); free(Lend);
            break;
        }
        case NODE_WHILE: {
            char *Lstart = new_label();
            char *Lend = new_label();
            emit("%s:", Lstart);
            char *c = gen_expr(node->data.while_stmt.cond);
            emit("ifFalse %s goto %s", c, Lend);
            free(c);
            gen_stmt(node->data.while_stmt.body);
            emit("goto %s", Lstart);
            emit("%s:", Lend);
            free(Lstart); free(Lend);
            break;
        }
        case NODE_FOR: {
            gen_stmt(node->data.for_stmt.init);
            char *Lstart = new_label();
            char *Lend = new_label();
            emit("%s:", Lstart);
            if (node->data.for_stmt.cond) {
                char *c = gen_expr(node->data.for_stmt.cond);
                emit("ifFalse %s goto %s", c, Lend);
                free(c);
            }
            gen_stmt(node->data.for_stmt.body);
            gen_stmt(node->data.for_stmt.step);
            emit("goto %s", Lstart);
            emit("%s:", Lend);
            free(Lstart); free(Lend);
            break;
        }
        case NODE_PRINT: {
            char *v = gen_expr(node->data.print_stmt.value);
            emit("print %s", v);
            free(v);
            break;
        }
        case NODE_BLOCK:
            gen_stmt(node->data.block.body);
            break;
        default: break;
    }
}

char **codegen_generate(ASTNode *program, int *out_count) {
    temp_count = 0; label_count = 0;
    g_lines = NULL; g_count = 0; g_cap = 0;
    gen_stmt(program);
    *out_count = g_count;
    return g_lines;
}

void codegen_free_lines(char **lines, int count) {
    if (!lines) return;
    for (int i = 0; i < count; i++) free(lines[i]);
    free(lines);
}
