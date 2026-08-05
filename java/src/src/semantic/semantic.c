#include <stdio.h>
#include <string.h>
#include "semantic.h"

static int had_error = 0;

static void err(int line, const char *fmt, const char *arg) {
    fprintf(stderr, "Semantic Error (line %d): ", line);
    fprintf(stderr, fmt, arg);
    fprintf(stderr, "\n");
    had_error = 1;
}

static DataType check_expr(ASTNode *node, SymbolTable *table) {
    if (!node) return TYPE_UNKNOWN;
    switch (node->type) {
        case NODE_INT_LITERAL: node->data_type = TYPE_INT; return TYPE_INT;
        case NODE_FLOAT_LITERAL: node->data_type = TYPE_FLOAT; return TYPE_FLOAT;
        case NODE_BOOL_LITERAL: node->data_type = TYPE_BOOL; return TYPE_BOOL;
        case NODE_STRING_LITERAL: node->data_type = TYPE_STRING; return TYPE_STRING;
        case NODE_IDENT: {
            Symbol *s = symtab_lookup(table, node->data.ident.name);
            if (!s) { err(node->line, "undeclared variable '%s'", node->data.ident.name); node->data_type = TYPE_UNKNOWN; return TYPE_UNKNOWN; }
            node->data_type = s->type;
            return s->type;
        }
        case NODE_UNOP: {
            DataType t = check_expr(node->data.unop.operand, table);
            if (node->data.unop.op == OP_NOT) {
                if (t != TYPE_BOOL) err(node->line, "'!' requires bool operand", NULL);
                node->data_type = TYPE_BOOL;
            } else { /* OP_NEG */
                if (t != TYPE_INT && t != TYPE_FLOAT) err(node->line, "unary '-' requires numeric operand", NULL);
                node->data_type = t;
            }
            return node->data_type;
        }
        case NODE_BINOP: {
            DataType lt = check_expr(node->data.binop.left, table);
            DataType rt = check_expr(node->data.binop.right, table);
            OpType op = node->data.binop.op;
            if (op == OP_AND || op == OP_OR) {
                if (lt != TYPE_BOOL || rt != TYPE_BOOL) err(node->line, "logical operator requires bool operands", NULL);
                node->data_type = TYPE_BOOL;
            } else if (op == OP_EQ || op == OP_NE) {
                if (lt != rt) err(node->line, "type mismatch in equality comparison", NULL);
                node->data_type = TYPE_BOOL;
            } else if (op == OP_LT || op == OP_GT || op == OP_LE || op == OP_GE) {
                if ((lt != TYPE_INT && lt != TYPE_FLOAT) || (rt != TYPE_INT && rt != TYPE_FLOAT))
                    err(node->line, "relational operator requires numeric operands", NULL);
                node->data_type = TYPE_BOOL;
            } else { /* arithmetic */
                if ((lt != TYPE_INT && lt != TYPE_FLOAT) || (rt != TYPE_INT && rt != TYPE_FLOAT)) {
                    err(node->line, "arithmetic operator requires numeric operands", NULL);
                    node->data_type = TYPE_UNKNOWN;
                } else {
                    node->data_type = (lt == TYPE_FLOAT || rt == TYPE_FLOAT) ? TYPE_FLOAT : TYPE_INT;
                }
            }
            return node->data_type;
        }
        default:
            return TYPE_UNKNOWN;
    }
}

static void check_stmt(ASTNode *node, SymbolTable *table) {
    if (!node) return;
    switch (node->type) {
        case NODE_STMT_LIST:
            for (int i = 0; i < node->data.stmt_list.count; i++)
                check_stmt(node->data.stmt_list.stmts[i], table);
            break;
        case NODE_DECL: {
            if (!symtab_declare(table, node->data.decl.name, node->data.decl.decl_type, node->line))
                err(node->line, "redeclaration of variable '%s'", node->data.decl.name);
            if (node->data.decl.init) {
                DataType it = check_expr(node->data.decl.init, table);
                if (it != TYPE_UNKNOWN && it != node->data.decl.decl_type &&
                    !(it == TYPE_INT && node->data.decl.decl_type == TYPE_FLOAT))
                    err(node->line, "type mismatch in initializer for '%s'", node->data.decl.name);
            }
            break;
        }
        case NODE_ASSIGN: {
            Symbol *s = symtab_lookup(table, node->data.assign.name);
            if (!s) { err(node->line, "assignment to undeclared variable '%s'", node->data.assign.name); break; }
            DataType vt = check_expr(node->data.assign.value, table);
            if (vt != TYPE_UNKNOWN && vt != s->type && !(vt == TYPE_INT && s->type == TYPE_FLOAT))
                err(node->line, "type mismatch in assignment to '%s'", node->data.assign.name);
            break;
        }
        case NODE_IF:
            if (check_expr(node->data.if_stmt.cond, table) != TYPE_BOOL)
                err(node->line, "if-condition must be bool", NULL);
            check_stmt(node->data.if_stmt.then_branch, table);
            check_stmt(node->data.if_stmt.else_branch, table);
            break;
        case NODE_WHILE:
            if (check_expr(node->data.while_stmt.cond, table) != TYPE_BOOL)
                err(node->line, "while-condition must be bool", NULL);
            check_stmt(node->data.while_stmt.body, table);
            break;
        case NODE_FOR:
            check_stmt(node->data.for_stmt.init, table);
            if (node->data.for_stmt.cond && check_expr(node->data.for_stmt.cond, table) != TYPE_BOOL)
                err(node->line, "for-condition must be bool", NULL);
            check_stmt(node->data.for_stmt.step, table);
            check_stmt(node->data.for_stmt.body, table);
            break;
        case NODE_PRINT:
            check_expr(node->data.print_stmt.value, table);
            break;
        case NODE_BLOCK:
            check_stmt(node->data.block.body, table);
            break;
        default: break;
    }
}

int semantic_analyze(ASTNode *program, SymbolTable *table) {
    had_error = 0;
    check_stmt(program, table);
    return had_error ? 0 : 1;
}
