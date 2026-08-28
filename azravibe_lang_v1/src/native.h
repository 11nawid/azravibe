#ifndef AZR_NATIVE_H
#define AZR_NATIVE_H

#include "environment.h"

int azr_native_path_for_module(const char *root, const char *rel, char **out_path);
int azr_native_load_module(const char *path, Environment *module_env, char **error_out);

#endif
