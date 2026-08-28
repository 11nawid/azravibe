#include "ast.h"
#include "utf8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Allocates an AST node initialized with a type and source line. */
static ASTNode *node_new(NodeType type, int line) {
    ASTNode *n = (ASTNode *)calloc(1, sizeof(ASTNode));
    n->type = type;
    n->line = line;
    return n;
}

/* Creates a program or block node with a growable statement array. */
ASTNode *ast_make_block(NodeType type, int line) {
    ASTNode *n = node_new(type, line);
    n->block.capacity = 8;
    n->block.stmts = (ASTNode **)calloc((size_t)n->block.capacity, sizeof(ASTNode *));
    return n;
}

/* Appends a statement to a program or block node. */
void ast_block_add(ASTNode *block, ASTNode *stmt) {
    if (!block || !stmt) return;
    if (block->block.count >= block->block.capacity) {
        block->block.capacity *= 2;
        block->block.stmts = (ASTNode **)realloc(block->block.stmts, (size_t)block->block.capacity * sizeof(ASTNode *));
    }
    block->block.stmts[block->block.count++] = stmt;
}

/* Creates a normal assignment node. */
ASTNode *ast_make_assign(const char *name, ASTNode *value, int line) {
    ASTNode *n = node_new(NODE_ASSIGN, line);
    n->assign.name = azr_strdup(name);
    n->assign.value = value;
    return n;
}

/* Creates an augmented assignment node such as +=. */
ASTNode *ast_make_aug_assign(const char *name, const char *op, ASTNode *value, int line) {
    ASTNode *n = node_new(NODE_AUGMENTED_ASSIGN, line);
    n->aug_assign.name = azr_strdup(name);
    n->aug_assign.op = azr_strdup(op);
    n->aug_assign.value = value;
    return n;
}

/* Creates an if statement with a required condition and then block. */
ASTNode *ast_make_if(ASTNode *condition, ASTNode *then_block, int line) {
    ASTNode *n = node_new(NODE_IF, line);
    n->if_stmt.condition = condition;
    n->if_stmt.then_block = then_block;
    return n;
}

/* Adds an elif condition/block pair to an if statement. */
void ast_if_add_elif(ASTNode *node, ASTNode *condition, ASTNode *block) {
    int c = node->if_stmt.elif_count;
    node->if_stmt.elif_conditions = (ASTNode **)realloc(node->if_stmt.elif_conditions, (size_t)(c + 1) * sizeof(ASTNode *));
    node->if_stmt.elif_blocks = (ASTNode **)realloc(node->if_stmt.elif_blocks, (size_t)(c + 1) * sizeof(ASTNode *));
    node->if_stmt.elif_conditions[c] = condition;
    node->if_stmt.elif_blocks[c] = block;
    node->if_stmt.elif_count++;
}

/* Sets the optional else block of an if statement. */
void ast_if_set_else(ASTNode *node, ASTNode *block) {
    node->if_stmt.else_block = block;
}

/* Creates a while statement node. */
ASTNode *ast_make_while(ASTNode *condition, ASTNode *body, int line) {
    ASTNode *n = node_new(NODE_WHILE, line);
    n->while_stmt.condition = condition;
    n->while_stmt.body = body;
    return n;
}

/* Creates a for-each statement node. */
ASTNode *ast_make_for(const char *var_name, ASTNode *iterable, ASTNode *body, int line) {
    ASTNode *n = node_new(NODE_FOR, line);
    n->for_stmt.var_name = azr_strdup(var_name);
    n->for_stmt.iterable = iterable;
    n->for_stmt.body = body;
    return n;
}

/* Creates a user function definition node. */
ASTNode *ast_make_function_def(const char *name, char **params, ASTNode **defaults, int param_count, const char *vararg, ASTNode *body, int line) {
    ASTNode *n = node_new(NODE_FUNCTION_DEF, line);
    n->func_def.name = azr_strdup(name);
    n->func_def.params = params;
    n->func_def.defaults = defaults;
    n->func_def.param_count = param_count;
    n->func_def.vararg = vararg ? azr_strdup(vararg) : NULL;
    n->func_def.body = body;
    return n;
}

