#ifndef AZR_ENVIRONMENT_H
#define AZR_ENVIRONMENT_H

#include "value.h"

typedef struct Environment {
    char **keys;
    Value **values;
    int count;
    int capacity;
    struct Environment *parent;
    int owns_values;
} Environment;

Environment *env_new(Environment *parent);
void env_set(Environment *e, const char *key, Value *val);
Value *env_get(Environment *e, const char *key);
Value *env_get_local(Environment *e, const char *key);
int env_exists(Environment *e, const char *key);
void env_set_global(Environment *e, const char *key, Value *val);
void env_free(Environment *e);

#endif
