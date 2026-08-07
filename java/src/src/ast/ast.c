#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

static ASTNode *alloc_node(NodeType type, int line) {
    ASTNode *n = calloc(1, sizeof(ASTNode));
    n->type = type; n->line = line; n->data_type = TYPE_UNKNOWN;
    return n;
}

ASTNode *ast_new_stmt_list(void) {
    ASTNode *n = alloc_node(NODE_STMT_LIST, 0);
    n->data.stmt_list.capacity = 8;
    n->data.stmt_list.count = 0;
    n->data.stmt_list.stmts = malloc(sizeof(ASTNode*) * 8);
    return n;
}
ASTNode *ast_append_stmt(ASTNode *list, ASTNode *stmt) {
    if (list->data.stmt_list.count >= list->data.stmt_list.capacity) {
        list->data.stmt_list.capacity *= 2;
        list->data.stmt_list.stmts = realloc(list->data.stmt_list.stmts,
            sizeof(ASTNode*) * list->data.stmt_list.capacity);
    }
    list->data.stmt_list.stmts[list->data.stmt_list.count++] = stmt;
    return list;
}
ASTNode *ast_new_decl(DataType type, char *name, ASTNode *init, int line) {
    ASTNode *n = alloc_node(NODE_DECL, line);
    n->data.decl.decl_type = type; n->data.decl.name = strdup(name); n->data.decl.init = init;
    return n;
}
ASTNode *ast_new_assign(char *name, ASTNode *value, int line) {
    ASTNode *n = alloc_node(NODE_ASSIGN, line);
    n->data.assign.name = strdup(name); n->data.assign.value = value;
    return n;
}
ASTNode *ast_new_if(ASTNode *cond, ASTNode *then_branch, ASTNode *else_branch, int line) {
    ASTNode *n = alloc_node(NODE_IF, line);
    n->data.if_stmt.cond = cond; n->data.if_stmt.then_branch = then_branch; n->data.if_stmt.else_branch = else_branch;
    return n;
}
ASTNode *ast_new_while(ASTNode *cond, ASTNode *body, int line) {
    ASTNode *n = alloc_node(NODE_WHILE, line);
    n->data.while_stmt.cond = cond; n->data.while_stmt.body = body;
    return n;
}
ASTNode *ast_new_for(ASTNode *init, ASTNode *cond, ASTNode *step, ASTNode *body, int line) {
    ASTNode *n = alloc_node(NODE_FOR, line);
    n->data.for_stmt.init = init; n->data.for_stmt.cond = cond;
    n->data.for_stmt.step = step; n->data.for_stmt.body = body;
    return n;
}
ASTNode *ast_new_print(ASTNode *value, int line) {
    ASTNode *n = alloc_node(NODE_PRINT, line);
    n->data.print_stmt.value = value;
    return n;
}
ASTNode *ast_new_block(ASTNode *body) {
    ASTNode *n = alloc_node(NODE_BLOCK, body ? body->line : 0);
    n->data.block.body = body;
    return n;
}
ASTNode *ast_new_binop(OpType op, ASTNode *left, ASTNode *right, int line) {
    ASTNode *n = alloc_node(NODE_BINOP, line);
    n->data.binop.op = op; n->data.binop.left = left; n->data.binop.right = right;
    return n;
}
ASTNode *ast_new_unop(OpType op, ASTNode *operand, int line) {
    ASTNode *n = alloc_node(NODE_UNOP, line);
    n->data.unop.op = op; n->data.unop.operand = operand;
    return n;
}
ASTNode *ast_new_ident(char *name, int line) {
    ASTNode *n = alloc_node(NODE_IDENT, line);
    n->data.ident.name = strdup(name);
    return n;
}
ASTNode *ast_new_int_literal(int value, int line) {
    ASTNode *n = alloc_node(NODE_INT_LITERAL, line);
    n->data.int_literal.value = value; n->data_type = TYPE_INT;
    return n;
}
ASTNode *ast_new_float_literal(double value, int line) {
    ASTNode *n = alloc_node(NODE_FLOAT_LITERAL, line);
    n->data.float_literal.value = value; n->data_type = TYPE_FLOAT;
    return n;
}
ASTNode *ast_new_bool_literal(int value, int line) {
    ASTNode *n = alloc_node(NODE_BOOL_LITERAL, line);
    n->data.bool_literal.value = value; n->data_type = TYPE_BOOL;
    return n;
}
ASTNode *ast_new_string_literal(char *value, int line) {
    ASTNode *n = alloc_node(NODE_STRING_LITERAL, line);
    n->data.string_literal.value = strdup(value); n->data_type = TYPE_STRING;
    return n;
}

