#ifndef AST_H
#define AST_H
/* ===== MM_compiler / C frontend : AST ===== */
typedef enum {
    NODE_STMT_LIST, NODE_DECL, NODE_ASSIGN, NODE_IF, NODE_WHILE, NODE_FOR,
    NODE_PRINT, NODE_BLOCK, NODE_BINOP, NODE_UNOP, NODE_IDENT,
    NODE_INT_LITERAL, NODE_FLOAT_LITERAL, NODE_BOOL_LITERAL, NODE_STRING_LITERAL
} NodeType;

typedef enum { TYPE_INT, TYPE_FLOAT, TYPE_BOOL, TYPE_STRING, TYPE_UNKNOWN } DataType;

typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_LT, OP_GT, OP_LE, OP_GE, OP_EQ, OP_NE,
    OP_AND, OP_OR, OP_NOT, OP_NEG
} OpType;

typedef struct ASTNode {
    NodeType type;
    int line;
    DataType data_type;
    union {
        struct { struct ASTNode **stmts; int count; int capacity; } stmt_list;
        struct { DataType decl_type; char *name; struct ASTNode *init; } decl;
        struct { char *name; struct ASTNode *value; } assign;
        struct { struct ASTNode *cond, *then_branch, *else_branch; } if_stmt;
        struct { struct ASTNode *cond, *body; } while_stmt;
        struct { struct ASTNode *init, *cond, *step, *body; } for_stmt;
        struct { struct ASTNode *value; } print_stmt;
        struct { struct ASTNode *body; } block;
        struct { OpType op; struct ASTNode *left, *right; } binop;
        struct { OpType op; struct ASTNode *operand; } unop;
        struct { char *name; } ident;
        struct { int value; } int_literal;
        struct { double value; } float_literal;
        struct { int value; } bool_literal;
        struct { char *value; } string_literal;
    } data;
} ASTNode;

ASTNode *ast_new_stmt_list(void);
ASTNode *ast_append_stmt(ASTNode *list, ASTNode *stmt);
ASTNode *ast_new_decl(DataType type, char *name, ASTNode *init, int line);
ASTNode *ast_new_assign(char *name, ASTNode *value, int line);
ASTNode *ast_new_if(ASTNode *cond, ASTNode *then_branch, ASTNode *else_branch, int line);
ASTNode *ast_new_while(ASTNode *cond, ASTNode *body, int line);
ASTNode *ast_new_for(ASTNode *init, ASTNode *cond, ASTNode *step, ASTNode *body, int line);
ASTNode *ast_new_print(ASTNode *value, int line);
ASTNode *ast_new_block(ASTNode *body);
ASTNode *ast_new_binop(OpType op, ASTNode *left, ASTNode *right, int line);
ASTNode *ast_new_unop(OpType op, ASTNode *operand, int line);
ASTNode *ast_new_ident(char *name, int line);
ASTNode *ast_new_int_literal(int value, int line);
ASTNode *ast_new_float_literal(double value, int line);
ASTNode *ast_new_bool_literal(int value, int line);
ASTNode *ast_new_string_literal(char *value, int line);

const char *datatype_to_string(DataType t);
const char *optype_to_string(OpType op);
void ast_print(const ASTNode *node, int indent);
void ast_print_typed(const ASTNode *node, int indent);
void ast_print_tree(const ASTNode *node, int typed);
void ast_free(ASTNode *node);
#endif
