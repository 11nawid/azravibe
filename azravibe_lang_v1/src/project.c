#include "project.h"
#include "utf8.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

#define AZR_PROJECT_FILE "azr.project"
#define AZR_LOCK_FILE "azr.lock"
#define AZR_DEFAULT_PACKAGE_DIR ".azr_env/packages"

static char *project_read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    long len = 0;
    char *buf = NULL;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    rewind(f);
    buf = (char *)malloc((size_t)len + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    fread(buf, 1, (size_t)len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

static int project_file_exists(const char *path) {
    FILE *f = NULL;
    if (!path) return 0;
    f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static int project_is_dir(const char *path) {
    struct stat st;
    if (!path) return 0;
    if (stat(path, &st) != 0) return 0;
    return (st.st_mode & S_IFDIR) != 0;
}

static int project_is_file(const char *path) {
    struct stat st;
    if (!path) return 0;
    if (stat(path, &st) != 0) return 0;
    return (st.st_mode & S_IFREG) != 0;
}

static char *project_join_path(const char *a, const char *b) {
    size_t la = strlen(a ? a : "");
    size_t lb = strlen(b ? b : "");
    int need_sep = la > 0 && a[la - 1] != '/' && a[la - 1] != '\\';
    char *out = (char *)malloc(la + lb + (need_sep ? 2 : 1));
    if (!out) return NULL;
    strcpy(out, a ? a : "");
    if (need_sep) strcat(out, "/");
    strcat(out, b ? b : "");
    return out;
}

static char *project_dirname_dup(const char *path) {
    const char *slash1 = NULL;
    const char *slash2 = NULL;
    const char *slash = NULL;
    size_t len = 0;
    if (!path || !*path) return azr_strdup(".");
    slash1 = strrchr(path, '/');
    slash2 = strrchr(path, '\\');
    slash = slash1;
    if (!slash || (slash2 && slash2 > slash)) slash = slash2;
    if (!slash) return azr_strdup(".");
    len = (size_t)(slash - path);
    if (len == 0) return azr_strdup(".");
    return azr_strndup(path, (int)len);
}

static char *project_basename_dup(const char *path) {
    const char *slash1 = NULL;
    const char *slash2 = NULL;
    const char *slash = NULL;
    const char *base = path ? path : "";
    slash1 = strrchr(base, '/');
    slash2 = strrchr(base, '\\');
    slash = slash1;
    if (!slash || (slash2 && slash2 > slash)) slash = slash2;
    return azr_strdup(slash ? slash + 1 : base);
}

static char *project_stem_dup(const char *path) {
    char *base = project_basename_dup(path);
    char *dot = strrchr(base, '.');
    if (dot) *dot = '\0';
    return base;
}

static int project_is_abs(const char *path) {
    if (!path || !*path) return 0;
    if (path[0] == '/' || path[0] == '\\') return 1;
    if (((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) && path[1] == ':') return 1;
    return 0;
}

static char *project_resolve_path(const char *base_dir, const char *path) {
    if (!path || !*path) return azr_strdup(base_dir ? base_dir : ".");
    if (project_is_abs(path)) return azr_strdup(path);
    return project_join_path(base_dir ? base_dir : ".", path);
}

static char *project_trim_dup(const char *text) {
    const char *start = text ? text : "";
    const char *end = start + strlen(start);
    while (*start && isspace((unsigned char)*start)) start++;
    while (end > start && isspace((unsigned char)end[-1])) end--;
    return azr_strndup(start, (int)(end - start));
}

static char *project_parse_value(const char *text) {
    char *trimmed = project_trim_dup(text);
    size_t len = strlen(trimmed);
    if (len >= 2 && ((trimmed[0] == '"' && trimmed[len - 1] == '"') || (trimmed[0] == '\'' && trimmed[len - 1] == '\''))) {
        char *inner = azr_strndup(trimmed + 1, (int)len - 2);
        free(trimmed);
        return inner;
    }
    return trimmed;
}

static void project_add_path(char ***items, int *count, const char *path) {
    int idx = 0;
    if (!path || !*path) return;
    for (idx = 0; idx < *count; idx++) {
        if (strcmp((*items)[idx], path) == 0) return;
    }
    *items = (char **)realloc(*items, (size_t)(*count + 1) * sizeof(char *));
    (*items)[*count] = azr_strdup(path);
    (*count)++;
}

static void project_set_dependency(AzrProjectConfig *config, const char *name, const char *version, const char *source_path) {
    int idx = 0;
    for (idx = 0; idx < config->dependency_count; idx++) {
        if (strcmp(config->dependencies[idx].name, name) == 0) {
            free(config->dependencies[idx].version);
            free(config->dependencies[idx].source_path);
            config->dependencies[idx].version = azr_strdup(version ? version : "0.0.0");
            config->dependencies[idx].source_path = azr_strdup(source_path ? source_path : "");
            return;
        }
    }
    config->dependencies = (AzrProjectDependency *)realloc(config->dependencies, (size_t)(config->dependency_count + 1) * sizeof(AzrProjectDependency));
    config->dependencies[config->dependency_count].name = azr_strdup(name ? name : "");
    config->dependencies[config->dependency_count].version = azr_strdup(version ? version : "0.0.0");
    config->dependencies[config->dependency_count].source_path = azr_strdup(source_path ? source_path : "");
    config->dependency_count++;
}

static int project_mkdir_one(const char *path) {
#ifdef _WIN32
    return _mkdir(path);
#else
    return mkdir(path, 0777);
#endif
}

static int project_ensure_dir(const char *path) {
    char *copy = NULL;
    size_t idx = 0;
    if (!path || !*path) return 0;
    if (project_is_dir(path)) return 1;
    copy = azr_strdup(path);
    for (idx = 1; copy[idx]; idx++) {
        if (copy[idx] == '/' || copy[idx] == '\\') {
            char saved = copy[idx];
            copy[idx] = '\0';
            if (*copy && !project_is_dir(copy) && project_mkdir_one(copy) != 0 && errno != EEXIST) {
                free(copy);
                return 0;
            }
            copy[idx] = saved;
        }
    }
    if (!project_is_dir(copy) && project_mkdir_one(copy) != 0 && errno != EEXIST) {
        free(copy);
        return 0;
    }
    free(copy);
    return 1;
}

static int project_copy_file(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb");
    FILE *out = NULL;
    char buf[8192];
    size_t n = 0;
    char *parent = NULL;
    if (!in) return 0;
    parent = project_dirname_dup(dst);
    if (!project_ensure_dir(parent)) {
        free(parent);
        fclose(in);
        return 0;
    }
    free(parent);
    out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return 0;
    }
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0) fwrite(buf, 1, n, out);
    fclose(out);
    fclose(in);
    return 1;
}

static int project_remove_path(const char *path);

static int project_copy_dir(const char *src, const char *dst) {
    if (!project_ensure_dir(dst)) return 0;
#ifdef _WIN32
    {
        char pattern[1024];
        struct _finddata_t data;
        intptr_t h = -1;
        snprintf(pattern, sizeof(pattern), "%s\\*", src);
        h = _findfirst(pattern, &data);
        if (h == -1) return 0;
        do {
            char *src_child = NULL;
            char *dst_child = NULL;
            int ok = 1;
            if (strcmp(data.name, ".") == 0 || strcmp(data.name, "..") == 0) continue;
            src_child = project_join_path(src, data.name);
            dst_child = project_join_path(dst, data.name);
            if (data.attrib & _A_SUBDIR) ok = project_copy_dir(src_child, dst_child);
            else ok = project_copy_file(src_child, dst_child);
            free(src_child);
            free(dst_child);
            if (!ok) {
                _findclose(h);
                return 0;
            }
        } while (_findnext(h, &data) == 0);
        _findclose(h);
    }
#else
    {
        DIR *dir = opendir(src);
        struct dirent *ent = NULL;
        if (!dir) return 0;
        while ((ent = readdir(dir)) != NULL) {
            char *src_child = NULL;
            char *dst_child = NULL;
            int ok = 1;
            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
            src_child = project_join_path(src, ent->d_name);
            dst_child = project_join_path(dst, ent->d_name);
            if (project_is_dir(src_child)) ok = project_copy_dir(src_child, dst_child);
            else ok = project_copy_file(src_child, dst_child);
            free(src_child);
            free(dst_child);
            if (!ok) {
                closedir(dir);
                return 0;
            }
        }
        closedir(dir);
    }
#endif
    return 1;
}

static int project_remove_path(const char *path) {
    if (!path || !*path) return 1;
    if (project_is_file(path)) return remove(path) == 0 || errno == ENOENT;
    if (!project_is_dir(path)) return 1;
#ifdef _WIN32
    {
        char pattern[1024];
        struct _finddata_t data;
        intptr_t h = -1;
        snprintf(pattern, sizeof(pattern), "%s\\*", path);
        h = _findfirst(pattern, &data);
        if (h != -1) {
            do {
                char *child = NULL;
                if (strcmp(data.name, ".") == 0 || strcmp(data.name, "..") == 0) continue;
                child = project_join_path(path, data.name);
                if (!project_remove_path(child)) {
                    free(child);
                    _findclose(h);
                    return 0;
                }
                free(child);
            } while (_findnext(h, &data) == 0);
            _findclose(h);
        }
        return _rmdir(path) == 0 || errno == ENOENT;
    }
#else
    {
        DIR *dir = opendir(path);
        struct dirent *ent = NULL;
        if (dir) {
            while ((ent = readdir(dir)) != NULL) {
                char *child = NULL;
                if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
                child = project_join_path(path, ent->d_name);
                if (!project_remove_path(child)) {
                    free(child);
                    closedir(dir);
                    return 0;
                }
                free(child);
            }
            closedir(dir);
        }
        return rmdir(path) == 0 || errno == ENOENT;
    }
#endif
}

static int project_write_manifest(const AzrProjectConfig *config) {
    char *path = project_join_path(config->root_dir, AZR_PROJECT_FILE);
    FILE *f = fopen(path, "wb");
    int idx = 0;
    if (!f) {
        free(path);
        return 0;
    }
    fprintf(f, "name = \"%s\"\n", config->name ? config->name : "");
    fprintf(f, "version = \"%s\"\n", config->version ? config->version : "0.1.0");
    fprintf(f, "package_dir = \"%s\"\n", config->package_dir ? config->package_dir : AZR_DEFAULT_PACKAGE_DIR);
    fprintf(f, "path = \"%s\"\n", config->package_dir ? config->package_dir : AZR_DEFAULT_PACKAGE_DIR);
    for (idx = 0; idx < config->extra_path_count; idx++) {
        if (config->package_dir && strcmp(config->extra_paths[idx], config->package_dir) == 0) continue;
        fprintf(f, "path = \"%s\"\n", config->extra_paths[idx]);
    }
    for (idx = 0; idx < config->dependency_count; idx++) {
        fprintf(f, "dependency = \"%s|%s|%s\"\n",
            config->dependencies[idx].name ? config->dependencies[idx].name : "",
            config->dependencies[idx].version ? config->dependencies[idx].version : "0.0.0",
            config->dependencies[idx].source_path ? config->dependencies[idx].source_path : "");
    }
    fclose(f);
    free(path);
    return 1;
}

static int project_write_lock(const AzrProjectConfig *config) {
    char *path = project_join_path(config->root_dir, AZR_LOCK_FILE);
    char *pkg_root = project_resolve_path(config->root_dir, config->package_dir ? config->package_dir : AZR_DEFAULT_PACKAGE_DIR);
    FILE *f = fopen(path, "wb");
    int idx = 0;
    if (!f) {
        free(pkg_root);
        free(path);
        return 0;
    }
    fprintf(f, "name = \"%s\"\n", config->name ? config->name : "");
    fprintf(f, "version = \"%s\"\n", config->version ? config->version : "0.1.0");
    for (idx = 0; idx < config->dependency_count; idx++) {
        char *installed = project_join_path(pkg_root, config->dependencies[idx].name ? config->dependencies[idx].name : "");
        fprintf(f, "installed = \"%s|%s|%s|%s\"\n",
            config->dependencies[idx].name ? config->dependencies[idx].name : "",
            config->dependencies[idx].version ? config->dependencies[idx].version : "0.0.0",
            config->dependencies[idx].source_path ? config->dependencies[idx].source_path : "",
            installed ? installed : "");
        free(installed);
    }
    fclose(f);
    free(pkg_root);
    free(path);
    return 1;
}

static int project_parse_dependency(AzrProjectConfig *config, const char *value) {
    char *copy = azr_strdup(value ? value : "");
    char *name = copy;
    char *version = strchr(copy, '|');
    char *source = NULL;
    if (!version) {
        free(copy);
        return 0;
    }
    *version++ = '\0';
    source = strchr(version, '|');
    if (!source) {
        free(copy);
        return 0;
    }
    *source++ = '\0';
    project_set_dependency(config, name, version, source);
    free(copy);
    return 1;
}

static int project_load_from_manifest(const char *manifest_path, const char *root_dir, AzrProjectConfig *out) {
    char *source = project_read_file(manifest_path);
    char *cursor = source;
    if (!source) return 0;
    memset(out, 0, sizeof(*out));
    out->root_dir = azr_strdup(root_dir);
    out->version = azr_strdup("0.1.0");
    out->package_dir = azr_strdup(AZR_DEFAULT_PACKAGE_DIR);
    while (cursor && *cursor) {
        char *line_end = strchr(cursor, '\n');
        char *line = NULL;
        char *eq = NULL;
        char *key = NULL;
        char *value = NULL;
        if (line_end) {
            line = azr_strndup(cursor, (int)(line_end - cursor));
            cursor = line_end + 1;
        } else {
            line = azr_strdup(cursor);
            cursor += strlen(cursor);
        }
        key = project_trim_dup(line);
        free(line);
        if (!*key || key[0] == '#') {
            free(key);
            continue;
        }
        eq = strchr(key, '=');
        if (!eq) {
            free(key);
            continue;
        }
        *eq = '\0';
        value = project_parse_value(eq + 1);
        {
            char *trimmed_key = project_trim_dup(key);
            free(key);
            key = trimmed_key;
        }
        if (strcmp(key, "name") == 0) {
            free(out->name);
            out->name = azr_strdup(value);
        } else if (strcmp(key, "version") == 0) {
            free(out->version);
            out->version = azr_strdup(value);
        } else if (strcmp(key, "package_dir") == 0) {
            free(out->package_dir);
            out->package_dir = azr_strdup(value);
        } else if (strcmp(key, "path") == 0) {
            project_add_path(&out->extra_paths, &out->extra_path_count, value);
        } else if (strcmp(key, "dependency") == 0) {
            project_parse_dependency(out, value);
        }
        free(key);
        free(value);
    }
    if (!out->name) out->name = project_basename_dup(root_dir);
    if (out->extra_path_count == 0) project_add_path(&out->extra_paths, &out->extra_path_count, out->package_dir);
    free(source);
    return 1;
}

static int project_find_manifest(const char *start_dir, char **manifest_out, char **root_out) {
    char *current = project_resolve_path(".", start_dir ? start_dir : ".");
    while (current && *current) {
        char *manifest = project_join_path(current, AZR_PROJECT_FILE);
        if (project_file_exists(manifest)) {
            *manifest_out = manifest;
            *root_out = current;
            return 1;
        }
        free(manifest);
        {
            char *parent = project_dirname_dup(current);
            if (strcmp(parent, current) == 0 || (strcmp(current, ".") == 0 && strcmp(parent, ".") == 0)) {
                free(parent);
                break;
            }
            free(current);
            current = parent;
        }
    }
    free(current);
    return 0;
}

int azr_project_load(const char *start_dir, AzrProjectConfig *out) {
    char *manifest = NULL;
    char *root = NULL;
    int ok = project_find_manifest(start_dir, &manifest, &root);
    if (!ok) return 0;
    ok = project_load_from_manifest(manifest, root, out);
    free(manifest);
    free(root);
    return ok;
}

int azr_project_collect_import_roots(const char *start_dir, char ***roots_out, int *count_out) {
    AzrProjectConfig config;
    int idx = 0;
    memset(&config, 0, sizeof(config));
    *roots_out = NULL;
    *count_out = 0;
    if (!azr_project_load(start_dir, &config)) return 0;
    {
        char *pkg_root = project_resolve_path(config.root_dir, config.package_dir ? config.package_dir : AZR_DEFAULT_PACKAGE_DIR);
        project_add_path(roots_out, count_out, pkg_root);
        free(pkg_root);
    }
    for (idx = 0; idx < config.extra_path_count; idx++) {
        char *resolved = project_resolve_path(config.root_dir, config.extra_paths[idx]);
        project_add_path(roots_out, count_out, resolved);
        free(resolved);
    }
    azr_project_free(&config);
    return *count_out > 0;
}

int azr_project_init(const char *project_dir, const char *name, const char *version, char **error_out) {
    AzrProjectConfig config;
    char *pkg_root = NULL;
    memset(&config, 0, sizeof(config));
    config.root_dir = project_resolve_path(".", project_dir ? project_dir : ".");
    config.name = azr_strdup(name && *name ? name : "azr_project");
    config.version = azr_strdup(version && *version ? version : "0.1.0");
    config.package_dir = azr_strdup(AZR_DEFAULT_PACKAGE_DIR);
    project_add_path(&config.extra_paths, &config.extra_path_count, config.package_dir);
    if (!project_ensure_dir(config.root_dir)) {
        if (error_out) *error_out = azr_strdup("ساختن پوشه پروژه ممکن نشد");
        azr_project_free(&config);
        return 0;
    }
    pkg_root = project_resolve_path(config.root_dir, config.package_dir);
    if (!project_ensure_dir(pkg_root)) {
        if (error_out) *error_out = azr_strdup("ساختن پوشه محیط محلی ممکن نشد");
        free(pkg_root);
        azr_project_free(&config);
        return 0;
    }
    free(pkg_root);
    if (!project_write_manifest(&config) || !project_write_lock(&config)) {
        if (error_out) *error_out = azr_strdup("نوشتن فایل‌های پروژه ممکن نشد");
        azr_project_free(&config);
        return 0;
    }
    azr_project_free(&config);
    return 1;
}

static int project_detect_source(const char *source_path, char **name_out, char **version_out, char **payload_path_out, int *is_dir_out) {
    char *abs_source = project_resolve_path(".", source_path);
    char *manifest = NULL;
    *name_out = NULL;
    *version_out = NULL;
    *payload_path_out = NULL;
    *is_dir_out = 0;
    if (project_is_dir(abs_source)) {
        AzrProjectConfig dep_config;
        memset(&dep_config, 0, sizeof(dep_config));
        manifest = project_join_path(abs_source, AZR_PROJECT_FILE);
        if (project_file_exists(manifest) && project_load_from_manifest(manifest, abs_source, &dep_config)) {
            *name_out = azr_strdup(dep_config.name ? dep_config.name : "");
            *version_out = azr_strdup(dep_config.version ? dep_config.version : "0.0.0");
            azr_project_free(&dep_config);
        } else {
            char *init_path = project_join_path(abs_source, "__init__.azr");
            if (!project_file_exists(init_path)) {
                free(init_path);
                free(manifest);
                free(abs_source);
                return 0;
            }
            free(init_path);
            *name_out = project_basename_dup(abs_source);
            *version_out = azr_strdup("0.0.0");
        }
        *payload_path_out = abs_source;
        *is_dir_out = 1;
        free(manifest);
        return 1;
    }
    free(manifest);
    if (!project_is_file(abs_source)) {
        free(abs_source);
        return 0;
    }
    *name_out = project_stem_dup(abs_source);
    *version_out = azr_strdup("0.0.0");
    *payload_path_out = abs_source;
    *is_dir_out = 0;
    return 1;
}

int azr_project_install_local(const char *project_dir, const char *source_path, char **error_out) {
    AzrProjectConfig config;
    char *project_root = project_resolve_path(".", project_dir ? project_dir : ".");
    char *manifest = project_join_path(project_root, AZR_PROJECT_FILE);
    char *name = NULL;
    char *version = NULL;
    char *payload = NULL;
    char *pkg_root = NULL;
    char *source_record = NULL;
    int is_dir = 0;
    int ok = 0;
    memset(&config, 0, sizeof(config));
    if (!project_file_exists(manifest)) {
        char *default_name = project_basename_dup(project_root);
        if (!azr_project_init(project_root, default_name, "0.1.0", error_out)) {
            free(default_name);
            free(project_root);
            free(manifest);
            return 0;
        }
        free(default_name);
    }
    if (!azr_project_load(project_root, &config)) {
        if (error_out) *error_out = azr_strdup("خواندن فایل پروژه ممکن نشد");
        free(project_root);
        free(manifest);
        return 0;
    }
    if (!project_detect_source(source_path, &name, &version, &payload, &is_dir)) {
        if (error_out) *error_out = azr_strdup("بسته محلی معتبر نیست");
        azr_project_free(&config);
        free(project_root);
        free(manifest);
        return 0;
    }
    pkg_root = project_resolve_path(config.root_dir, config.package_dir ? config.package_dir : AZR_DEFAULT_PACKAGE_DIR);
    if (!project_ensure_dir(pkg_root)) {
        if (error_out) *error_out = azr_strdup("ساختن پوشه بسته‌های محلی ممکن نشد");
        azr_project_free(&config);
        free(project_root);
        free(manifest);
        free(name);
        free(version);
        free(payload);
        free(pkg_root);
        return 0;
    }
    source_record = project_resolve_path(".", source_path);
    if (is_dir) {
        char *dst = project_join_path(pkg_root, name);
        project_remove_path(dst);
        ok = project_copy_dir(payload, dst);
        free(dst);
    } else {
        char *file_name = (char *)malloc(strlen(name) + 5);
        char *dst = NULL;
        sprintf(file_name, "%s.azr", name);
        dst = project_join_path(pkg_root, file_name);
        ok = project_copy_file(payload, dst);
        free(dst);
        free(file_name);
    }
    if (!ok) {
        if (error_out) *error_out = azr_strdup("کپی بسته محلی ناموفق بود");
        azr_project_free(&config);
        free(project_root);
        free(manifest);
        free(name);
        free(version);
        free(payload);
        free(pkg_root);
        free(source_record);
        return 0;
    }
    project_add_path(&config.extra_paths, &config.extra_path_count, config.package_dir ? config.package_dir : AZR_DEFAULT_PACKAGE_DIR);
    project_set_dependency(&config, name, version, source_record);
    if (!project_write_manifest(&config) || !project_write_lock(&config)) {
        if (error_out) *error_out = azr_strdup("به‌روزرسانی فایل پروژه ممکن نشد");
        azr_project_free(&config);
        free(project_root);
        free(manifest);
        free(name);
        free(version);
        free(payload);
        free(pkg_root);
        free(source_record);
        return 0;
    }
    azr_project_free(&config);
    free(project_root);
    free(manifest);
    free(name);
    free(version);
    free(payload);
    free(pkg_root);
    free(source_record);
    return 1;
}

void azr_project_free(AzrProjectConfig *config) {
    int idx = 0;
    if (!config) return;
    free(config->root_dir);
    free(config->name);
    free(config->version);
    free(config->package_dir);
    for (idx = 0; idx < config->extra_path_count; idx++) free(config->extra_paths[idx]);
    free(config->extra_paths);
    for (idx = 0; idx < config->dependency_count; idx++) {
        free(config->dependencies[idx].name);
        free(config->dependencies[idx].version);
        free(config->dependencies[idx].source_path);
    }
    free(config->dependencies);
    memset(config, 0, sizeof(*config));
}