const char *datatype_to_string(DataType t) {
    switch (t) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_BOOL: return "bool";
        case TYPE_STRING: return "string";
        default: return "unknown";
    }
}
const char *optype_to_string(OpType op) {
    switch (op) {
        case OP_ADD: return "+"; case OP_SUB: return "-"; case OP_MUL: return "*";
        case OP_DIV: return "/"; case OP_MOD: return "%";
        case OP_LT: return "<"; case OP_GT: return ">"; case OP_LE: return "<=";
        case OP_GE: return ">="; case OP_EQ: return "=="; case OP_NE: return "!=";
        case OP_AND: return "&&"; case OP_OR: return "||";
        case OP_NOT: return "!"; case OP_NEG: return "neg";
        default: return "?";
    }
}

static void indent_print(int indent) { for (int i = 0; i < indent; i++) printf("  "); }

void ast_print(const ASTNode *node, int indent) {
    if (!node) return;
    switch (node->type) {
        case NODE_STMT_LIST:
            for (int i = 0; i < node->data.stmt_list.count; i++)
                ast_print(node->data.stmt_list.stmts[i], indent);
            break;
        case NODE_DECL:
            indent_print(indent);
            printf("Decl(%s %s)\n", datatype_to_string(node->data.decl.decl_type), node->data.decl.name);
            if (node->data.decl.init) ast_print(node->data.decl.init, indent + 1);
            break;
        case NODE_ASSIGN:
            indent_print(indent); printf("Assign(%s)\n", node->data.assign.name);
            ast_print(node->data.assign.value, indent + 1);
            break;
        case NODE_IF:
            indent_print(indent); printf("If\n");
            ast_print(node->data.if_stmt.cond, indent + 1);
            indent_print(indent); printf("Then\n");
            ast_print(node->data.if_stmt.then_branch, indent + 1);
            if (node->data.if_stmt.else_branch) {
                indent_print(indent); printf("Else\n");
                ast_print(node->data.if_stmt.else_branch, indent + 1);
            }
            break;
        case NODE_WHILE:
            indent_print(indent); printf("While\n");
            ast_print(node->data.while_stmt.cond, indent + 1);
            ast_print(node->data.while_stmt.body, indent + 1);
            break;
        case NODE_FOR:
            indent_print(indent); printf("For\n");
            if (node->data.for_stmt.init) ast_print(node->data.for_stmt.init, indent + 1);
            if (node->data.for_stmt.cond) ast_print(node->data.for_stmt.cond, indent + 1);
            if (node->data.for_stmt.step) ast_print(node->data.for_stmt.step, indent + 1);
            ast_print(node->data.for_stmt.body, indent + 1);
            break;
        case NODE_PRINT:
            indent_print(indent); printf("Print\n");
            ast_print(node->data.print_stmt.value, indent + 1);
            break;
        case NODE_BLOCK:
            indent_print(indent); printf("Block\n");
            ast_print(node->data.block.body, indent + 1);
            break;
        case NODE_BINOP:
            indent_print(indent); printf("BinOp(%s)\n", optype_to_string(node->data.binop.op));
            ast_print(node->data.binop.left, indent + 1);
            ast_print(node->data.binop.right, indent + 1);
            break;
        case NODE_UNOP:
            indent_print(indent); printf("UnOp(%s)\n", optype_to_string(node->data.unop.op));
            ast_print(node->data.unop.operand, indent + 1);
            break;
        case NODE_IDENT:
            indent_print(indent); printf("Ident(%s)\n", node->data.ident.name); break;
        case NODE_INT_LITERAL:
            indent_print(indent); printf("Int(%d)\n", node->data.int_literal.value); break;
        case NODE_FLOAT_LITERAL:
            indent_print(indent); printf("Float(%g)\n", node->data.float_literal.value); break;
        case NODE_BOOL_LITERAL:
            indent_print(indent); printf("Bool(%s)\n", node->data.bool_literal.value ? "true" : "false"); break;
        case NODE_STRING_LITERAL:
            indent_print(indent); printf("String(\"%s\")\n", node->data.string_literal.value); break;
    }
}