/* Creates a return node with an optional value expression. */
ASTNode *ast_make_return(ASTNode *value, int line) {
    ASTNode *n = node_new(NODE_RETURN, line);
    n->return_stmt.value = value;
    return n;
}

ASTNode *ast_make_yield(ASTNode *value, int line) {
    ASTNode *n = node_new(NODE_YIELD, line);
    n->yield_stmt.value = value;
    return n;
}

/* Creates a simple statement node without child fields. */
ASTNode *ast_make_simple(NodeType type, int line) {
    return node_new(type, line);
}

/* Creates a call expression node. */
ASTNode *ast_make_call(ASTNode *callee, ASTNode **args, char **arg_names, int arg_count, int line) {
    ASTNode *n = node_new(NODE_CALL, line);
    n->call.callee = callee;
    n->call.args = args;
    n->call.arg_names = arg_names;
    n->call.arg_count = arg_count;
    return n;
}

/* Creates a binary operator expression node. */
ASTNode *ast_make_binary(const char *op, ASTNode *left, ASTNode *right, int line) {
    ASTNode *n = node_new(NODE_BINARY_OP, line);
    n->binary.op = azr_strdup(op);
    n->binary.left = left;
    n->binary.right = right;
    return n;
}

/* Creates a unary operator expression node. */
ASTNode *ast_make_unary(const char *op, ASTNode *operand, int line) {
    ASTNode *n = node_new(NODE_UNARY_OP, line);
    n->unary.op = azr_strdup(op);
    n->unary.operand = operand;
    return n;
}

/* Creates an identifier expression node. */
ASTNode *ast_make_identifier(const char *name, int line) {
    ASTNode *n = node_new(NODE_IDENTIFIER, line);
    n->identifier.name = azr_strdup(name);
    return n;
}

/* Creates an integer literal node. */
ASTNode *ast_make_int(long long value, int line) {
    ASTNode *n = node_new(NODE_INT_LITERAL, line);
    n->int_lit.value = value;
    return n;
}

/* Creates a floating-point literal node. */
ASTNode *ast_make_float(double value, int line) {
    ASTNode *n = node_new(NODE_FLOAT_LITERAL, line);
    n->float_lit.value = value;
    return n;
}

/* Creates a string literal node. */
ASTNode *ast_make_string(const char *value, int line) {
    ASTNode *n = node_new(NODE_STRING_LITERAL, line);
    n->str_lit.value = azr_strdup(value);
    return n;
}

/* Creates a boolean literal node. */
ASTNode *ast_make_bool(int value, int line) {
    ASTNode *n = node_new(NODE_BOOL_LITERAL, line);
    n->bool_lit.value = value;
    return n;
}

/* Creates a none literal node. */
ASTNode *ast_make_none(int line) {
    return node_new(NODE_NONE_LITERAL, line);
}

/* Creates a list literal node. */
ASTNode *ast_make_list(ASTNode **items, int count, int line) {
    ASTNode *n = node_new(NODE_LIST_LITERAL, line);
    n->list_lit.items = items;
    n->list_lit.count = count;
    return n;
}

ASTNode *ast_make_tuple(ASTNode **items, int count, int line) {
    ASTNode *n = node_new(NODE_TUPLE_LITERAL, line);
    n->tuple_lit.items = items;
    n->tuple_lit.count = count;
    return n;
}

ASTNode *ast_make_dict(ASTNode **keys, ASTNode **values, int count, int is_set, int line) {
    ASTNode *n = node_new(is_set ? NODE_SET_LITERAL : NODE_DICT_LITERAL, line);
    n->dict_lit.keys = keys;
    n->dict_lit.values = values;
    n->dict_lit.count = count;
    n->dict_lit.is_set = is_set;
    return n;
}

/* Creates an index expression node. */
ASTNode *ast_make_index(ASTNode *object, ASTNode *index, int line) {
    ASTNode *n = node_new(NODE_INDEX, line);
    n->index.object = object;
    n->index.index = index;
    return n;
}

