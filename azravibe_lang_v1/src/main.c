#include "ast.h"
#include "console.h"
#include "error.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "project.h"
#include "utf8.h"
#include "version.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#endif
#ifndef _WIN32
#include <dirent.h>
#endif

/* Returns a heap path to the stdlib folder next to the executable. */
static char *stdlib_from_argv0(const char *argv0) {
    char *copy = NULL;
    char *slash1 = NULL;
    char *slash2 = NULL;
    size_t len = 0;
    if (!argv0 || !strchr(argv0, '\\')) return NULL;
    copy = (char *)malloc(strlen(argv0) + 16);
    strcpy(copy, argv0);
    slash1 = strrchr(copy, '\\');
    slash2 = strrchr(copy, '/');
    if (!slash1 || (slash2 && slash2 > slash1)) slash1 = slash2;
    if (!slash1) { free(copy); return NULL; }
    *slash1 = '\0';
    len = strlen(copy);
    copy = (char *)realloc(copy, len + strlen("\\stdlib") + 1);
    strcat(copy, "\\stdlib");
    return copy;
}

static char *path_dirname_local(const char *path) {
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

static void configure_project_runtime(Interpreter *interp, const char *start_dir) {
    char **roots = NULL;
    int count = 0;
    int idx = 0;
    if (!interp || !start_dir) return;
    if (!azr_project_collect_import_roots(start_dir, &roots, &count)) return;
    for (idx = 0; idx < count; idx++) {
        interpreter_add_import_root(interp, roots[idx]);
        free(roots[idx]);
    }
    free(roots);
}

/* Reads an entire UTF-8 text file into a null-terminated buffer. */
static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    long len = 0;
    char *buf = NULL;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    rewind(f);
    buf = (char *)malloc((size_t)len + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, (size_t)len, f);
    buf[len] = '\0';
    fclose(f);
    return buf;
}

/* Frees a token vector produced by lexer_tokenize_all. */
static void free_tokens(Token **tokens, int count) {
    int i = 0;
    for (i = 0; i < count; i++) token_free(tokens[i]);
    free(tokens);
}

/* Executes source text through lexing, parsing, and interpreting. */
static int run_source(Interpreter *interp, const char *source, int debug, int print_expr) {
    Lexer *lexer = lexer_new(source);
    int count = 0, i = 0;
    Token **tokens = lexer_tokenize_all(lexer, &count);
    Parser *parser = parser_new(tokens, count);
    ASTNode *program = NULL;
    ExecResult result;
    if (debug) {
        for (i = 0; i < count; i++) azr_console_printf(stdout, "%s('%s') خط %d ستون %d\n", token_type_name(tokens[i]->type), tokens[i]->value, tokens[i]->line, tokens[i]->col);
    }
    program = parse_program(parser);
    if (parser->error) {
        error_print_message(parser->error);
        ast_free(program); parser_free(parser); free_tokens(tokens, count); lexer_free(lexer);
        return 1;
    }
    if (debug) ast_print(program, 0);
    result = exec_program(interp, program);
    if (result.signal == SIG_ERROR || interp->had_error) {
        error_print_message(interp->error_message ? interp->error_message : "خطای ناشناخته");
        value_free(result.value);
        interp->had_error = 0;
        value_free(interp->current_exception);
        interp->current_exception = NULL;
        free(interp->error_message);
        interp->error_message = NULL;
        ast_free(program); parser_free(parser); free_tokens(tokens, count); lexer_free(lexer);
        return 1;
    }
    if (print_expr && result.value && result.value->type != VAL_NONE) {
        char *s = value_to_string(result.value);
        azr_console_printf(stdout, "%s\n", s);
        free(s);
    }
    value_free(result.value);
    ast_free(program); parser_free(parser); free_tokens(tokens, count); lexer_free(lexer);
    return 0;
}

/* Runs a source file from disk. */
static int run_file(const char *path, int debug, const char *argv0) {
    char *source = read_file(path);
    Interpreter *interp = NULL;
    char *stdlib_path = NULL;
    char *start_dir = NULL;
    int code = 0;
    if (!source) {
        char *msg = error_file_not_found(path);
        error_print_message(msg);
        free(msg);
        return 1;
    }
    interp = interpreter_new();
    interpreter_set_main_file(interp, path);
    stdlib_path = stdlib_from_argv0(argv0);
    if (stdlib_path) interpreter_set_stdlib_path(interp, stdlib_path);
    start_dir = path_dirname_local(path);
    configure_project_runtime(interp, start_dir);
    code = run_source(interp, source, debug, 0);
    free(start_dir);
    free(stdlib_path);
    interpreter_free(interp);
    free(source);
    return code;
}