void ast_print_typed(const ASTNode *node, int indent) {
    if (!node) return;
    switch (node->type) {
        case NODE_STMT_LIST:
            for (int i = 0; i < node->data.stmt_list.count; i++)
                ast_print_typed(node->data.stmt_list.stmts[i], indent);
            break;
        case NODE_DECL:
            indent_print(indent);
            printf("Decl(%s %s)\n", datatype_to_string(node->data.decl.decl_type), node->data.decl.name);
            if (node->data.decl.init) ast_print_typed(node->data.decl.init, indent + 1);
            break;
        case NODE_ASSIGN:
            indent_print(indent); printf("Assign(%s)\n", node->data.assign.name);
            ast_print_typed(node->data.assign.value, indent + 1);
            break;
        case NODE_IF:
            indent_print(indent); printf("If\n");
            ast_print_typed(node->data.if_stmt.cond, indent + 1);
            indent_print(indent); printf("Then\n");
            ast_print_typed(node->data.if_stmt.then_branch, indent + 1);
            if (node->data.if_stmt.else_branch) {
                indent_print(indent); printf("Else\n");
                ast_print_typed(node->data.if_stmt.else_branch, indent + 1);
            }
            break;
        case NODE_WHILE:
            indent_print(indent); printf("While\n");
            ast_print_typed(node->data.while_stmt.cond, indent + 1);
            ast_print_typed(node->data.while_stmt.body, indent + 1);
            break;
        case NODE_FOR:
            indent_print(indent); printf("For\n");
            if (node->data.for_stmt.init) ast_print_typed(node->data.for_stmt.init, indent + 1);
            if (node->data.for_stmt.cond) ast_print_typed(node->data.for_stmt.cond, indent + 1);
            if (node->data.for_stmt.step) ast_print_typed(node->data.for_stmt.step, indent + 1);
            ast_print_typed(node->data.for_stmt.body, indent + 1);
            break;
        case NODE_PRINT:
            indent_print(indent); printf("Print\n");
            ast_print_typed(node->data.print_stmt.value, indent + 1);
            break;
        case NODE_BLOCK:
            indent_print(indent); printf("Block\n");
            ast_print_typed(node->data.block.body, indent + 1);
            break;
        case NODE_BINOP:
            indent_print(indent); printf("BinOp(%s) : %s\n", optype_to_string(node->data.binop.op), datatype_to_string(node->data_type));
            ast_print_typed(node->data.binop.left, indent + 1);
            ast_print_typed(node->data.binop.right, indent + 1);
            break;
        case NODE_UNOP:
            indent_print(indent); printf("UnOp(%s) : %s\n", optype_to_string(node->data.unop.op), datatype_to_string(node->data_type));
            ast_print_typed(node->data.unop.operand, indent + 1);
            break;
        case NODE_IDENT:
            indent_print(indent); printf("Ident(%s) : %s\n", node->data.ident.name, datatype_to_string(node->data_type)); break;
        case NODE_INT_LITERAL:
            indent_print(indent); printf("Int(%d) : %s\n", node->data.int_literal.value, datatype_to_string(node->data_type)); break;
        case NODE_FLOAT_LITERAL:
            indent_print(indent); printf("Float(%g) : %s\n", node->data.float_literal.value, datatype_to_string(node->data_type)); break;
        case NODE_BOOL_LITERAL:
            indent_print(indent); printf("Bool(%s) : %s\n", node->data.bool_literal.value ? "true" : "false", datatype_to_string(node->data_type)); break;
        case NODE_STRING_LITERAL:
            indent_print(indent); printf("String(\"%s\") : %s\n", node->data.string_literal.value, datatype_to_string(node->data_type)); break;
    }
}

void ast_free(ASTNode *node) {
    if (!node) return;
    switch (node->type) {
        case NODE_STMT_LIST:
            for (int i = 0; i < node->data.stmt_list.count; i++) ast_free(node->data.stmt_list.stmts[i]);
            free(node->data.stmt_list.stmts);
            break;
        case NODE_DECL:
            free(node->data.decl.name); ast_free(node->data.decl.init); break;
        case NODE_ASSIGN:
            free(node->data.assign.name); ast_free(node->data.assign.value); break;
        case NODE_IF:
            ast_free(node->data.if_stmt.cond); ast_free(node->data.if_stmt.then_branch);
            ast_free(node->data.if_stmt.else_branch); break;
        case NODE_WHILE:
            ast_free(node->data.while_stmt.cond); ast_free(node->data.while_stmt.body); break;
        case NODE_FOR:
            ast_free(node->data.for_stmt.init); ast_free(node->data.for_stmt.cond);
            ast_free(node->data.for_stmt.step); ast_free(node->data.for_stmt.body); break;
        case NODE_PRINT:
            ast_free(node->data.print_stmt.value); break;
        case NODE_BLOCK:
            ast_free(node->data.block.body); break;
        case NODE_BINOP:
            ast_free(node->data.binop.left); ast_free(node->data.binop.right); break;
        case NODE_UNOP:
            ast_free(node->data.unop.operand); break;
        case NODE_IDENT:
            free(node->data.ident.name); break;
        case NODE_STRING_LITERAL:
            free(node->data.string_literal.value); break;
        default: break;
    }
    free(node);
}


