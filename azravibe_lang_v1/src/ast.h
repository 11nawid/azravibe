#ifndef AZR_AST_H
#define AZR_AST_H

typedef enum {
    NODE_PROGRAM, NODE_BLOCK, NODE_ASSIGN, NODE_AUGMENTED_ASSIGN, NODE_IF, NODE_WHILE,
    NODE_FOR, NODE_FUNCTION_DEF, NODE_RETURN, NODE_BREAK, NODE_CONTINUE, NODE_PASS,
    NODE_CALL, NODE_BINARY_OP, NODE_UNARY_OP, NODE_IDENTIFIER, NODE_INT_LITERAL,
    NODE_FLOAT_LITERAL, NODE_STRING_LITERAL, NODE_BOOL_LITERAL, NODE_NONE_LITERAL,
    NODE_LIST_LITERAL, NODE_TUPLE_LITERAL, NODE_DICT_LITERAL, NODE_SET_LITERAL,
    NODE_INDEX, NODE_SLICE, NODE_IMPORT, NODE_FROM_IMPORT, NODE_TRY_EXCEPT, NODE_THROW,
    NODE_CLASS_DEF, NODE_ATTRIBUTE, NODE_ATTR_ASSIGN, NODE_WITH, NODE_EXPR_STMT,
    NODE_INDEX_ASSIGN, NODE_INDEX_AUGMENTED_ASSIGN, NODE_MULTI_ASSIGN, NODE_LAMBDA,
    NODE_TERNARY, NODE_LIST_COMPREHENSION, NODE_SET_COMPREHENSION, NODE_DICT_COMPREHENSION,
    NODE_YIELD
} NodeType;

typedef struct ASTNode {
    NodeType type;
    int line;
    union {
        struct { struct ASTNode **stmts; int count; int capacity; } block;
        struct { char *name; struct ASTNode *value; } assign;
        struct { struct ASTNode **targets; int target_count; struct ASTNode *value; } multi_assign;
        struct { char *name; char *op; struct ASTNode *value; } aug_assign;
        struct {
            struct ASTNode *condition;
            struct ASTNode *then_block;
            struct ASTNode **elif_conditions; int elif_count;
            struct ASTNode **elif_blocks;
            struct ASTNode *else_block;
        } if_stmt;
        struct { struct ASTNode *condition; struct ASTNode *body; } while_stmt;
        struct { char *var_name; struct ASTNode *iterable; struct ASTNode *body; } for_stmt;
        struct { char *name; char **params; struct ASTNode **defaults; int param_count; char *vararg; struct ASTNode *body; } func_def;
        struct { struct ASTNode *value; } return_stmt;
        struct { struct ASTNode *value; } yield_stmt;
        struct { struct ASTNode *callee; struct ASTNode **args; char **arg_names; int arg_count; } call;
        struct { char *op; struct ASTNode *left; struct ASTNode *right; } binary;
        struct { char *op; struct ASTNode *operand; } unary;
        struct { char *name; } identifier;
        struct { long long value; } int_lit;
        struct { double value; } float_lit;
        struct { char *value; } str_lit;
        struct { int value; } bool_lit;
        struct { struct ASTNode **items; int count; } list_lit;
        struct { struct ASTNode **items; int count; } tuple_lit;
        struct { struct ASTNode **keys; struct ASTNode **values; int count; int is_set; } dict_lit;
        struct { struct ASTNode *object; struct ASTNode *index; } index;
        struct { struct ASTNode *object; struct ASTNode *start; struct ASTNode *end; } slice;
        struct { struct ASTNode *object; struct ASTNode *index; struct ASTNode *value; } index_assign;
        struct { struct ASTNode *object; struct ASTNode *index; char *op; struct ASTNode *value; } index_aug_assign;
        struct { char *module_name; char *alias; } import;
        struct { char *module_name; char **names; char **aliases; int count; } from_import;
        struct {
            struct ASTNode *try_block;
            struct ASTNode **exception_types;
            char **exception_vars;
            struct ASTNode **except_blocks;
            int except_count;
            struct ASTNode *finally_block;
        } try_except;
        struct { struct ASTNode *value; } throw_stmt;
        struct { char *name; struct ASTNode *base_class; struct ASTNode *body; } class_def;
        struct { struct ASTNode *object; char *attr; } attribute;
        struct { struct ASTNode *object; char *attr; struct ASTNode *value; } attr_assign;
        struct { struct ASTNode *expr; char *alias; struct ASTNode *body; } with_stmt;
        struct { char **params; struct ASTNode **defaults; int param_count; char *vararg; struct ASTNode *body; } lambda;
        struct { struct ASTNode *then_expr; struct ASTNode *condition; struct ASTNode *else_expr; } ternary;
        struct {
            struct ASTNode *item_expr;
            struct ASTNode *value_expr;
            char **var_names;
            struct ASTNode **iterables;
            struct ASTNode **conditions;
            int loop_count;
        } comprehension;
        struct { struct ASTNode *expr; } expr_stmt;
    };
} ASTNode;

