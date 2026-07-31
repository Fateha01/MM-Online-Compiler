#ifndef AST_H
#define AST_H

/* ============================================================
 * Abstract Syntax Tree node definitions.
 *
 * Every construct in the language (Section 5 of the manual)
 * becomes one of these node types. Expression nodes carry a
 * `data_type` field that starts as TYPE_UNKNOWN and is filled
 * in by the semantic analyzer (src/semantic).
 * ============================================================ */

typedef enum {
    NODE_STMT_LIST,
    NODE_DECL,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_PRINT,
    NODE_BLOCK,
    NODE_BINOP,
    NODE_UNOP,
    NODE_IDENT,
    NODE_INT_LITERAL,
    NODE_FLOAT_LITERAL,
    NODE_BOOL_LITERAL
} NodeType;

typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_UNKNOWN   /* set before semantic analysis runs, or on error */
} DataType;

typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_LT, OP_GT, OP_LE, OP_GE, OP_EQ, OP_NE,
    OP_AND, OP_OR,
    OP_NOT, OP_NEG
} OpType;

typedef struct ASTNode {
    NodeType type;
    int line;
    DataType data_type;   /* filled in during semantic analysis for expr nodes */

    union {
        struct {
            struct ASTNode **stmts;
            int count;
            int capacity;
        } stmt_list;

        struct {
            DataType decl_type;
            char *name;
        } decl;

        struct {
            char *name;
            struct ASTNode *value;
        } assign;

        struct {
            struct ASTNode *cond;
            struct ASTNode *then_branch;
            struct ASTNode *else_branch; /* NULL if no else */
        } if_stmt;

        struct {
            struct ASTNode *cond;
            struct ASTNode *body;
        } while_stmt;

        struct {
            struct ASTNode *value;
        } print_stmt;

        struct {
            struct ASTNode *body; /* a NODE_STMT_LIST */
        } block;

        struct {
            OpType op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binop;

        struct {
            OpType op;
            struct ASTNode *operand;
        } unop;

        struct {
            char *name;
        } ident;

        struct { int value; }    int_literal;
        struct { double value; } float_literal;
        struct { int value; }    bool_literal; /* 0 = false, 1 = true */
    } data;
} ASTNode;

/* ---- construction ---- */
ASTNode *ast_new_stmt_list(void);
ASTNode *ast_append_stmt(ASTNode *list, ASTNode *stmt);

ASTNode *ast_new_decl(DataType type, char *name, int line);
ASTNode *ast_new_assign(char *name, ASTNode *value, int line);
ASTNode *ast_new_if(ASTNode *cond, ASTNode *then_branch, ASTNode *else_branch, int line);
ASTNode *ast_new_while(ASTNode *cond, ASTNode *body, int line);
ASTNode *ast_new_print(ASTNode *value, int line);
ASTNode *ast_new_block(ASTNode *body);

ASTNode *ast_new_binop(OpType op, ASTNode *left, ASTNode *right, int line);
ASTNode *ast_new_unop(OpType op, ASTNode *operand, int line);
ASTNode *ast_new_ident(char *name, int line);
ASTNode *ast_new_int_literal(int value, int line);
ASTNode *ast_new_float_literal(double value, int line);
ASTNode *ast_new_bool_literal(int value, int line);

/* ---- utility ---- */
const char *datatype_to_string(DataType t);
const char *optype_to_string(OpType op);

/* Formats a float value guaranteeing a decimal point is present (e.g.
 * "2.0" not "2"), so float literals never look like ints in printed
 * AST/TAC output. Writes into caller-supplied buf (must be >= 32 bytes)
 * and returns buf for convenience. */
char *format_float(double value, char *buf, int buf_size);

void ast_print(const ASTNode *node, int indent);
void ast_free(ASTNode *node);

#endif