/* ================= Box-drawing Parse/Semantic Tree Printer ================= */
static char *node_label(const ASTNode *node, int typed, char *buf, size_t bufsz) {
    switch (node->type) {
        case NODE_STMT_LIST: snprintf(buf, bufsz, "Program"); break;
        case NODE_DECL:
            snprintf(buf, bufsz, "Decl(%s %s)", datatype_to_string(node->data.decl.decl_type), node->data.decl.name);
            break;
        case NODE_ASSIGN: snprintf(buf, bufsz, "Assignment(%s)", node->data.assign.name); break;
        case NODE_IF: snprintf(buf, bufsz, "If"); break;
        case NODE_WHILE: snprintf(buf, bufsz, "While"); break;
        case NODE_FOR: snprintf(buf, bufsz, "For"); break;
        case NODE_PRINT: snprintf(buf, bufsz, "Print"); break;
        case NODE_BLOCK: snprintf(buf, bufsz, "Block"); break;
        case NODE_BINOP:
            if (typed) snprintf(buf, bufsz, "Expr(%s) : %s", optype_to_string(node->data.binop.op), datatype_to_string(node->data_type));
            else snprintf(buf, bufsz, "Expr(%s)", optype_to_string(node->data.binop.op));
            break;
        case NODE_UNOP:
            if (typed) snprintf(buf, bufsz, "Expr(%s) : %s", optype_to_string(node->data.unop.op), datatype_to_string(node->data_type));
            else snprintf(buf, bufsz, "Expr(%s)", optype_to_string(node->data.unop.op));
            break;
        case NODE_IDENT:
            if (typed) snprintf(buf, bufsz, "id(%s) : %s", node->data.ident.name, datatype_to_string(node->data_type));
            else snprintf(buf, bufsz, "id(%s)", node->data.ident.name);
            break;
        case NODE_INT_LITERAL:
            if (typed) snprintf(buf, bufsz, "num(%d) : %s", node->data.int_literal.value, datatype_to_string(node->data_type));
            else snprintf(buf, bufsz, "num(%d)", node->data.int_literal.value);
            break;
        case NODE_FLOAT_LITERAL:
            if (typed) snprintf(buf, bufsz, "num(%g) : %s", node->data.float_literal.value, datatype_to_string(node->data_type));
            else snprintf(buf, bufsz, "num(%g)", node->data.float_literal.value);
            break;
        case NODE_BOOL_LITERAL:
            if (typed) snprintf(buf, bufsz, "bool(%s) : %s", node->data.bool_literal.value ? "true" : "false", datatype_to_string(node->data_type));
            else snprintf(buf, bufsz, "bool(%s)", node->data.bool_literal.value ? "true" : "false");
            break;
        case NODE_STRING_LITERAL:
            if (typed) snprintf(buf, bufsz, "str(\"%s\") : %s", node->data.string_literal.value, datatype_to_string(node->data_type));
            else snprintf(buf, bufsz, "str(\"%s\")", node->data.string_literal.value);
            break;
        default: snprintf(buf, bufsz, "?"); break;
    }
    return buf;
}

static void print_branch_line(const char *prefix, int is_last, const char *label) {
    printf("%s%s%s\n", prefix, is_last ? "\xE2\x94\x94\xE2\x94\x80\xE2\x94\x80 " : "\xE2\x94\x9C\xE2\x94\x80\xE2\x94\x80 ", label);
}
static void ext_prefix(const char *prefix, int is_last, char *out, size_t outsz) {
    snprintf(out, outsz, "%s%s", prefix, is_last ? "    " : "\xE2\x94\x82   ");
}