static void print_help(void) {
    azr_console_printf(stdout, "azravibe %s\n", azr_version_get());
    azr_console_printf(stdout, "استفاده:\n");
    azr_console_printf(stdout, "  azravibe <file.azr> [--debug]\n");
    azr_console_printf(stdout, "  azravibe --test <folder>\n");
    azr_console_printf(stdout, "  azravibe --pkg-init <dir> <name> [version]\n");
    azr_console_printf(stdout, "  azravibe --pkg-install <dir> <local_path>\n");
    azr_console_printf(stdout, "  azravibe --version\n");
    azr_console_printf(stdout, "  azravibe --help\n");
}

static int run_tests(const char *folder, const char *argv0) {
#ifdef _WIN32
    char pattern[1024];
    struct _finddata_t data;
    intptr_t h = -1;
    int failed = 0, total = 0;
    snprintf(pattern, sizeof(pattern), "%s\\*.azr", folder);
    h = _findfirst(pattern, &data);
    if (h == -1) return 1;
    azr_console_printf(stdout, "اجرای تست‌ها در %s\n", folder);
    do {
        char path[1024];
        int code = 0;
        if (data.attrib & _A_SUBDIR) continue;
        snprintf(path, sizeof(path), "%s\\%s", folder, data.name);
        total++;
        code = run_file(path, 0, argv0);
        if (code != 0) {
            failed++;
            azr_console_printf(stdout, "[خطا] %s\n", data.name);
        } else {
            azr_console_printf(stdout, "[قبول] %s\n", data.name);
        }
    } while (_findnext(h, &data) == 0);
    _findclose(h);
    azr_console_printf(stdout, "تست‌ها: %d اجرا، %d خطا\n", total, failed);
    return failed ? 1 : 0;
#else
    int failed = 0, total = 0;
    DIR *d = opendir(folder);
    struct dirent *ent = NULL;
    if (!d) return 1;
    azr_console_printf(stdout, "اجرای تست‌ها در %s\n", folder);
    while ((ent = readdir(d)) != NULL) {
        char path[1024];
        size_t a = strlen(ent->d_name);
        int code = 0;
        if (a < 4 || strcmp(ent->d_name + a - 4, ".azr") != 0) continue;
        snprintf(path, sizeof(path), "%s/%s", folder, ent->d_name);
        total++;
        code = run_file(path, 0, argv0);
        if (code != 0) {
            failed++;
            azr_console_printf(stdout, "[خطا] %s\n", ent->d_name);
        } else {
            azr_console_printf(stdout, "[قبول] %s\n", ent->d_name);
        }
    }
    closedir(d);
    azr_console_printf(stdout, "تست‌ها: %d اجرا، %d خطا\n", total, failed);
    return failed ? 1 : 0;
#endif
}

/* Returns true if a REPL buffer likely needs more indented input. */
static int needs_more(const char *buf) {
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r' || buf[len - 1] == ' ' || buf[len - 1] == '\t')) len--;
    return len > 0 && buf[len - 1] == ':';
}

/* Runs the interactive Farsi REPL and keeps going after errors. */
static int run_repl(const char *argv0) {
    Interpreter *interp = interpreter_new();
    char *stdlib_path = stdlib_from_argv0(argv0);
    char line[4096];
    char buffer[65536];
    if (stdlib_path) interpreter_set_stdlib_path(interp, stdlib_path);
    interpreter_set_main_file(interp, "<تعاملی>");
    configure_project_runtime(interp, ".");
    azr_console_printf(stdout, "══════════════════════════════\n");
    azr_console_printf(stdout, "Azravibe %s\n", azr_version_get());
    azr_console_printf(stdout, "زبان برنامه نویسی فارسی\n");
    azr_console_printf(stdout, "برای خروج این را بنویسید\n");
    azr_console_printf(stdout, "خروج()\n");
    azr_console_printf(stdout, "══════════════════════════════\n");
    while (1) {
        buffer[0] = '\0';
        azr_console_printf(stdout, "<< ");
        if (!fgets(line, sizeof(line), stdin)) break;
        strcat(buffer, line);
        while (needs_more(buffer)) {
            azr_console_printf(stdout, ".. ");
            if (!fgets(line, sizeof(line), stdin)) break;
            if (strcmp(line, "\n") == 0 || strcmp(line, "\r\n") == 0) break;
            strcat(buffer, line);
        }
        run_source(interp, buffer, 0, 1);
    }
    free(stdlib_path);
    interpreter_free(interp);
    return 0;
}

