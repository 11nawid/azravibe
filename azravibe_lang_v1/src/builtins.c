#include "builtins.h"
#include "console.h"
#include "interpreter.h"
#include "stdlib/stdlib_native.h"
#include "utf8.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <regex.h>
#endif

/* Converts a numeric value to double for builtin math. */
static double as_double(Value *v) {
    if (v->type == VAL_INT) return (double)v->int_val;
    if (v->type == VAL_FLOAT) return v->float_val;
    if (v->type == VAL_STRING) return strtod(v->str_val, NULL);
    return 0.0;
}

/* Converts a numeric value to integer for builtin indexes and ranges. */
static long long as_int(Value *v) {
    if (v->type == VAL_INT) return v->int_val;
    if (v->type == VAL_FLOAT) return (long long)v->float_val;
    if (v->type == VAL_STRING) return strtoll(v->str_val, NULL, 10);
    if (v->type == VAL_BOOL) return v->bool_val;
    return 0;
}

/* Implements بنویس by printing all arguments with spaces and a newline. */
static Value *b_print(Value **args, int argc) {
    int i = 0;
    for (i = 0; i < argc; i++) {
        Value *sv = NULL;
        char *s = NULL;
        if (interpreter_try_protocol_string(args[i], &sv)) {
            if (!sv) return NULL;
            s = azr_strdup(sv->str_val);
            value_free(sv);
        } else {
            s = value_to_string(args[i]);
        }
        if (i) azr_console_printf(stdout, " ");
        azr_console_printf(stdout, "%s", s);
        free(s);
    }
    azr_console_printf(stdout, "\n");
    return make_none();
}

/* Implements بگیر by printing an optional prompt and reading a UTF-8 line. */
static Value *b_input(Value **args, int argc) {
    char buf[4096];
    if (argc > 0) {
        char *p = value_to_string(args[0]);
        azr_console_printf(stdout, "%s", p);
        fflush(stdout);
        free(p);
    }
    if (!fgets(buf, sizeof(buf), stdin)) return make_string("");
    buf[strcspn(buf, "\r\n")] = '\0';
    return make_string(buf);
}

/* Implements عدد_صحیح conversion. */
static Value *b_int(Value **args, int argc) {
    return argc ? make_int(as_int(args[0])) : make_int(0);
}

/* Implements عدد_اعشاری conversion. */
static Value *b_float(Value **args, int argc) {
    return argc ? make_float(as_double(args[0])) : make_float(0.0);
}

/* Implements متن conversion. */
static Value *b_str(Value **args, int argc) {
    Value *out = NULL;
    if (argc < 1) return make_string("");
    if (interpreter_try_protocol_string(args[0], &out)) return out;
    {
        char *s = value_to_string(args[0]);
        Value *v = make_string(s);
        free(s);
        return v;
    }
}

/* Implements بولی conversion. */
static Value *b_bool(Value **args, int argc) {
    return make_bool(argc ? value_is_truthy(args[0]) : 0);
}

/* Implements نوعش by returning the Farsi type name. */
static Value *b_typeof(Value **args, int argc) {
    return make_string(argc ? value_type_name(args[0]) : "هیچ");
}

/* Implements طول for strings and lists. */
static Value *b_len(Value **args, int argc) {
    Value *out = NULL;
    if (argc < 1) return make_int(0);
    if (interpreter_try_protocol_length(args[0], &out)) return out;
    if (args[0]->type == VAL_STRING) return make_int(utf8_strlen(args[0]->str_val));
    if (args[0]->type == VAL_LIST) return make_int(args[0]->list.count);
    if (args[0]->type == VAL_TUPLE) return make_int(args[0]->list.count);
    if (args[0]->type == VAL_SET) return make_int(args[0]->set.count);
    if (args[0]->type == VAL_DICT) return make_int(args[0]->dict.count);
    if (args[0]->type == VAL_BYTES) return make_int(args[0]->bytes.count);
    return make_int(0);
}

static Value *b_class_method(Value **args, int argc) {
    if (argc < 1 || args[0]->type != VAL_FUNCTION) {
        return interpreter_builtin_exception("خطای_نوع", "کلاس_متد فقط یک کار را می‌پذیرد");
    }
    return make_class_method(args[0]);
}