/* prefix = the indentation string at which THIS node's children rows should be drawn.
   (The node's own row was already printed by its caller.) */
static void tree_walk(const ASTNode *node, const char *prefix, int typed) {
    if (!node) return;
    char buf[160], cprefix[512];

    if (node->type == NODE_STMT_LIST) {
        int n = node->data.stmt_list.count;
        for (int i = 0; i < n; i++) {
            int last = (i == n - 1);
            ASTNode *child = node->data.stmt_list.stmts[i];
            print_branch_line(prefix, last, node_label(child, typed, buf, sizeof buf));
            ext_prefix(prefix, last, cprefix, sizeof cprefix);
            tree_walk(child, cprefix, typed);
        }
        return;
    }

    switch (node->type) {
        case NODE_DECL:
            if (node->data.decl.init) {
                print_branch_line(prefix, 1, node_label(node->data.decl.init, typed, buf, sizeof buf));
                ext_prefix(prefix, 1, cprefix, sizeof cprefix);
                tree_walk(node->data.decl.init, cprefix, typed);
            }
            break;
        case NODE_ASSIGN:
            print_branch_line(prefix, 1, node_label(node->data.assign.value, typed, buf, sizeof buf));
            ext_prefix(prefix, 1, cprefix, sizeof cprefix);
            tree_walk(node->data.assign.value, cprefix, typed);
            break;
        case NODE_IF: {
            int has_else = node->data.if_stmt.else_branch != NULL;
            print_branch_line(prefix, 0, node_label(node->data.if_stmt.cond, typed, buf, sizeof buf));
            ext_prefix(prefix, 0, cprefix, sizeof cprefix);
            tree_walk(node->data.if_stmt.cond, cprefix, typed);

            print_branch_line(prefix, !has_else, "Then");
            ext_prefix(prefix, !has_else, cprefix, sizeof cprefix);
            tree_walk(node->data.if_stmt.then_branch, cprefix, typed);

            if (has_else) {
                print_branch_line(prefix, 1, "Else");
                ext_prefix(prefix, 1, cprefix, sizeof cprefix);
                tree_walk(node->data.if_stmt.else_branch, cprefix, typed);
            }
            break;
        }
        case NODE_WHILE:
            print_branch_line(prefix, 0, node_label(node->data.while_stmt.cond, typed, buf, sizeof buf));
            ext_prefix(prefix, 0, cprefix, sizeof cprefix);
            tree_walk(node->data.while_stmt.cond, cprefix, typed);

            print_branch_line(prefix, 1, "Body");
            ext_prefix(prefix, 1, cprefix, sizeof cprefix);
            tree_walk(node->data.while_stmt.body, cprefix, typed);
            break;
        case NODE_FOR: {
            /* collect present parts: init, cond, step, body(always) */
            const char *names[4]; const ASTNode *parts[4]; int npart = 0;
            if (node->data.for_stmt.init) { names[npart]="Init"; parts[npart]=node->data.for_stmt.init; npart++; }
            if (node->data.for_stmt.cond) { names[npart]="Cond"; parts[npart]=node->data.for_stmt.cond; npart++; }
            if (node->data.for_stmt.step) { names[npart]="Step"; parts[npart]=node->data.for_stmt.step; npart++; }
            names[npart]="Body"; parts[npart]=node->data.for_stmt.body; npart++;
            for (int i = 0; i < npart; i++) {
                int last = (i == npart - 1);
                /* Init/Cond/Step show as "Name: <expr-label>" leaf-style; Body shown as its own subtree */
                if (strcmp(names[i], "Body") == 0) {
                    print_branch_line(prefix, last, "Body");
                    ext_prefix(prefix, last, cprefix, sizeof cprefix);
                    tree_walk(parts[i], cprefix, typed);
                } else {
                    print_branch_line(prefix, last, names[i]);
                    ext_prefix(prefix, last, cprefix, sizeof cprefix);
                    print_branch_line(cprefix, 1, node_label(parts[i], typed, buf, sizeof buf));
                    char cprefix2[512]; ext_prefix(cprefix, 1, cprefix2, sizeof cprefix2);
                    tree_walk(parts[i], cprefix2, typed);
                }
            }
            break;
        }
        case NODE_PRINT:
            print_branch_line(prefix, 1, node_label(node->data.print_stmt.value, typed, buf, sizeof buf));
            ext_prefix(prefix, 1, cprefix, sizeof cprefix);
            tree_walk(node->data.print_stmt.value, cprefix, typed);
            break;
        case NODE_BLOCK:
            tree_walk(node->data.block.body, prefix, typed);
            break;
        case NODE_BINOP:
            print_branch_line(prefix, 0, node_label(node->data.binop.left, typed, buf, sizeof buf));
            ext_prefix(prefix, 0, cprefix, sizeof cprefix);
            tree_walk(node->data.binop.left, cprefix, typed);

            print_branch_line(prefix, 1, node_label(node->data.binop.right, typed, buf, sizeof buf));
            ext_prefix(prefix, 1, cprefix, sizeof cprefix);
            tree_walk(node->data.binop.right, cprefix, typed);
            break;
        case NODE_UNOP:
            print_branch_line(prefix, 1, node_label(node->data.unop.operand, typed, buf, sizeof buf));
            ext_prefix(prefix, 1, cprefix, sizeof cprefix);
            tree_walk(node->data.unop.operand, cprefix, typed);
            break;
        default: break; /* leaves: ident/literals have no children */
    }
}