ASTNode *ast_make_slice(ASTNode *object, ASTNode *start, ASTNode *end, int line) {
    ASTNode *n = node_new(NODE_SLICE, line);
    n->slice.object = object;
    n->slice.start = start;
    n->slice.end = end;
    return n;
}

/* Creates an indexed assignment node such as list[i] = value. */
ASTNode *ast_make_index_assign(ASTNode *object, ASTNode *index, ASTNode *value, int line) {
    ASTNode *n = node_new(NODE_INDEX_ASSIGN, line);
    n->index_assign.object = object;
    n->index_assign.index = index;
    n->index_assign.value = value;
    return n;
}

/* Creates an indexed augmented assignment node such as list[i] += value. */
ASTNode *ast_make_index_aug_assign(ASTNode *object, ASTNode *index, const char *op, ASTNode *value, int line) {
    ASTNode *n = node_new(NODE_INDEX_AUGMENTED_ASSIGN, line);
    n->index_aug_assign.object = object;
    n->index_aug_assign.index = index;
    n->index_aug_assign.op = azr_strdup(op);
    n->index_aug_assign.value = value;
    return n;
}

/* Creates an import statement node with optional alias. */
ASTNode *ast_make_import(const char *module_name, const char *alias, int line) {
    ASTNode *n = node_new(NODE_IMPORT, line);
    n->import.module_name = azr_strdup(module_name);
    n->import.alias = alias ? azr_strdup(alias) : NULL;
    return n;
}

ASTNode *ast_make_from_import(const char *module_name, char **names, char **aliases, int count, int line) {
    ASTNode *n = node_new(NODE_FROM_IMPORT, line);
    n->from_import.module_name = azr_strdup(module_name);
    n->from_import.names = names;
    n->from_import.aliases = aliases;
    n->from_import.count = count;
    return n;
}

/* Creates a try/except/finally statement node. */
ASTNode *ast_make_try_except(ASTNode *try_block, ASTNode *finally_block, int line) {
    ASTNode *n = node_new(NODE_TRY_EXCEPT, line);
    n->try_except.try_block = try_block;
    n->try_except.finally_block = finally_block;
    return n;
}

void ast_try_add_except(ASTNode *node, ASTNode *exception_type, const char *exception_var, ASTNode *except_block) {
    int c = 0;
    if (!node || node->type != NODE_TRY_EXCEPT) return;
    c = node->try_except.except_count;
    node->try_except.exception_types = (ASTNode **)realloc(node->try_except.exception_types, (size_t)(c + 1) * sizeof(ASTNode *));
    node->try_except.exception_vars = (char **)realloc(node->try_except.exception_vars, (size_t)(c + 1) * sizeof(char *));
    node->try_except.except_blocks = (ASTNode **)realloc(node->try_except.except_blocks, (size_t)(c + 1) * sizeof(ASTNode *));
    node->try_except.exception_types[c] = exception_type;
    node->try_except.exception_vars[c] = exception_var ? azr_strdup(exception_var) : NULL;
    node->try_except.except_blocks[c] = except_block;
    node->try_except.except_count++;
}

ASTNode *ast_make_throw(ASTNode *value, int line) {
    ASTNode *n = node_new(NODE_THROW, line);
    n->throw_stmt.value = value;
    return n;
}

ASTNode *ast_make_class_def(const char *name, ASTNode *base_class, ASTNode *body, int line) {
    ASTNode *n = node_new(NODE_CLASS_DEF, line);
    n->class_def.name = azr_strdup(name);
    n->class_def.base_class = base_class;
    n->class_def.body = body;
    return n;
}

/* Creates an attribute access expression node. */
ASTNode *ast_make_attribute(ASTNode *object, const char *attr, int line) {
    ASTNode *n = node_new(NODE_ATTRIBUTE, line);
    n->attribute.object = object;
    n->attribute.attr = azr_strdup(attr);
    return n;
}