static Value *b_static_method(Value **args, int argc) {
    if (argc < 1 || args[0]->type != VAL_FUNCTION) {
        return interpreter_builtin_exception("خطای_نوع", "ایستا فقط یک کار را می‌پذیرد");
    }
    return make_static_method(args[0]);
}

static Value *b_property(Value **args, int argc) {
    if (argc < 1 || args[0]->type != VAL_FUNCTION) {
        return interpreter_builtin_exception("خطای_نوع", "ویژگی به تابع getter نیاز دارد");
    }
    if (argc > 1 && args[1]->type != VAL_FUNCTION && args[1]->type != VAL_NONE) {
        return interpreter_builtin_exception("خطای_نوع", "setter ویژگی باید کار باشد");
    }
    return make_property(args[0], argc > 1 && args[1]->type == VAL_FUNCTION ? args[1] : NULL);
}

static Value *b_iter(Value **args, int argc) {
    if (argc < 1) return interpreter_builtin_exception("خطای_نوع", "تکرار به یک مقدار نیاز دارد");
    return interpreter_builtin_iter(args[0]);
}

static Value *b_next(Value **args, int argc) {
    if (argc < 1) return interpreter_builtin_exception("خطای_نوع", "بعدی به یک تکرارگر نیاز دارد");
    return interpreter_builtin_next(args[0]);
}

static Value *b_assert(Value **args, int argc) {
    char *message = NULL;
    if (argc < 1) return interpreter_builtin_exception("خطای_ادعا", "ادعا به یک شرط نیاز دارد");
    if (value_is_truthy(args[0])) return make_none();
    if (argc > 1) {
        Value *msgv = NULL;
        if (interpreter_try_protocol_string(args[1], &msgv)) {
            if (!msgv) return NULL;
            message = azr_strdup(msgv->str_val);
            value_free(msgv);
        } else {
            message = value_to_string(args[1]);
        }
    } else {
        message = azr_strdup("ادعا برقرار نیست");
    }
    {
        Value *err = interpreter_builtin_exception("خطای_ادعا", message);
        free(message);
        return err;
    }
}

/* Implements اضافه by appending to a list in place. */
static Value *b_append(Value **args, int argc) {
    if (argc >= 2 && args[0]->type == VAL_LIST) list_append(args[0], value_copy(args[1]));
    return make_none();
}

static Value *b_keys(Value **args, int argc) {
    Value *out = make_list();
    int i = 0;
    if (argc < 1 || args[0]->type != VAL_DICT) return out;
    for (i = 0; i < args[0]->dict.count; i++) list_append(out, make_string(args[0]->dict.keys[i]));
    return out;
}

static Value *b_values(Value **args, int argc) {
    Value *out = make_list();
    int i = 0;
    if (argc < 1 || args[0]->type != VAL_DICT) return out;
    for (i = 0; i < args[0]->dict.count; i++) list_append(out, value_copy(args[0]->dict.values[i]));
    return out;
}

static Value *b_has(Value **args, int argc) {
    char *key = NULL;
    Value *found = NULL;
    if (argc < 2) return make_bool(0);
    if (args[0]->type == VAL_DICT) {
        key = value_to_string(args[1]);
        found = dict_get(args[0], key);
        free(key);
        return make_bool(found != NULL);
    }
    if (args[0]->type == VAL_SET) {
        int i = 0;
        for (i = 0; i < args[0]->set.count; i++) if (value_equals(args[0]->set.items[i], args[1])) return make_bool(1);
    }
    return make_bool(0);
}

static Value *b_delete(Value **args, int argc) {
    int i = 0, j = 0;
    char *key = NULL;
    if (argc < 2) return make_bool(0);
    if (args[0]->type == VAL_DICT) {
        key = value_to_string(args[1]);
        for (i = 0; i < args[0]->dict.count; i++) {
            if (strcmp(args[0]->dict.keys[i], key) == 0) {
                free(args[0]->dict.keys[i]);
                value_free(args[0]->dict.values[i]);
                for (j = i + 1; j < args[0]->dict.count; j++) {
                    args[0]->dict.keys[j - 1] = args[0]->dict.keys[j];
                    args[0]->dict.values[j - 1] = args[0]->dict.values[j];
                }
                args[0]->dict.count--;
                free(key);
                return make_bool(1);
            }
        }
        free(key);
    } else if (args[0]->type == VAL_SET) {
        for (i = 0; i < args[0]->set.count; i++) {
            if (value_equals(args[0]->set.items[i], args[1])) {
                value_free(args[0]->set.items[i]);
                for (j = i + 1; j < args[0]->set.count; j++) args[0]->set.items[j - 1] = args[0]->set.items[j];
                args[0]->set.count--;
                return make_bool(1);
            }
        }
    }
    return make_bool(0);
}

