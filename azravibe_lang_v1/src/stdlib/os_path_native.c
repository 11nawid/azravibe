#include "common.h"
#include "../version.h"

#include <errno.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#include <dirent.h>
#define getcwd _getcwd
#define chdir _chdir
#define mkdir_one(p) _mkdir(p)
#define rmdir_one(p) _rmdir(p)
#else
#include <dirent.h>
#include <unistd.h>
#define mkdir_one(p) mkdir(p, 0755)
#define rmdir_one(p) rmdir(p)
#endif

static Value *b_os_getcwd(Value **args, int argc) {
    char buf[2048]; (void)args; (void)argc;
    return getcwd(buf, sizeof(buf)) ? make_string(buf) : make_string("");
}

static Value *b_os_chdir(Value **args, int argc) {
    return make_bool(chdir(azr_str_arg(args, argc, 0, ".")) == 0);
}

static Value *b_os_mkdir(Value **args, int argc) {
    return make_bool(mkdir_one(azr_str_arg(args, argc, 0, "")) == 0 || errno == EEXIST);
}

static Value *b_os_remove(Value **args, int argc) {
    return make_bool(remove(azr_str_arg(args, argc, 0, "")) == 0);
}

static Value *b_os_rmdir(Value **args, int argc) {
    return make_bool(rmdir_one(azr_str_arg(args, argc, 0, "")) == 0);
}

static Value *b_os_rename(Value **args, int argc) {
    return make_bool(rename(azr_str_arg(args, argc, 0, ""), azr_str_arg(args, argc, 1, "")) == 0);
}

static Value *b_os_getenv(Value **args, int argc) {
    const char *v = getenv(azr_str_arg(args, argc, 0, ""));
    return v ? make_string(v) : make_none();
}

static Value *b_os_setenv(Value **args, int argc) {
    const char *key = azr_str_arg(args, argc, 0, "");
    const char *val = azr_str_arg(args, argc, 1, "");
#ifdef _WIN32
    return make_bool(_putenv_s(key, val) == 0);
#else
    return make_bool(setenv(key, val, 1) == 0);
#endif
}

static Value *b_path_basename(Value **args, int argc) {
    const char *p = azr_str_arg(args, argc, 0, "");
    const char *a = strrchr(p, '/'), *b = strrchr(p, '\\'), *m = a > b ? a : b;
    return make_string(m ? m + 1 : p);
}

static Value *b_path_dirname(Value **args, int argc) {
    const char *p = azr_str_arg(args, argc, 0, "");
    const char *a = strrchr(p, '/'), *b = strrchr(p, '\\'), *m = a > b ? a : b;
    return m ? make_string(azr_strndup(p, (int)(m - p))) : make_string(".");
}

static Value *b_path_ext(Value **args, int argc) {
    const char *p = azr_str_arg(args, argc, 0, "");
    const char *base = strrchr(p, '/'), *dot = strrchr(p, '.');
    if (!base) base = strrchr(p, '\\');
    return dot && (!base || dot > base) ? make_string(dot) : make_string("");
}

static Value *b_path_splitext(Value **args, int argc) {
    const char *p = azr_str_arg(args, argc, 0, "");
    const char *base = strrchr(p, '/'), *dot = strrchr(p, '.');
    Value *out = make_list();
    char *root = NULL;
    if (!base) base = strrchr(p, '\\');
    if (dot && (!base || dot > base)) {
        root = azr_strndup(p, (int)(dot - p));
        list_append(out, make_string(root));
        list_append(out, make_string(dot));
        free(root);
    } else {
        list_append(out, make_string(p));
        list_append(out, make_string(""));
    }
    return out;
}

static Value *b_path_isabs(Value **args, int argc) {
    const char *p = azr_str_arg(args, argc, 0, "");
    return make_bool(p[0] == '/' || p[0] == '\\' || (isalpha((unsigned char)p[0]) && p[1] == ':'));
}

static Value *b_path_abs(Value **args, int argc) {
    const char *p = azr_str_arg(args, argc, 0, "");
    if (p[0] == '/' || p[0] == '\\' || (isalpha((unsigned char)p[0]) && p[1] == ':')) return make_string(p);
    {
        char cwd[2048], out[4096];
        getcwd(cwd, sizeof(cwd));
        snprintf(out, sizeof(out), "%s/%s", cwd, p);
        return make_string(out);
    }
}

static Value *b_path_norm(Value **args, int argc) {
    char *s = azr_strdup(azr_str_arg(args, argc, 0, ""));
    int i = 0;
    for (i = 0; s[i]; i++) if (s[i] == '\\') s[i] = '/';
    { Value *v = make_string(s); free(s); return v; }
}

