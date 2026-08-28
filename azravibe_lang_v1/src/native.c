#include "native.h"
#include "azravibe_native.h"
#include "utf8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

typedef struct NativeHandle {
    void *handle;
    struct NativeHandle *next;
} NativeHandle;

static NativeHandle *g_native_handles = NULL;

static void keep_handle(void *handle) {
    NativeHandle *item = NULL;
    if (!handle) return;
    item = (NativeHandle *)calloc(1, sizeof(NativeHandle));
    if (!item) return;
    item->handle = handle;
    item->next = g_native_handles;
    g_native_handles = item;
}

static int file_exists_local(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static char *dup_msg(const char *msg) {
    return azr_strdup(msg ? msg : "native module error");
}

static long long native_value_as_int(Value *v) {
    if (!v) return 0;
    if (v->type == VAL_INT) return v->int_val;
    if (v->type == VAL_FLOAT) return (long long)v->float_val;
    if (v->type == VAL_BOOL) return v->bool_val ? 1 : 0;
    return 0;
}

static double native_value_as_float(Value *v) {
    if (!v) return 0.0;
    if (v->type == VAL_FLOAT) return v->float_val;
    if (v->type == VAL_INT) return (double)v->int_val;
    if (v->type == VAL_BOOL) return v->bool_val ? 1.0 : 0.0;
    return 0.0;
}

static const AzrNativeAPI *native_api(void) {
    static AzrNativeAPI api;
    static int initialized = 0;
    if (!initialized) {
        api.abi_version = AZR_NATIVE_ABI_VERSION;
        api.make_int = make_int;
        api.make_float = make_float;
        api.make_string = make_string;
        api.make_bool = make_bool;
        api.make_none = make_none;
        api.make_list = make_list;
        api.make_dict = make_dict;
        api.make_builtin = make_builtin;
        api.list_append = list_append;
        api.dict_set = dict_set;
        api.dict_get = dict_get;
        api.value_copy = value_copy;
        api.value_free = value_free;
        api.value_to_string = value_to_string;
        api.value_is_truthy = value_is_truthy;
        api.value_type_name = value_type_name;
        api.value_as_int = native_value_as_int;
        api.value_as_float = native_value_as_float;
        api.env_set = env_set;
        api.env_get = env_get;
        initialized = 1;
    }
    return &api;
}

int azr_native_path_for_module(const char *root, const char *rel, char **out_path) {
    char path[1024];
    static const char *exts[] = {
#ifdef _WIN32
        ".dll",
#elif defined(__APPLE__)
        ".dylib",
        ".so",
#else
        ".so",
#endif
        NULL
    };
    int idx = 0;
    if (!root || !*root || !rel || !*rel || !out_path) return 0;
    for (idx = 0; exts[idx]; idx++) {
        snprintf(path, sizeof(path), "%s/%s%s", root, rel, exts[idx]);
        if (file_exists_local(path)) {
            *out_path = azr_strdup(path);
            return 1;
        }
    }
    return 0;
}

int azr_native_load_module(const char *path, Environment *module_env, char **error_out) {
    void *handle = NULL;
    AzrNativeInit init = NULL;
    int ok = 0;
    if (error_out) *error_out = NULL;
    if (!path || !*path || !module_env) {
        if (error_out) *error_out = dup_msg("native module path or environment is missing");
        return 0;
    }
#ifdef _WIN32
    handle = (void *)LoadLibraryA(path);
    if (!handle) {
        if (error_out) *error_out = dup_msg("native module could not be loaded");
        return 0;
    }
    init = (AzrNativeInit)(void *)GetProcAddress((HMODULE)handle, "azr_module_init");
    if (!init) {
        if (error_out) *error_out = dup_msg("native module is missing azr_module_init");
        FreeLibrary((HMODULE)handle);
        return 0;
    }
#else
    handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (!handle) {
        if (error_out) *error_out = dup_msg(dlerror());
        return 0;
    }
    init = (AzrNativeInit)dlsym(handle, "azr_module_init");
    if (!init) {
        if (error_out) *error_out = dup_msg(dlerror());
        dlclose(handle);
        return 0;
    }
#endif
    ok = init(module_env, native_api());
    if (!ok) {
        if (error_out) *error_out = dup_msg("native module initialization failed");
#ifdef _WIN32
        FreeLibrary((HMODULE)handle);
#else
        dlclose(handle);
#endif
        return 0;
    }
    keep_handle(handle);
    return 1;
}