static Value *b_set_add(Value **args, int argc) {
    if (argc >= 2 && args[0]->type == VAL_SET) set_add(args[0], value_copy(args[1]));
    return make_none();
}

static Value *b_union(Value **args, int argc) {
    Value *out = make_set();
    int i = 0;
    if (argc < 1 || args[0]->type != VAL_SET) return out;
    for (i = 0; i < args[0]->set.count; i++) set_add(out, value_copy(args[0]->set.items[i]));
    if (argc > 1 && args[1]->type == VAL_SET) for (i = 0; i < args[1]->set.count; i++) set_add(out, value_copy(args[1]->set.items[i]));
    return out;
}

static Value *b_intersection(Value **args, int argc) {
    Value *out = make_set();
    int i = 0, j = 0;
    if (argc < 2 || args[0]->type != VAL_SET || args[1]->type != VAL_SET) return out;
    for (i = 0; i < args[0]->set.count; i++) for (j = 0; j < args[1]->set.count; j++) if (value_equals(args[0]->set.items[i], args[1]->set.items[j])) set_add(out, value_copy(args[0]->set.items[i]));
    return out;
}

static Value *b_difference(Value **args, int argc) {
    Value *out = make_set();
    int i = 0, j = 0, found = 0;
    if (argc < 2 || args[0]->type != VAL_SET || args[1]->type != VAL_SET) return out;
    for (i = 0; i < args[0]->set.count; i++) {
        found = 0;
        for (j = 0; j < args[1]->set.count; j++) if (value_equals(args[0]->set.items[i], args[1]->set.items[j])) found = 1;
        if (!found) set_add(out, value_copy(args[0]->set.items[i]));
    }
    return out;
}

/* Implements حذف by removing and returning a list item at an index. */
static Value *b_pop(Value **args, int argc) {
    int i = 0, idx = 0;
    Value *out = NULL;
    if (argc < 2 || args[0]->type != VAL_LIST) return make_none();
    idx = (int)as_int(args[1]);
    if (idx < 0 || idx >= args[0]->list.count) return make_none();
    out = args[0]->list.items[idx];
    for (i = idx + 1; i < args[0]->list.count; i++) args[0]->list.items[i - 1] = args[0]->list.items[i];
    args[0]->list.count--;
    return out;
}

/* Compares list items as strings for stable simple sorting. */
static int cmp_values(const void *a, const void *b) {
    Value *va = *(Value **)a, *vb = *(Value **)b;
    char *sa = value_to_string(va), *sb = value_to_string(vb);
    int r = strcmp(sa, sb);
    free(sa); free(sb);
    return r;
}

/* Implements مرتب by sorting a list in place. */
static Value *b_sort(Value **args, int argc) {
    if (argc >= 1 && args[0]->type == VAL_LIST) qsort(args[0]->list.items, (size_t)args[0]->list.count, sizeof(Value *), cmp_values);
    return make_none();
}

/* Implements برعکس by reversing a list in place. */
static Value *b_reverse(Value **args, int argc) {
    int i = 0;
    if (argc >= 1 && args[0]->type == VAL_LIST) {
        for (i = 0; i < args[0]->list.count / 2; i++) {
            Value *tmp = args[0]->list.items[i];
            args[0]->list.items[i] = args[0]->list.items[args[0]->list.count - i - 1];
            args[0]->list.items[args[0]->list.count - i - 1] = tmp;
        }
    }
    return make_none();
}

/* Implements پیدا by returning an item's first list index or -1. */
static Value *b_find(Value **args, int argc) {
    int i = 0;
    if (argc >= 2 && args[0]->type == VAL_LIST) {
        for (i = 0; i < args[0]->list.count; i++) if (value_equals(args[0]->list.items[i], args[1])) return make_int(i);
    }
    return make_int(-1);
}