ASTNode *ast_make_attr_assign(ASTNode *object, const char *attr, ASTNode *value, int line) {
    ASTNode *n = node_new(NODE_ATTR_ASSIGN, line);
    n->attr_assign.object = object;
    n->attr_assign.attr = azr_strdup(attr);
    n->attr_assign.value = value;
    return n;
}

ASTNode *ast_make_with(ASTNode *expr, const char *alias, ASTNode *body, int line) {
    ASTNode *n = node_new(NODE_WITH, line);
    n->with_stmt.expr = expr;
    n->with_stmt.alias = alias ? azr_strdup(alias) : NULL;
    n->with_stmt.body = body;
    return n;
}

ASTNode *ast_make_multi_assign(ASTNode **targets, int target_count, ASTNode *value, int line) {
    ASTNode *n = node_new(NODE_MULTI_ASSIGN, line);
    n->multi_assign.targets = targets;
    n->multi_assign.target_count = target_count;
    n->multi_assign.value = value;
    return n;
}

ASTNode *ast_make_lambda(char **params, ASTNode **defaults, int param_count, const char *vararg, ASTNode *body, int line) {
    ASTNode *n = node_new(NODE_LAMBDA, line);
    n->lambda.params = params;
    n->lambda.defaults = defaults;
    n->lambda.param_count = param_count;
    n->lambda.vararg = vararg ? azr_strdup(vararg) : NULL;
    n->lambda.body = body;
    return n;
}

ASTNode *ast_make_ternary(ASTNode *then_expr, ASTNode *condition, ASTNode *else_expr, int line) {
    ASTNode *n = node_new(NODE_TERNARY, line);
    n->ternary.then_expr = then_expr;
    n->ternary.condition = condition;
    n->ternary.else_expr = else_expr;
    return n;
}

ASTNode *ast_make_comprehension(NodeType type, ASTNode *item_expr, ASTNode *value_expr, char **var_names, ASTNode **iterables, ASTNode **conditions, int loop_count, int line) {
    ASTNode *n = node_new(type, line);
    n->comprehension.item_expr = item_expr;
    n->comprehension.value_expr = value_expr;
    n->comprehension.var_names = var_names;
    n->comprehension.iterables = iterables;
    n->comprehension.conditions = conditions;
    n->comprehension.loop_count = loop_count;
    return n;
}

/* Wraps an expression so it can be executed as a statement. */
ASTNode *ast_make_expr_stmt(ASTNode *expr, int line) {
    ASTNode *n = node_new(NODE_EXPR_STMT, line);
    n->expr_stmt.expr = expr;
    return n;
}

/* Frees an array of child nodes. */
static void free_node_array(ASTNode **items, int count) {
    int i = 0;
    for (i = 0; i < count; i++) ast_free(items[i]);
    free(items);
}

