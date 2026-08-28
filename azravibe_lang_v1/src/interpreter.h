#ifndef AZR_INTERPRETER_H
#define AZR_INTERPRETER_H

#include "ast.h"
#include "environment.h"

typedef struct {
    char *name;
    char *file;
    int line;
} CallFrame;

typedef struct {
    Environment *global_env;
    int had_error;
    char *error_message;
    Value *current_exception;
    Value *module_cache;
    char *stdlib_path;
    char *current_file;
    char *entry_dir;
    char **import_roots;
    int import_root_count;
    int import_root_cap;
    char *module_name;
    char *module_package;
    int current_line;
    int in_generator;
    CallFrame *frames;
    int frame_count;
    int frame_cap;
} Interpreter;

typedef enum { SIG_NONE, SIG_RETURN, SIG_BREAK, SIG_CONTINUE, SIG_YIELD, SIG_ERROR } Signal;

typedef struct {
    Signal signal;
    Value *value;
} ExecResult;

Interpreter *interpreter_new(void);
void interpreter_set_stdlib_path(Interpreter *i, const char *path);
void interpreter_set_current_file(Interpreter *i, const char *path);
void interpreter_set_main_file(Interpreter *i, const char *path);
void interpreter_add_import_root(Interpreter *i, const char *path);
ExecResult exec_program(Interpreter *i, ASTNode *node);
ExecResult exec_block(Interpreter *i, ASTNode *node, Environment *env);
ExecResult exec_statement(Interpreter *i, ASTNode *node, Environment *env);
ExecResult exec_if(Interpreter *i, ASTNode *node, Environment *env);
ExecResult exec_while(Interpreter *i, ASTNode *node, Environment *env);
ExecResult exec_for(Interpreter *i, ASTNode *node, Environment *env);
ExecResult exec_function_def(Interpreter *i, ASTNode *node, Environment *env);
ExecResult exec_return(Interpreter *i, ASTNode *node, Environment *env);
ExecResult exec_try_except(Interpreter *i, ASTNode *node, Environment *env);
Value *eval_expression(Interpreter *i, ASTNode *node, Environment *env);
Value *eval_binary(Interpreter *i, ASTNode *node, Environment *env);
Value *eval_unary(Interpreter *i, ASTNode *node, Environment *env);
Value *eval_call(Interpreter *i, ASTNode *node, Environment *env);
Value *eval_index(Interpreter *i, ASTNode *node, Environment *env);
int interpreter_try_protocol_string(Value *v, Value **out);
int interpreter_try_protocol_length(Value *v, Value **out);
int interpreter_try_protocol_equals(Value *a, Value *b, int *out, int *handled);
Value *interpreter_builtin_iter(Value *v);
Value *interpreter_builtin_next(Value *v);
Value *interpreter_builtin_exception(const char *class_name, const char *message);
void interpreter_free(Interpreter *i);

#endif