void ast_print_tree(const ASTNode *node, int typed) {
    printf("Program\n");
    tree_walk(node, "", typed);
}

/* ================= Centered ASCII Tree Printer (/, |, \ style) ================= */
typedef struct GNode {
    char label[160];
    struct GNode *children[16];
    int nchild;
} GNode;

static GNode *gnew(const char *label) {
    GNode *g = calloc(1, sizeof(GNode));
    strncpy(g->label, label, sizeof(g->label) - 1);
    return g;
}
static void gadd(GNode *p, GNode *c) {
    if (c && p->nchild < 16) p->children[p->nchild++] = c;
}

static GNode *build_one(const ASTNode *node, int typed);
static int build_list(const ASTNode *node, int typed, GNode **out, int max);

static int build_list(const ASTNode *node, int typed, GNode **out, int max) {
    if (!node) return 0;
    if (node->type == NODE_BLOCK) return build_list(node->data.block.body, typed, out, max);
    if (node->type == NODE_STMT_LIST) {
        int n = 0;
        for (int i = 0; i < node->data.stmt_list.count && n < max; i++)
            out[n++] = build_one(node->data.stmt_list.stmts[i], typed);
        return n;
    }
    out[0] = build_one(node, typed);
    return 1;
}

static GNode *build_one(const ASTNode *node, int typed) {
    if (!node) return NULL;
    char buf[160];
    GNode *g = gnew(node_label(node, typed, buf, sizeof buf));
    GNode *kids[16]; int n;
    switch (node->type) {
        case NODE_DECL:
            if (node->data.decl.init) { n = build_list(node->data.decl.init, typed, kids, 16); for (int i = 0; i < n; i++) gadd(g, kids[i]); }
            break;
        case NODE_ASSIGN:
            n = build_list(node->data.assign.value, typed, kids, 16); for (int i = 0; i < n; i++) gadd(g, kids[i]);
            break;
        case NODE_IF: {
            n = build_list(node->data.if_stmt.cond, typed, kids, 16); for (int i = 0; i < n; i++) gadd(g, kids[i]);
            GNode *thenw = gnew("Then");
            n = build_list(node->data.if_stmt.then_branch, typed, kids, 16); for (int i = 0; i < n; i++) gadd(thenw, kids[i]);
            gadd(g, thenw);
            if (node->data.if_stmt.else_branch) {
                GNode *elsew = gnew("Else");
                n = build_list(node->data.if_stmt.else_branch, typed, kids, 16); for (int i = 0; i < n; i++) gadd(elsew, kids[i]);
                gadd(g, elsew);
            }
            break;
        }
        case NODE_WHILE: {
            n = build_list(node->data.while_stmt.cond, typed, kids, 16); for (int i = 0; i < n; i++) gadd(g, kids[i]);
            GNode *bodyw = gnew("Body");
            n = build_list(node->data.while_stmt.body, typed, kids, 16); for (int i = 0; i < n; i++) gadd(bodyw, kids[i]);
            gadd(g, bodyw);
            break;
        }
        case NODE_FOR: {
            if (node->data.for_stmt.init) gadd(g, build_one(node->data.for_stmt.init, typed));
            if (node->data.for_stmt.cond) gadd(g, build_one(node->data.for_stmt.cond, typed));
            if (node->data.for_stmt.step) gadd(g, build_one(node->data.for_stmt.step, typed));
            GNode *bodyw = gnew("Body");
            n = build_list(node->data.for_stmt.body, typed, kids, 16); for (int i = 0; i < n; i++) gadd(bodyw, kids[i]);
            gadd(g, bodyw);
            break;
        }
        case NODE_PRINT:
            n = build_list(node->data.print_stmt.value, typed, kids, 16); for (int i = 0; i < n; i++) gadd(g, kids[i]);
            break;
        case NODE_BLOCK:
            n = build_list(node->data.block.body, typed, kids, 16); for (int i = 0; i < n; i++) gadd(g, kids[i]);
            break;
        case NODE_BINOP:
            gadd(g, build_one(node->data.binop.left, typed));
            gadd(g, build_one(node->data.binop.right, typed));
            break;
        case NODE_UNOP:
            gadd(g, build_one(node->data.unop.operand, typed));
            break;
        default: break;
    }
    return g;
}