/* Recursively frees an AST node and all owned children. */
void ast_free(ASTNode *node) {
    int i = 0;
    if (!node) return;
    switch (node->type) {
        case NODE_PROGRAM:
        case NODE_BLOCK:
            free_node_array(node->block.stmts, node->block.count);
            break;
        case NODE_ASSIGN:
            free(node->assign.name); ast_free(node->assign.value); break;
        case NODE_MULTI_ASSIGN:
            free_node_array(node->multi_assign.targets, node->multi_assign.target_count); ast_free(node->multi_assign.value); break;
        case NODE_AUGMENTED_ASSIGN:
            free(node->aug_assign.name); free(node->aug_assign.op); ast_free(node->aug_assign.value); break;
        case NODE_IF:
            ast_free(node->if_stmt.condition); ast_free(node->if_stmt.then_block);
            for (i = 0; i < node->if_stmt.elif_count; i++) { ast_free(node->if_stmt.elif_conditions[i]); ast_free(node->if_stmt.elif_blocks[i]); }
            free(node->if_stmt.elif_conditions); free(node->if_stmt.elif_blocks); ast_free(node->if_stmt.else_block); break;
        case NODE_WHILE:
            ast_free(node->while_stmt.condition); ast_free(node->while_stmt.body); break;
        case NODE_FOR:
            free(node->for_stmt.var_name); ast_free(node->for_stmt.iterable); ast_free(node->for_stmt.body); break;
        case NODE_FUNCTION_DEF:
            free(node->func_def.name);
            for (i = 0; i < node->func_def.param_count; i++) free(node->func_def.params[i]);
            free(node->func_def.params); free_node_array(node->func_def.defaults, node->func_def.param_count); free(node->func_def.vararg); ast_free(node->func_def.body); break;
        case NODE_RETURN:
            ast_free(node->return_stmt.value); break;
        case NODE_YIELD:
            ast_free(node->yield_stmt.value); break;
        case NODE_CALL:
            ast_free(node->call.callee); free_node_array(node->call.args, node->call.arg_count);
            for (i = 0; i < node->call.arg_count; i++) free(node->call.arg_names ? node->call.arg_names[i] : NULL);
            free(node->call.arg_names); break;
        case NODE_BINARY_OP:
            free(node->binary.op); ast_free(node->binary.left); ast_free(node->binary.right); break;
        case NODE_UNARY_OP:
            free(node->unary.op); ast_free(node->unary.operand); break;
        case NODE_IDENTIFIER:
            free(node->identifier.name); break;
        case NODE_STRING_LITERAL:
            free(node->str_lit.value); break;
        case NODE_LIST_LITERAL:
            free_node_array(node->list_lit.items, node->list_lit.count); break;
        case NODE_TUPLE_LITERAL:
            free_node_array(node->tuple_lit.items, node->tuple_lit.count); break;
        case NODE_DICT_LITERAL:
        case NODE_SET_LITERAL:
            free_node_array(node->dict_lit.keys, node->dict_lit.count); free_node_array(node->dict_lit.values, node->dict_lit.count); break;
        case NODE_INDEX:
            ast_free(node->index.object); ast_free(node->index.index); break;
        case NODE_SLICE:
            ast_free(node->slice.object); ast_free(node->slice.start); ast_free(node->slice.end); break;
        case NODE_INDEX_ASSIGN:
            ast_free(node->index_assign.object); ast_free(node->index_assign.index); ast_free(node->index_assign.value); break;
        case NODE_INDEX_AUGMENTED_ASSIGN:
            ast_free(node->index_aug_assign.object); ast_free(node->index_aug_assign.index); free(node->index_aug_assign.op); ast_free(node->index_aug_assign.value); break;
        case NODE_IMPORT:
            free(node->import.module_name); free(node->import.alias); break;
        case NODE_FROM_IMPORT:
            free(node->from_import.module_name);
            for (i = 0; i < node->from_import.count; i++) { free(node->from_import.names[i]); free(node->from_import.aliases[i]); }
            free(node->from_import.names); free(node->from_import.aliases); break;
        case NODE_TRY_EXCEPT:
            ast_free(node->try_except.try_block);
            for (i = 0; i < node->try_except.except_count; i++) {
                ast_free(node->try_except.exception_types[i]);
                free(node->try_except.exception_vars[i]);
                ast_free(node->try_except.except_blocks[i]);
            }
            free(node->try_except.exception_types);
            free(node->try_except.exception_vars);
            free(node->try_except.except_blocks);
            ast_free(node->try_except.finally_block);
            break;
        case NODE_THROW:
            ast_free(node->throw_stmt.value); break;
        case NODE_CLASS_DEF:
            free(node->class_def.name); ast_free(node->class_def.base_class); ast_free(node->class_def.body); break;
        case NODE_ATTRIBUTE:
            ast_free(node->attribute.object); free(node->attribute.attr); break;
        case NODE_ATTR_ASSIGN:
            ast_free(node->attr_assign.object); free(node->attr_assign.attr); ast_free(node->attr_assign.value); break;
        case NODE_WITH:
            ast_free(node->with_stmt.expr); free(node->with_stmt.alias); ast_free(node->with_stmt.body); break;
        case NODE_LAMBDA:
            for (i = 0; i < node->lambda.param_count; i++) free(node->lambda.params[i]);
            free(node->lambda.params); free_node_array(node->lambda.defaults, node->lambda.param_count); free(node->lambda.vararg); ast_free(node->lambda.body); break;
        case NODE_TERNARY:
            ast_free(node->ternary.then_expr); ast_free(node->ternary.condition); ast_free(node->ternary.else_expr); break;
        case NODE_LIST_COMPREHENSION:
        case NODE_SET_COMPREHENSION:
        case NODE_DICT_COMPREHENSION:
            ast_free(node->comprehension.item_expr);
            ast_free(node->comprehension.value_expr);
            for (i = 0; i < node->comprehension.loop_count; i++) {
                free(node->comprehension.var_names[i]);
                ast_free(node->comprehension.iterables[i]);
                ast_free(node->comprehension.conditions[i]);
            }
            free(node->comprehension.var_names);
            free(node->comprehension.iterables);
            free(node->comprehension.conditions);
            break;
        case NODE_EXPR_STMT:
            ast_free(node->expr_stmt.expr); break;
        default:
            break;
    }
    free(node);
}