/* Implements برش for UTF-8 strings using codepoint offsets. */
static Value *b_slice(Value **args, int argc) {
    const char *s = argc ? args[0]->str_val : "";
    int start = argc > 1 ? (int)as_int(args[1]) : 0;
    int end = argc > 2 ? (int)as_int(args[2]) : utf8_strlen(s);
    int cp = 0, pos = 0, bs = 0, be = (int)strlen(s), br = 0;
    while (s[pos]) {
        if (cp == start) bs = pos;
        if (cp == end) { be = pos; break; }
        utf8_codepoint(s + pos, &br);
        pos += br; cp++;
    }
    return make_string(azr_strndup(s + bs, be - bs));
}

/* Implements بزرگ for ASCII letters while preserving UTF-8 bytes. */
static Value *b_upper(Value **args, int argc) {
    char *s = argc ? azr_strdup(args[0]->str_val) : azr_strdup("");
    int i = 0;
    for (i = 0; s[i]; i++) s[i] = (char)toupper((unsigned char)s[i]);
    { Value *v = make_string(s); free(s); return v; }
}

/* Implements کوچک for ASCII letters while preserving UTF-8 bytes. */
static Value *b_lower(Value **args, int argc) {
    char *s = argc ? azr_strdup(args[0]->str_val) : azr_strdup("");
    int i = 0;
    for (i = 0; s[i]; i++) s[i] = (char)tolower((unsigned char)s[i]);
    { Value *v = make_string(s); free(s); return v; }
}

/* Implements تکرار_داره by substring search. */
static Value *b_contains(Value **args, int argc) {
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_STRING) return make_bool(0);
    return make_bool(strstr(args[0]->str_val, args[1]->str_val) != NULL);
}

/* Implements جدا_کن by splitting a string on a separator. */
static Value *b_split(Value **args, int argc) {
    Value *list = make_list();
    char *copy = NULL, *p = NULL, *next = NULL;
    const char *sep = (argc > 1 && args[1]->type == VAL_STRING) ? args[1]->str_val : " ";
    if (argc < 1 || args[0]->type != VAL_STRING) return list;
    copy = azr_strdup(args[0]->str_val);
    p = copy;
    while ((next = strstr(p, sep)) != NULL) {
        *next = '\0';
        list_append(list, make_string(p));
        p = next + strlen(sep);
    }
    list_append(list, make_string(p));
    free(copy);
    return list;
}

/* Implements بچسبان by joining list items with a separator. */
static Value *b_join(Value **args, int argc) {
    Value *list = argc ? args[0] : NULL;
    const char *sep = (argc > 1 && args[1]->type == VAL_STRING) ? args[1]->str_val : "";
    char *out = azr_strdup("");
    size_t cap = 1, len = 0;
    int i = 0;
    if (!list || list->type != VAL_LIST) return make_string("");
    for (i = 0; i < list->list.count; i++) {
        char *part = value_to_string(list->list.items[i]);
        size_t need = len + strlen(part) + strlen(sep) + 1;
        if (need > cap) { cap = need * 2; out = (char *)realloc(out, cap); }
        if (i > 0) { strcat(out, sep); len += strlen(sep); }
        strcat(out, part); len += strlen(part);
        free(part);
    }
    { Value *v = make_string(out); free(out); return v; }
}

/* Implements فاصله_نگیر by trimming ASCII whitespace around a UTF-8 string. */
static Value *b_strip(Value **args, int argc) {
    char *s = argc ? args[0]->str_val : "";
    char *start = s;
    char *end = NULL;
    while (*start && isspace((unsigned char)*start)) start++;
    end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    return make_string(azr_strndup(start, (int)(end - start)));
}

/* Implements مطلق absolute value. */
static Value *b_abs(Value **args, int argc) {
    if (argc && args[0]->type == VAL_INT) return make_int(llabs(args[0]->int_val));
    return make_float(fabs(argc ? as_double(args[0]) : 0.0));
}

/* Implements گرد rounding. */
static Value *b_round(Value **args, int argc) { return make_int((long long)llround(argc ? as_double(args[0]) : 0.0)); }

