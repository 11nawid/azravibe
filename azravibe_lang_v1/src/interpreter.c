#include "interpreter.h"
#include "builtins.h"
#include "error.h"
#include "lexer.h"
#include "native.h"
#include "parser.h"
#include "utf8.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Value *lookup_attr_value(Interpreter *i, Value *obj, const char *attr);
static char *format_exception(Value *exc);
static Interpreter *g_active_interpreter = NULL;

typedef enum {
    ITER_LIST = 1,
    ITER_TUPLE,
    ITER_STRING,
    ITER_SET,
    ITER_DICT,
    ITER_BYTES,
    ITER_CUSTOM,
    ITER_GENERATOR
} IteratorKind;

typedef enum {
    GEN_FRAME_BLOCK = 1,
    GEN_FRAME_WHILE,
    GEN_FRAME_FOR
} GeneratorFrameKind;

static Value *call_function_value(Interpreter *i, Value *fn, Value *receiver, ASTNode *call_node, Value **args, int argc, Environment *caller_env);
static Value *call_callable_value(Interpreter *i, Value *callable, Value **args, int argc);
static int value_is_stop_iteration(Interpreter *i, Value *exc);
static Value *iterator_from_value(Interpreter *i, Value *value);
static Value *iterator_next_value_internal(Interpreter *i, Value *iterator, int *stopped);
static Value *generator_next_value_internal(Interpreter *i, Value *generator, int *stopped);

#include "interpreter_runtime.inc"
#include "interpreter_iteration.inc"
#include "interpreter_members.inc"
#include "interpreter_statements.inc"
#include "interpreter_expressions.inc"