static int line_is_exit_command(const char *line) {
    size_t len = 0;
    if (!line) return 0;
    while (*line == ' ' || *line == '\t') line++;
    len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r' || line[len - 1] == ' ' || line[len - 1] == '\t')) len--;
    return len == strlen("خروج()") && strncmp(line, "خروج()", len) == 0;
}

static int run_gui_repl(const char *argv0) {
    Interpreter *interp = interpreter_new();
    char *stdlib_path = stdlib_from_argv0(argv0);
    char line[4096];
    char buffer[65536];
    int multiline = 0;
    if (stdlib_path) interpreter_set_stdlib_path(interp, stdlib_path);
    interpreter_set_main_file(interp, "<رابط گرافیکی>");
    configure_project_runtime(interp, ".");
    buffer[0] = '\0';
    azr_console_printf(stdout, "[[AZR_READY]]\n");
    while (fgets(line, sizeof(line), stdin)) {
        if (!multiline && line_is_exit_command(line)) break;
        if (multiline && (strcmp(line, "\n") == 0 || strcmp(line, "\r\n") == 0)) {
            run_source(interp, buffer, 0, 1);
            buffer[0] = '\0';
            multiline = 0;
            azr_console_printf(stdout, "[[AZR_READY]]\n");
            continue;
        }
        strcat(buffer, line);
        if (!multiline && needs_more(buffer)) {
            multiline = 1;
            azr_console_printf(stdout, "[[AZR_MORE]]\n");
            continue;
        }
        if (multiline) {
            azr_console_printf(stdout, "[[AZR_MORE]]\n");
            continue;
        }
        run_source(interp, buffer, 0, 1);
        buffer[0] = '\0';
        azr_console_printf(stdout, "[[AZR_READY]]\n");
    }
    free(stdlib_path);
    interpreter_free(interp);
    return 0;
}

/* Selects file-runner or REPL mode based on command-line arguments. */
int main(int argc, char **argv) {
    azr_console_init();
    azr_version_init(argc > 0 ? argv[0] : NULL);
    if (argc == 1) return run_repl(argv[0]);
    if (argc >= 2) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) { print_help(); return 0; }
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) { azr_console_printf(stdout, "%s\n", azr_version_get()); return 0; }
        if (strcmp(argv[1], "--gui-repl") == 0) return run_gui_repl(argv[0]);
        if (strcmp(argv[1], "--test") == 0) return run_tests(argc >= 3 ? argv[2] : "tests", argv[0]);
        if (strcmp(argv[1], "--pkg-init") == 0) {
            char *err = NULL;
            if (argc < 4) { print_help(); return 1; }
            if (!azr_project_init(argv[2], argv[3], argc >= 5 ? argv[4] : "0.1.0", &err)) {
                error_print_message(err ? err : "راه‌اندازی پروژه ناموفق بود");
                free(err);
                return 1;
            }
            azr_console_printf(stdout, "پروژه آماده شد: %s\n", argv[2]);
            return 0;
        }
        if (strcmp(argv[1], "--pkg-install") == 0) {
            char *err = NULL;
            if (argc < 4) { print_help(); return 1; }
            if (!azr_project_install_local(argv[2], argv[3], &err)) {
                error_print_message(err ? err : "نصب بسته محلی ناموفق بود");
                free(err);
                return 1;
            }
            azr_console_printf(stdout, "بسته محلی نصب شد در %s\n", argv[2]);
            return 0;
        }
        int debug = argc >= 3 && strcmp(argv[2], "--debug") == 0;
        return run_file(argv[1], debug, argv[0]);
    }
    return 0;
}