/* Implements بالا ceiling. */
static Value *b_ceil(Value **args, int argc) { return make_float(ceil(argc ? as_double(args[0]) : 0.0)); }

/* Implements پایین floor. */
static Value *b_floor(Value **args, int argc) { return make_float(floor(argc ? as_double(args[0]) : 0.0)); }

/* Implements بیشترین for two comparable numeric values. */
static Value *b_max(Value **args, int argc) {
    double a = argc > 0 ? as_double(args[0]) : 0, b = argc > 1 ? as_double(args[1]) : 0;
    return make_float(a > b ? a : b);
}

/* Implements کمترین for two comparable numeric values. */
static Value *b_min(Value **args, int argc) {
    double a = argc > 0 ? as_double(args[0]) : 0, b = argc > 1 ? as_double(args[1]) : 0;
    return make_float(a < b ? a : b);
}

/* Implements محدوده with one, two, or three integer arguments. */
static Value *b_range(Value **args, int argc) {
    long long start = 0, end = 0, step = 1, x = 0;
    Value *list = make_list();
    if (argc == 1) { end = as_int(args[0]); }
    else if (argc >= 2) { start = as_int(args[0]); end = as_int(args[1]); }
    if (argc >= 3) step = as_int(args[2]);
    if (step == 0) step = 1;
    if (step > 0) for (x = start; x < end; x += step) list_append(list, make_int(x));
    else for (x = start; x > end; x += step) list_append(list, make_int(x));
    return list;
}

/* Implements خروج by terminating the host process. */
static Value *b_exit(Value **args, int argc) {
    exit(argc ? (int)as_int(args[0]) : 0);
    return make_none();
}

/* Implements تصادفی as a random floating-point number between 0 and 1. */
static Value *b_random(Value **args, int argc) {
    (void)args; (void)argc;
    return make_float((double)rand() / (double)RAND_MAX);
}

/* Implements زمان as current Unix timestamp. */
static Value *b_time(Value **args, int argc) {
    (void)args; (void)argc;
    return make_float((double)time(NULL));
}

static Value *b_read_file(Value **args, int argc) {
    FILE *f = NULL;
    long len = 0;
    char *buf = NULL;
    Value *out = NULL;
    if (argc < 1 || args[0]->type != VAL_STRING) return make_string("");
    f = fopen(args[0]->str_val, "rb");
    if (!f) return make_string("");
    fseek(f, 0, SEEK_END); len = ftell(f); rewind(f);
    buf = (char *)malloc((size_t)len + 1);
    fread(buf, 1, (size_t)len, f); buf[len] = 0; fclose(f);
    out = make_string(buf); free(buf); return out;
}

static Value *b_read_bytes(Value **args, int argc) {
    FILE *f = NULL;
    long len = 0;
    unsigned char *buf = NULL;
    Value *out = NULL;
    if (argc < 1 || args[0]->type != VAL_STRING) return make_bytes(NULL, 0);
    f = fopen(args[0]->str_val, "rb");
    if (!f) return make_bytes(NULL, 0);
    fseek(f, 0, SEEK_END); len = ftell(f); rewind(f);
    buf = (unsigned char *)malloc((size_t)len);
    fread(buf, 1, (size_t)len, f); fclose(f);
    out = make_bytes(buf, (int)len); free(buf); return out;
}

static Value *b_write_bytes(Value **args, int argc) {
    FILE *f = NULL;
    if (argc < 2 || args[0]->type != VAL_STRING || args[1]->type != VAL_BYTES) return make_bool(0);
    f = fopen(args[0]->str_val, "wb");
    if (!f) return make_bool(0);
    fwrite(args[1]->bytes.data, 1, (size_t)args[1]->bytes.count, f);
    fclose(f);
    return make_bool(1);
}

static Value *b_write_file(Value **args, int argc) {
    FILE *f = NULL; char *s = NULL;
    if (argc < 2 || args[0]->type != VAL_STRING) return make_bool(0);
    f = fopen(args[0]->str_val, "wb");
    if (!f) return make_bool(0);
    s = value_to_string(args[1]); fwrite(s, 1, strlen(s), f); free(s); fclose(f);
    return make_bool(1);
}