typedef struct Box {
    char **lines;
    int height, width, root_col;
} Box;

static char *mkline(int width) {
    char *s = malloc((size_t)width + 1);
    memset(s, ' ', (size_t)width);
    s[width] = 0;
    return s;
}
static void put_str(char *line, int col, const char *s) {
    int len = (int)strlen(s);
    for (int i = 0; i < len; i++) line[col + i] = s[i];
}

static void box_free(Box b) {
    for (int i = 0; i < b.height; i++) free(b.lines[i]);
    free(b.lines);
}

static Box render_box(GNode *g) {
    Box b;
    int lw = (int)strlen(g->label);
    if (g->nchild == 0) {
        b.height = 1; b.width = lw; b.root_col = lw / 2;
        b.lines = malloc(sizeof(char *));
        b.lines[0] = mkline(lw);
        put_str(b.lines[0], 0, g->label);
        return b;
    }
    Box *cb = malloc(sizeof(Box) * g->nchild);
    for (int i = 0; i < g->nchild; i++) cb[i] = render_box(g->children[i]);

    int gap = 2, content_width = 0;
    for (int i = 0; i < g->nchild; i++) content_width += cb[i].width + (i ? gap : 0);
    int final_width = content_width > lw ? content_width : lw;
    int left_pad = final_width > content_width ? (final_width - content_width) / 2 : 0;

    int *start = malloc(sizeof(int) * g->nchild);
    int *abs_root = malloc(sizeof(int) * g->nchild);
    int col = left_pad;
    for (int i = 0; i < g->nchild; i++) { start[i] = col; abs_root[i] = col + cb[i].root_col; col += cb[i].width + gap; }
    int center = (abs_root[0] + abs_root[g->nchild - 1]) / 2;

    int max_h = 0;
    for (int i = 0; i < g->nchild; i++) if (cb[i].height > max_h) max_h = cb[i].height;

    char **lines = malloc(sizeof(char *) * (2 + max_h));
    lines[0] = mkline(final_width);
    int lstart = center - lw / 2;
    if (lstart < 0) lstart = 0;
    if (lstart + lw > final_width) lstart = final_width - lw;
    put_str(lines[0], lstart, g->label);

    lines[1] = mkline(final_width);
    for (int i = 0; i < g->nchild; i++) {
        char c = (abs_root[i] == center) ? '|' : (abs_root[i] < center ? '/' : '\\');
        lines[1][abs_root[i]] = c;
    }
    for (int r = 0; r < max_h; r++) {
        lines[2 + r] = mkline(final_width);
        for (int i = 0; i < g->nchild; i++)
            if (r < cb[i].height) put_str(lines[2 + r], start[i], cb[i].lines[r]);
    }
    b.lines = lines; b.height = 2 + max_h; b.width = final_width; b.root_col = center;
    for (int i = 0; i < g->nchild; i++) box_free(cb[i]);
    free(start); free(abs_root); free(cb);
    return b;
}

static void gnode_free(GNode *g) {
    if (!g) return;
    for (int i = 0; i < g->nchild; i++) gnode_free(g->children[i]);
    free(g);
}

void ast_print_tree_centered(const ASTNode *node, int typed) {
    GNode *root = gnew("Program");
    GNode *kids[64]; int n = build_list(node, typed, kids, 64);
    for (int i = 0; i < n; i++) gadd(root, kids[i]);
    Box b = render_box(root);
    for (int i = 0; i < b.height; i++) printf("%s\n", b.lines[i]);
    box_free(b);
    gnode_free(root);
}
