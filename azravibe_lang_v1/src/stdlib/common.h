#ifndef AZR_STDLIB_COMMON_H
#define AZR_STDLIB_COMMON_H

#include "../environment.h"
#include "../value.h"
#include "../utf8.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static inline void azr_reg(Environment *env, const char *name, Value *(*fn)(Value **, int)) {
    env_set(env, name, make_builtin(name, fn));
}

static inline const char *azr_str_arg(Value **args, int argc, int index, const char *fallback) {
    return index < argc && args[index]->type == VAL_STRING ? args[index]->str_val : fallback;
}

static inline long long azr_int_arg(Value **args, int argc, int index, long long fallback) {
    if (index >= argc) return fallback;
    if (args[index]->type == VAL_INT) return args[index]->int_val;
    if (args[index]->type == VAL_FLOAT) return (long long)args[index]->float_val;
    if (args[index]->type == VAL_BOOL) return args[index]->bool_val;
    if (args[index]->type == VAL_STRING) return strtoll(args[index]->str_val, NULL, 10);
    return fallback;
}

static inline double azr_double_arg(Value **args, int argc, int index, double fallback) {
    if (index >= argc) return fallback;
    if (args[index]->type == VAL_INT) return (double)args[index]->int_val;
    if (args[index]->type == VAL_FLOAT) return args[index]->float_val;
    if (args[index]->type == VAL_STRING) return strtod(args[index]->str_val, NULL);
    return fallback;
}

char *azr_read_process(const char *cmd);
char *azr_shell_quote(const char *s);

#endif