static Value *b_path_expanduser(Value **args, int argc) {
    const char *p = azr_str_arg(args, argc, 0, "");
    const char *home = getenv(
#ifdef _WIN32
        "USERPROFILE"
#else
        "HOME"
#endif
    );
    if (p[0] != '~') return make_string(p);
    if (!home) home = "";
    {
        char *out = (char *)calloc(strlen(home) + strlen(p) + 2, 1);
        sprintf(out, "%s%s", home, p + 1);
        Value *v = make_string(out);
        free(out);
        return v;
    }
}

static Value *b_path_split(Value **args, int argc) {
    Value *out = make_list();
    list_append(out, b_path_dirname(args, argc));
    list_append(out, b_path_basename(args, argc));
    return out;
}

static Value *b_sys_platform(Value **args, int argc) {
    (void)args; (void)argc;
#ifdef _WIN32
    return make_string("windows");
#elif __APPLE__
    return make_string("macos");
#elif __linux__
    return make_string("linux");
#else
    return make_string("unknown");
#endif
}

static Value *b_sys_version(Value **args, int argc) {
    (void)args; (void)argc;
    return make_string(azr_version_get());
}

static Value *b_os_stat(Value **args, int argc) {
    struct stat st; Value *out = make_dict();
    const char *p = azr_str_arg(args, argc, 0, "");
    if (stat(p, &st) != 0) return out;
    dict_set(out, "size", make_int((long long)st.st_size));
    dict_set(out, "mtime", make_int((long long)st.st_mtime));
    dict_set(out, "ctime", make_int((long long)st.st_ctime));
    dict_set(out, "isdir", make_bool((st.st_mode & S_IFDIR) != 0));
    dict_set(out, "isfile", make_bool((st.st_mode & S_IFREG) != 0));
    return out;
}

static Value *b_os_getsize(Value **args, int argc) {
    struct stat st; const char *p = azr_str_arg(args, argc, 0, "");
    return stat(p, &st) == 0 ? make_int((long long)st.st_size) : make_int(-1);
}

static Value *b_os_getmtime(Value **args, int argc) {
    struct stat st; const char *p = azr_str_arg(args, argc, 0, "");
    return stat(p, &st) == 0 ? make_int((long long)st.st_mtime) : make_int(-1);
}

static Value *b_os_isfile(Value **args, int argc) {
    struct stat st; const char *p = azr_str_arg(args, argc, 0, "");
    return make_bool(stat(p, &st) == 0 && (st.st_mode & S_IFREG) != 0);
}

static Value *b_os_isdir(Value **args, int argc) {
    struct stat st; const char *p = azr_str_arg(args, argc, 0, "");
    return make_bool(stat(p, &st) == 0 && (st.st_mode & S_IFDIR) != 0);
}

static void walk_collect(Value *out, const char *root, int recursive) {
    DIR *d = opendir(root);
    struct dirent *ent = NULL;
    if (!d) return;
    while ((ent = readdir(d)) != NULL) {
        char child[4096];
        struct stat st;
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
        snprintf(child, sizeof(child), "%s/%s", root, ent->d_name);
        list_append(out, make_string(child));
        if (recursive && stat(child, &st) == 0 && (st.st_mode & S_IFDIR)) walk_collect(out, child, recursive);
    }
    closedir(d);
}

static Value *b_os_walk(Value **args, int argc) {
    Value *out = make_list();
    walk_collect(out, azr_str_arg(args, argc, 0, "."), (int)azr_int_arg(args, argc, 1, 1));
    return out;
}

void azr_register_os_path(Environment *env) {
    azr_reg(env, "__os_getcwd", b_os_getcwd);
    azr_reg(env, "__os_chdir", b_os_chdir);
    azr_reg(env, "__os_mkdir", b_os_mkdir);
    azr_reg(env, "__os_remove", b_os_remove);
    azr_reg(env, "__os_rmdir", b_os_rmdir);
    azr_reg(env, "__os_rename", b_os_rename);
    azr_reg(env, "__os_getenv", b_os_getenv);
    azr_reg(env, "__os_setenv", b_os_setenv);
    azr_reg(env, "__path_basename", b_path_basename);
    azr_reg(env, "__path_dirname", b_path_dirname);
    azr_reg(env, "__path_ext", b_path_ext);
    azr_reg(env, "__path_splitext", b_path_splitext);
    azr_reg(env, "__path_isabs", b_path_isabs);
    azr_reg(env, "__path_split", b_path_split);
    azr_reg(env, "__path_abs", b_path_abs);
    azr_reg(env, "__path_norm", b_path_norm);
    azr_reg(env, "__path_expanduser", b_path_expanduser);
    azr_reg(env, "__sys_platform", b_sys_platform);
    azr_reg(env, "__sys_version", b_sys_version);
    azr_reg(env, "__os_stat", b_os_stat);
    azr_reg(env, "__os_getsize", b_os_getsize);
    azr_reg(env, "__os_getmtime", b_os_getmtime);
    azr_reg(env, "__os_isfile", b_os_isfile);
    azr_reg(env, "__os_isdir", b_os_isdir);
    azr_reg(env, "__os_walk", b_os_walk);
}