/* Prints indentation spaces for AST debug output. */
static void pindent(int indent) {
    int i = 0;
    for (i = 0; i < indent; i++) printf("  ");
}

/* Prints a readable debug representation of an AST. */
void ast_print(ASTNode *node, int indent) {
    int i = 0;
    if (!node) return;
    pindent(indent);
    switch (node->type) {
        case NODE_PROGRAM: printf("PROGRAM\n"); for (i = 0; i < node->block.count; i++) ast_print(node->block.stmts[i], indent + 1); break;
        case NODE_BLOCK: printf("BLOCK\n"); for (i = 0; i < node->block.count; i++) ast_print(node->block.stmts[i], indent + 1); break;
        case NODE_ASSIGN: printf("ASSIGN %s\n", node->assign.name); ast_print(node->assign.value, indent + 1); break;
        case NODE_MULTI_ASSIGN: printf("MULTI_ASSIGN\n"); ast_print(node->multi_assign.value, indent + 1); break;
        case NODE_AUGMENTED_ASSIGN: printf("AUG_ASSIGN %s %s\n", node->aug_assign.name, node->aug_assign.op); ast_print(node->aug_assign.value, indent + 1); break;
        case NODE_IF: printf("IF\n"); ast_print(node->if_stmt.condition, indent + 1); ast_print(node->if_stmt.then_block, indent + 1); break;
        case NODE_WHILE: printf("WHILE\n"); ast_print(node->while_stmt.condition, indent + 1); ast_print(node->while_stmt.body, indent + 1); break;
        case NODE_FOR: printf("FOR %s\n", node->for_stmt.var_name); ast_print(node->for_stmt.iterable, indent + 1); ast_print(node->for_stmt.body, indent + 1); break;
        case NODE_FUNCTION_DEF: printf("FUNCTION %s\n", node->func_def.name); ast_print(node->func_def.body, indent + 1); break;
        case NODE_RETURN: printf("RETURN\n"); ast_print(node->return_stmt.value, indent + 1); break;
        case NODE_YIELD: printf("YIELD\n"); ast_print(node->yield_stmt.value, indent + 1); break;
        case NODE_BREAK: printf("BREAK\n"); break;
        case NODE_CONTINUE: printf("CONTINUE\n"); break;
        case NODE_PASS: printf("PASS\n"); break;
        case NODE_CALL: printf("CALL\n"); ast_print(node->call.callee, indent + 1); for (i = 0; i < node->call.arg_count; i++) ast_print(node->call.args[i], indent + 1); break;
        case NODE_BINARY_OP: printf("BINARY %s\n", node->binary.op); ast_print(node->binary.left, indent + 1); ast_print(node->binary.right, indent + 1); break;
        case NODE_UNARY_OP: printf("UNARY %s\n", node->unary.op); ast_print(node->unary.operand, indent + 1); break;
        case NODE_IDENTIFIER: printf("IDENT %s\n", node->identifier.name); break;
        case NODE_INT_LITERAL: printf("INT %lld\n", node->int_lit.value); break;
        case NODE_FLOAT_LITERAL: printf("FLOAT %g\n", node->float_lit.value); break;
        case NODE_STRING_LITERAL: printf("STRING \"%s\"\n", node->str_lit.value); break;
        case NODE_BOOL_LITERAL: printf("BOOL %d\n", node->bool_lit.value); break;
        case NODE_NONE_LITERAL: printf("NONE\n"); break;
        case NODE_LIST_LITERAL: printf("LIST\n"); for (i = 0; i < node->list_lit.count; i++) ast_print(node->list_lit.items[i], indent + 1); break;
        case NODE_TUPLE_LITERAL: printf("TUPLE\n"); for (i = 0; i < node->tuple_lit.count; i++) ast_print(node->tuple_lit.items[i], indent + 1); break;
        case NODE_DICT_LITERAL: printf("DICT\n"); break;
        case NODE_SET_LITERAL: printf("SET\n"); break;
        case NODE_INDEX: printf("INDEX\n"); ast_print(node->index.object, indent + 1); ast_print(node->index.index, indent + 1); break;
        case NODE_SLICE: printf("SLICE\n"); ast_print(node->slice.object, indent + 1); break;
        case NODE_INDEX_ASSIGN: printf("INDEX_ASSIGN\n"); ast_print(node->index_assign.object, indent + 1); ast_print(node->index_assign.index, indent + 1); ast_print(node->index_assign.value, indent + 1); break;
        case NODE_INDEX_AUGMENTED_ASSIGN: printf("INDEX_AUG_ASSIGN %s\n", node->index_aug_assign.op); ast_print(node->index_aug_assign.object, indent + 1); ast_print(node->index_aug_assign.index, indent + 1); ast_print(node->index_aug_assign.value, indent + 1); break;
        case NODE_IMPORT: printf("IMPORT %s\n", node->import.module_name); break;
        case NODE_FROM_IMPORT: printf("FROM_IMPORT %s\n", node->from_import.module_name); break;
        case NODE_TRY_EXCEPT:
            printf("TRY\n");
            ast_print(node->try_except.try_block, indent + 1);
            for (i = 0; i < node->try_except.except_count; i++) {
                pindent(indent + 1);
                printf("EXCEPT\n");
                ast_print(node->try_except.exception_types[i], indent + 2);
                ast_print(node->try_except.except_blocks[i], indent + 2);
            }
            if (node->try_except.finally_block) {
                pindent(indent + 1);
                printf("FINALLY\n");
                ast_print(node->try_except.finally_block, indent + 2);
            }
            break;
        case NODE_THROW: printf("THROW\n"); ast_print(node->throw_stmt.value, indent + 1); break;
        case NODE_CLASS_DEF:
            printf("CLASS %s\n", node->class_def.name);
            if (node->class_def.base_class) ast_print(node->class_def.base_class, indent + 1);
            ast_print(node->class_def.body, indent + 1);
            break;
        case NODE_ATTRIBUTE: printf("ATTR .%s\n", node->attribute.attr); ast_print(node->attribute.object, indent + 1); break;
        case NODE_ATTR_ASSIGN: printf("ATTR_ASSIGN .%s\n", node->attr_assign.attr); break;
        case NODE_WITH: printf("WITH\n"); ast_print(node->with_stmt.expr, indent + 1); ast_print(node->with_stmt.body, indent + 1); break;
        case NODE_LAMBDA: printf("LAMBDA\n"); break;
        case NODE_TERNARY: printf("TERNARY\n"); break;
        case NODE_LIST_COMPREHENSION: printf("LIST_COMP\n"); break;
        case NODE_SET_COMPREHENSION: printf("SET_COMP\n"); break;
        case NODE_DICT_COMPREHENSION: printf("DICT_COMP\n"); break;
        case NODE_EXPR_STMT: printf("EXPR_STMT\n"); ast_print(node->expr_stmt.expr, indent + 1); break;
    }
}
