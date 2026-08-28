#ifndef AZRAVIBE_NATIVE_H
#define AZRAVIBE_NATIVE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Value Value;
typedef struct Environment Environment;

typedef Value *(*AzrNativeFn)(Value **args, int argc);

typedef struct AzrNativeAPI {
    int abi_version;
    Value *(*make_int)(long long v);
    Value *(*make_float)(double v);
    Value *(*make_string)(const char *s);
    Value *(*make_bool)(int v);
    Value *(*make_none)(void);
    Value *(*make_list)(void);
    Value *(*make_dict)(void);
    Value *(*make_builtin)(const char *name, AzrNativeFn fn);
    void (*list_append)(Value *list, Value *item);
    void (*dict_set)(Value *dict, const char *key, Value *val);
    Value *(*dict_get)(Value *dict, const char *key);
    Value *(*value_copy)(Value *v);
    void (*value_free)(Value *v);
    char *(*value_to_string)(Value *v);
    int (*value_is_truthy)(Value *v);
    const char *(*value_type_name)(Value *v);
    long long (*value_as_int)(Value *v);
    double (*value_as_float)(Value *v);
    void (*env_set)(Environment *env, const char *key, Value *val);
    Value *(*env_get)(Environment *env, const char *key);
} AzrNativeAPI;

#define AZR_NATIVE_ABI_VERSION 1

#if defined(_WIN32)
#define AZR_NATIVE_EXPORT __declspec(dllexport)
#else
#define AZR_NATIVE_EXPORT __attribute__((visibility("default")))
#endif

typedef int (*AzrNativeInit)(Environment *module_env, const AzrNativeAPI *api);

#ifdef __cplusplus
}
#endif

#endif
