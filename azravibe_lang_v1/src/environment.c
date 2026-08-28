#include "environment.h"
#include "utf8.h"

#include <stdlib.h>
#include <string.h>

/* Allocates a lexical environment linked to an optional parent scope. */
Environment *env_new(Environment *parent) {
    Environment *e = (Environment *)calloc(1, sizeof(Environment));
    e->capacity = 16;
    e->keys = (char **)calloc((size_t)e->capacity, sizeof(char *));
    e->values = (Value **)calloc((size_t)e->capacity, sizeof(Value *));
    e->parent = parent;
    e->owns_values = 1;
    return e;
}

/* Finds a key index in only the current environment. */
static int env_find_local(Environment *e, const char *key) {
    int i = 0;
    for (i = 0; e && i < e->count; i++) {
        if (strcmp(e->keys[i], key) == 0) return i;
    }
    return -1;
}

/* Sets a variable in the current scope, replacing an existing binding there. */
void env_set(Environment *e, const char *key, Value *val) {
    int idx = env_find_local(e, key);
    if (!e) return;
    if (idx >= 0) {
        if (e->owns_values) value_free(e->values[idx]);
        e->values[idx] = val;
        return;
    }
    if (e->count >= e->capacity) {
        e->capacity *= 2;
        e->keys = (char **)realloc(e->keys, (size_t)e->capacity * sizeof(char *));
        e->values = (Value **)realloc(e->values, (size_t)e->capacity * sizeof(Value *));
    }
    e->keys[e->count] = azr_strdup(key);
    e->values[e->count] = val;
    e->count++;
}

/* Looks up a variable through the current scope and parent chain. */
Value *env_get(Environment *e, const char *key) {
    int idx = -1;
    Environment *cur = e;
    while (cur) {
        idx = env_find_local(cur, key);
        if (idx >= 0) return cur->values[idx];
        cur = cur->parent;
    }
    return NULL;
}

Value *env_get_local(Environment *e, const char *key) {
    int idx = env_find_local(e, key);
    return idx >= 0 ? e->values[idx] : NULL;
}

/* Returns true if a variable exists in this environment chain. */
int env_exists(Environment *e, const char *key) {
    return env_get(e, key) != NULL;
}

/* Sets a binding on the root global scope of an environment chain. */
void env_set_global(Environment *e, const char *key, Value *val) {
    Environment *cur = e;
    if (!cur) return;
    while (cur->parent) cur = cur->parent;
    env_set(cur, key, val);
}

/* Frees the environment keys and, when owned, the values stored in it. */
void env_free(Environment *e) {
    int i = 0;
    if (!e) return;
    for (i = 0; i < e->count; i++) {
        free(e->keys[i]);
        if (e->owns_values) value_free(e->values[i]);
    }
    free(e->keys);
    free(e->values);
    free(e);
}