static Value *b_append_file(Value **args, int argc) {
    FILE *f = NULL; char *s = NULL;
    if (argc < 2 || args[0]->type != VAL_STRING) return make_bool(0);
    f = fopen(args[0]->str_val, "ab");
    if (!f) return make_bool(0);
    s = value_to_string(args[1]); fwrite(s, 1, strlen(s), f); free(s); fclose(f);
    return make_bool(1);
}

static Value *b_exists(Value **args, int argc) {
    struct stat st;
    if (argc < 1 || args[0]->type != VAL_STRING) return make_bool(0);
    return make_bool(stat(args[0]->str_val, &st) == 0);
}

static Value *b_list_dir(Value **args, int argc) {
    const char *path = (argc > 0 && args[0]->type == VAL_STRING) ? args[0]->str_val : ".";
    Value *list = make_list();
    DIR *d = opendir(path);
    struct dirent *ent = NULL;
    if (!d) return list;
    while ((ent = readdir(d)) != NULL) {
        if (strcmp(ent->d_name, ".") && strcmp(ent->d_name, "..")) list_append(list, make_string(ent->d_name));
    }
    closedir(d);
    return list;
}

static Value *b_path_join(Value **args, int argc) {
    char *out = azr_strdup("");
    size_t cap = 1, len = 0;
    int i = 0;
    for (i = 0; i < argc; i++) {
        char *part = value_to_string(args[i]);
        size_t need = len + strlen(part) + 2;
        if (need > cap) { cap = need * 2; out = (char *)realloc(out, cap); }
        if (len > 0 && out[len - 1] != '/' && out[len - 1] != '\\') { strcat(out, "/"); len++; }
        strcat(out, part); len += strlen(part); free(part);
    }
    { Value *v = make_string(out); free(out); return v; }
}

static Value *b_env(Value **args, int argc) {
    char *v = NULL;
    if (argc < 1 || args[0]->type != VAL_STRING) return make_none();
    v = getenv(args[0]->str_val);
    return v ? make_string(v) : make_none();
}

static Value *b_shell(Value **args, int argc) {
    if (argc < 1 || args[0]->type != VAL_STRING) return make_int(-1);
    return make_int(system(args[0]->str_val));
}

static Value *b_regex(Value **args, int argc) {
    if (argc < 1) return make_regex("");
    return make_regex(args[0]->type == VAL_STRING ? args[0]->str_val : "");
}

static Value *b_regex_match(Value **args, int argc) {
#ifdef _WIN32
    const char *pat = NULL, *text = NULL;
    if (argc < 2) return make_bool(0);
    pat = args[0]->type == VAL_REGEX ? args[0]->regex.pattern : (args[0]->type == VAL_STRING ? args[0]->str_val : "");
    text = args[1]->type == VAL_STRING ? args[1]->str_val : "";
    return make_bool(strstr(text, pat) != NULL);
#else
    regex_t re;
    const char *pat = NULL, *text = NULL;
    int ok = 0;
    if (argc < 2) return make_bool(0);
    pat = args[0]->type == VAL_REGEX ? args[0]->regex.pattern : (args[0]->type == VAL_STRING ? args[0]->str_val : "");
    text = args[1]->type == VAL_STRING ? args[1]->str_val : "";
    if (regcomp(&re, pat, REG_EXTENDED | REG_NOSUB) != 0) return make_bool(0);
    ok = regexec(&re, text, 0, NULL, 0) == 0;
    regfree(&re);
    return make_bool(ok);
#endif
}

static Value *b_bytes(Value **args, int argc) {
    if (argc < 1) return make_bytes(NULL, 0);
    if (args[0]->type == VAL_STRING) return make_bytes((const unsigned char *)args[0]->str_val, (int)strlen(args[0]->str_val));
    if (args[0]->type == VAL_LIST) {
        int i = 0; unsigned char *buf = (unsigned char *)calloc((size_t)args[0]->list.count, 1);
        for (i = 0; i < args[0]->list.count; i++) buf[i] = (unsigned char)as_int(args[0]->list.items[i]);
        { Value *v = make_bytes(buf, args[0]->list.count); free(buf); return v; }
    }
    return make_bytes(NULL, 0);
}

static Value *b_bytes_to_text(Value **args, int argc) {
    if (argc < 1 || args[0]->type != VAL_BYTES) return make_string("");
    return make_string((const char *)args[0]->bytes.data);
}

