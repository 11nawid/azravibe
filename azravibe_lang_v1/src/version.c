#include "version.h"
#include "utf8.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *g_azr_version = NULL;

static char *version_dirname(const char *path) {
    const char *slash1 = NULL;
    const char *slash2 = NULL;
    const char *slash = NULL;
    size_t len = 0;
    if (!path || !*path) return NULL;
    slash1 = strrchr(path, '/');
    slash2 = strrchr(path, '\\');
    slash = slash1;
    if (!slash || (slash2 && slash2 > slash)) slash = slash2;
    if (!slash) return NULL;
    len = (size_t)(slash - path);
    if (len == 0) return azr_strdup(".");
    return azr_strndup(path, (int)len);
}

static char *join_version_path(const char *dir, const char *name) {
    const char *separator = "/";
    size_t len = 0;
    char *out = NULL;
    if (!dir || !*dir) return azr_strdup(name);
    if (strchr(dir, '\\')) separator = "\\";
    len = strlen(dir) + strlen(separator) + strlen(name) + 1;
    out = (char *)malloc(len);
    if (!out) return NULL;
    snprintf(out, len, "%s%s%s", dir, separator, name);
    return out;
}

static char *trimmed_version_line(char *line) {
    char *start = line;
    char *end = NULL;
    while (*start && isspace((unsigned char)*start)) start++;
    if (!*start) return NULL;
    end = start + strlen(start);
    while (end > start && isspace((unsigned char)end[-1])) end--;
    *end = '\0';
    return *start ? azr_strdup(start) : NULL;
}

static char *read_version_from_file(const char *path) {
    FILE *file = NULL;
    char buffer[256];
    char *result = NULL;
    if (!path) return NULL;
    file = fopen(path, "rb");
    if (!file) return NULL;
    while (fgets(buffer, sizeof(buffer), file)) {
        result = trimmed_version_line(buffer);
        if (result) break;
    }
    fclose(file);
    return result;
}

void azr_version_init(const char *argv0) {
    char *exe_dir = NULL;
    char *version_path = NULL;
    char *loaded = NULL;
    azr_version_shutdown();
    exe_dir = version_dirname(argv0);
    if (exe_dir) {
        version_path = join_version_path(exe_dir, "versions.txt");
        loaded = read_version_from_file(version_path);
        free(version_path);
        free(exe_dir);
    }
    if (!loaded) loaded = read_version_from_file("versions.txt");
    g_azr_version = loaded ? loaded : azr_strdup(AZR_DEFAULT_VERSION);
}

const char *azr_version_get(void) {
    return g_azr_version ? g_azr_version : AZR_DEFAULT_VERSION;
}

void azr_version_shutdown(void) {
    free(g_azr_version);
    g_azr_version = NULL;
}