ASTNode *ast_make_block(NodeType type, int line);
void ast_block_add(ASTNode *block, ASTNode *stmt);
ASTNode *ast_make_assign(const char *name, ASTNode *value, int line);
ASTNode *ast_make_aug_assign(const char *name, const char *op, ASTNode *value, int line);
ASTNode *ast_make_if(ASTNode *condition, ASTNode *then_block, int line);
void ast_if_add_elif(ASTNode *node, ASTNode *condition, ASTNode *block);
void ast_if_set_else(ASTNode *node, ASTNode *block);
ASTNode *ast_make_while(ASTNode *condition, ASTNode *body, int line);
ASTNode *ast_make_for(const char *var_name, ASTNode *iterable, ASTNode *body, int line);
ASTNode *ast_make_function_def(const char *name, char **params, ASTNode **defaults, int param_count, const char *vararg, ASTNode *body, int line);
ASTNode *ast_make_return(ASTNode *value, int line);
ASTNode *ast_make_yield(ASTNode *value, int line);
ASTNode *ast_make_simple(NodeType type, int line);
ASTNode *ast_make_call(ASTNode *callee, ASTNode **args, char **arg_names, int arg_count, int line);
ASTNode *ast_make_binary(const char *op, ASTNode *left, ASTNode *right, int line);
ASTNode *ast_make_unary(const char *op, ASTNode *operand, int line);
ASTNode *ast_make_identifier(const char *name, int line);
ASTNode *ast_make_int(long long value, int line);
ASTNode *ast_make_float(double value, int line);
ASTNode *ast_make_string(const char *value, int line);
ASTNode *ast_make_bool(int value, int line);
ASTNode *ast_make_none(int line);
ASTNode *ast_make_list(ASTNode **items, int count, int line);
ASTNode *ast_make_tuple(ASTNode **items, int count, int line);
ASTNode *ast_make_dict(ASTNode **keys, ASTNode **values, int count, int is_set, int line);
ASTNode *ast_make_index(ASTNode *object, ASTNode *index, int line);
ASTNode *ast_make_slice(ASTNode *object, ASTNode *start, ASTNode *end, int line);
ASTNode *ast_make_index_assign(ASTNode *object, ASTNode *index, ASTNode *value, int line);
ASTNode *ast_make_index_aug_assign(ASTNode *object, ASTNode *index, const char *op, ASTNode *value, int line);
ASTNode *ast_make_import(const char *module_name, const char *alias, int line);
ASTNode *ast_make_from_import(const char *module_name, char **names, char **aliases, int count, int line);
ASTNode *ast_make_try_except(ASTNode *try_block, ASTNode *finally_block, int line);
void ast_try_add_except(ASTNode *node, ASTNode *exception_type, const char *exception_var, ASTNode *except_block);
ASTNode *ast_make_throw(ASTNode *value, int line);
ASTNode *ast_make_class_def(const char *name, ASTNode *base_class, ASTNode *body, int line);
ASTNode *ast_make_attribute(ASTNode *object, const char *attr, int line);
ASTNode *ast_make_attr_assign(ASTNode *object, const char *attr, ASTNode *value, int line);
ASTNode *ast_make_with(ASTNode *expr, const char *alias, ASTNode *body, int line);
ASTNode *ast_make_multi_assign(ASTNode **targets, int target_count, ASTNode *value, int line);
ASTNode *ast_make_lambda(char **params, ASTNode **defaults, int param_count, const char *vararg, ASTNode *body, int line);
ASTNode *ast_make_ternary(ASTNode *then_expr, ASTNode *condition, ASTNode *else_expr, int line);
ASTNode *ast_make_comprehension(NodeType type, ASTNode *item_expr, ASTNode *value_expr, char **var_names, ASTNode **iterables, ASTNode **conditions, int loop_count, int line);
ASTNode *ast_make_expr_stmt(ASTNode *expr, int line);
void ast_free(ASTNode *node);
void ast_print(ASTNode *node, int indent);

#endif