static Value *b_datetime_now(Value **args, int argc) {
    (void)args; (void)argc;
    return make_datetime((double)time(NULL));
}

/* Registers one builtin function into the global environment. */
static void reg(Environment *env, const char *name, Value *(*fn)(Value **, int)) {
    env_set(env, name, make_builtin(name, fn));
}

/* Registers all built-in functions expected by azravibe_lang_v1. */
void builtins_register(Environment *env) {
    srand((unsigned int)time(NULL));
    reg(env, "بنویس", b_print); reg(env, "ینویس", b_print); reg(env, "print", b_print);
    reg(env, "بگیر", b_input); reg(env, "گیر", b_input); reg(env, "input", b_input);
    reg(env, "عدد_صحیح", b_int); reg(env, "int", b_int); reg(env, "عدد_اعشاری", b_float); reg(env, "float", b_float);
    reg(env, "متن", b_str); reg(env, "str", b_str); reg(env, "بولی", b_bool); reg(env, "bool", b_bool); reg(env, "نوعش", b_typeof); reg(env, "type", b_typeof);
    reg(env, "طول", b_len); reg(env, "len", b_len); reg(env, "اضافه", b_append); reg(env, "append", b_append); reg(env, "حذف", b_pop); reg(env, "pop", b_pop); reg(env, "مرتب", b_sort); reg(env, "sort", b_sort); reg(env, "برعکس", b_reverse); reg(env, "reverse", b_reverse); reg(env, "پیدا", b_find); reg(env, "find", b_find);
    reg(env, "کلیدها", b_keys); reg(env, "مقدارها", b_values); reg(env, "دارد", b_has); reg(env, "پاک_کن", b_delete);
    reg(env, "عضو_اضافه", b_set_add); reg(env, "اجتماع", b_union); reg(env, "اشتراک", b_intersection); reg(env, "تفاضل", b_difference);
    reg(env, "برش", b_slice); reg(env, "بزرگ", b_upper); reg(env, "کوچک", b_lower); reg(env, "تکرار_داره", b_contains); reg(env, "جدا_کن", b_split); reg(env, "بچسبان", b_join); reg(env, "فاصله_نگیر", b_strip);
    reg(env, "مطلق", b_abs); reg(env, "abs", b_abs); reg(env, "گرد", b_round); reg(env, "round", b_round); reg(env, "بالا", b_ceil); reg(env, "ceil", b_ceil); reg(env, "پایین", b_floor); reg(env, "floor", b_floor); reg(env, "بیشترین", b_max); reg(env, "max", b_max); reg(env, "کمترین", b_min); reg(env, "min", b_min);
    reg(env, "محدوده", b_range); reg(env, "range", b_range); reg(env, "خروج", b_exit); reg(env, "exit", b_exit); reg(env, "تصادفی", b_random); reg(env, "random", b_random); reg(env, "زمان", b_time); reg(env, "time", b_time);
    reg(env, "کلاس_متد", b_class_method); reg(env, "classmethod", b_class_method);
    reg(env, "ایستا", b_static_method); reg(env, "staticmethod", b_static_method);
    reg(env, "ویژگی", b_property); reg(env, "property", b_property);
    reg(env, "تکرار", b_iter); reg(env, "iter", b_iter); reg(env, "بعدی", b_next); reg(env, "next", b_next);
    reg(env, "ادعا", b_assert); reg(env, "assert", b_assert);
    reg(env, "فایل_بخوان", b_read_file); reg(env, "فایل_بنویس", b_write_file); reg(env, "فایل_اضافه", b_append_file); reg(env, "وجود_دارد", b_exists);
    reg(env, "بایت_بخوان", b_read_bytes); reg(env, "بایت_بنویس", b_write_bytes);
    reg(env, "پوشه_لیست", b_list_dir); reg(env, "مسیر_بچسبان", b_path_join); reg(env, "محیط", b_env); reg(env, "فرمان", b_shell);
    reg(env, "regex", b_regex); reg(env, "regex_دارد", b_regex_match); reg(env, "بایت", b_bytes); reg(env, "بایت_به_متن", b_bytes_to_text); reg(env, "زمان_اکنون", b_datetime_now);
    azr_stdlib_register(env);
}
