#include "common.h"

#include <math.h>

static int g_log_level = 20;
static char g_log_file[512] = "";

static double rand_unit(void) {
    return ((double)rand() + 1.0) / ((double)RAND_MAX + 2.0);
}

static Value *b_rand_int(Value **args, int argc) {
    long long a = azr_int_arg(args, argc, 0, 0), b = azr_int_arg(args, argc, 1, 100);
    if (b < a) { long long t = a; a = b; b = t; }
    return make_int(a + rand() % (int)(b - a + 1));
}

static Value *b_rand_choice(Value **args, int argc) {
    if (argc < 1 || args[0]->type != VAL_LIST || args[0]->list.count == 0) return make_none();
    return value_copy(args[0]->list.items[rand() % args[0]->list.count]);
}

static Value *b_rand_seed(Value **args, int argc) {
    srand((unsigned int)azr_int_arg(args, argc, 0, (long long)time(NULL)));
    return make_none();
}

static Value *b_rand_shuffle(Value **args, int argc) {
    int i = 0;
    if (argc < 1 || args[0]->type != VAL_LIST) return make_none();
    for (i = args[0]->list.count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Value *tmp = args[0]->list.items[i];
        args[0]->list.items[i] = args[0]->list.items[j];
        args[0]->list.items[j] = tmp;
    }
    return make_none();
}

static Value *b_rand_sample(Value **args, int argc) {
    Value *out = make_list(), *pool = NULL;
    long long k = azr_int_arg(args, argc, 1, 0);
    int i = 0;
    if (argc < 1 || args[0]->type != VAL_LIST || k <= 0) return out;
    pool = make_list();
    for (i = 0; i < args[0]->list.count; i++) list_append(pool, value_copy(args[0]->list.items[i]));
    if (k > pool->list.count) k = pool->list.count;
    for (i = 0; i < k; i++) {
        int j = rand() % pool->list.count;
        list_append(out, value_copy(pool->list.items[j]));
        value_free(pool->list.items[j]);
        for (int m = j + 1; m < pool->list.count; m++) pool->list.items[m - 1] = pool->list.items[m];
        pool->list.count--;
    }
    value_free(pool);
    return out;
}

static Value *b_rand_choices(Value **args, int argc) {
    Value *out = make_list();
    long long k = azr_int_arg(args, argc, 1, 1);
    int i = 0;
    if (argc < 1 || args[0]->type != VAL_LIST || args[0]->list.count == 0 || k <= 0) return out;
    for (i = 0; i < k; i++) list_append(out, value_copy(args[0]->list.items[rand() % args[0]->list.count]));
    return out;
}

static Value *b_rand_getrandbits(Value **args, int argc) {
    int bits = (int)azr_int_arg(args, argc, 0, 32);
    unsigned long long out = 0;
    if (bits < 1) bits = 1;
    if (bits > 62) bits = 62;
    for (int i = 0; i < bits; i += 15) out = (out << 15) ^ (unsigned long long)(rand() & 0x7fff);
    if (bits < 62) out &= ((1ULL << bits) - 1ULL);
    return make_int((long long)out);
}

static Value *b_rand_expovariate(Value **args, int argc) {
    double lambda = azr_double_arg(args, argc, 0, 1.0);
    if (lambda == 0.0) lambda = 1.0;
    return make_float(-log(rand_unit()) / lambda);
}

static Value *b_rand_gauss(Value **args, int argc) {
    double mu = azr_double_arg(args, argc, 0, 0.0);
    double sigma = azr_double_arg(args, argc, 1, 1.0);
    double u1 = rand_unit(), u2 = rand_unit();
    double z = sqrt(-2.0 * log(u1)) * cos(6.283185307179586 * u2);
    return make_float(mu + z * sigma);
}

static Value *b_rand_triangular(Value **args, int argc) {
    double low = azr_double_arg(args, argc, 0, 0.0);
    double high = azr_double_arg(args, argc, 1, 1.0);
    double mode = azr_double_arg(args, argc, 2, (low + high) / 2.0);
    double u = rand_unit();
    double c = (mode - low) / (high - low);
    if (high == low) return make_float(low);
    if (u <= c) return make_float(low + sqrt(u * (high - low) * (mode - low)));
    return make_float(high - sqrt((1.0 - u) * (high - low) * (high - mode)));
}

static Value *b_now_iso(Value **args, int argc) {
    time_t t = time(NULL); struct tm *tmv = localtime(&t); char buf[64]; (void)args; (void)argc;
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", tmv);
    return make_string(buf);
}

static Value *b_datetime_parts(Value **args, int argc) {
    time_t t = (time_t)azr_int_arg(args, argc, 0, (long long)time(NULL));
    struct tm *tmv = localtime(&t);
    Value *out = make_dict();
    dict_set(out, "year", make_int(tmv->tm_year + 1900));
    dict_set(out, "month", make_int(tmv->tm_mon + 1));
    dict_set(out, "day", make_int(tmv->tm_mday));
    dict_set(out, "hour", make_int(tmv->tm_hour));
    dict_set(out, "minute", make_int(tmv->tm_min));
    dict_set(out, "second", make_int(tmv->tm_sec));
    dict_set(out, "weekday", make_int(tmv->tm_wday == 0 ? 6 : tmv->tm_wday - 1));
    return out;
}

static Value *b_datetime_parse_iso(Value **args, int argc) {
    const char *s = azr_str_arg(args, argc, 0, "");
    struct tm tmv;
    int n = 0;
    memset(&tmv, 0, sizeof(tmv));
    n = sscanf(s, "%d-%d-%dT%d:%d:%d", &tmv.tm_year, &tmv.tm_mon, &tmv.tm_mday, &tmv.tm_hour, &tmv.tm_min, &tmv.tm_sec);
    if (n < 3) n = sscanf(s, "%d-%d-%d %d:%d:%d", &tmv.tm_year, &tmv.tm_mon, &tmv.tm_mday, &tmv.tm_hour, &tmv.tm_min, &tmv.tm_sec);
    if (n < 3) return make_int(0);
    tmv.tm_year -= 1900;
    tmv.tm_mon -= 1;
    tmv.tm_isdst = -1;
    return make_int((long long)mktime(&tmv));
}

static Value *b_time_format(Value **args, int argc) {
    time_t t = (time_t)azr_int_arg(args, argc, 0, (long long)time(NULL));
    const char *fmt = azr_str_arg(args, argc, 1, "%Y-%m-%d %H:%M:%S");
    struct tm *tmv = localtime(&t); char buf[128];
    strftime(buf, sizeof(buf), fmt, tmv);
    return make_string(buf);
}

static Value *b_math_sin(Value **a, int c) { return make_float(sin(azr_double_arg(a, c, 0, 0))); }
static Value *b_math_cos(Value **a, int c) { return make_float(cos(azr_double_arg(a, c, 0, 0))); }
static Value *b_math_tan(Value **a, int c) { return make_float(tan(azr_double_arg(a, c, 0, 0))); }
static Value *b_math_asin(Value **a, int c) { return make_float(asin(azr_double_arg(a, c, 0, 0))); }
static Value *b_math_acos(Value **a, int c) { return make_float(acos(azr_double_arg(a, c, 0, 0))); }
static Value *b_math_atan(Value **a, int c) { return make_float(atan(azr_double_arg(a, c, 0, 0))); }
static Value *b_math_atan2(Value **a, int c) { return make_float(atan2(azr_double_arg(a, c, 0, 0), azr_double_arg(a, c, 1, 0))); }
static Value *b_math_log(Value **a, int c) { return make_float(log(azr_double_arg(a, c, 0, 1))); }
static Value *b_math_log10(Value **a, int c) { return make_float(log10(azr_double_arg(a, c, 0, 1))); }
static Value *b_math_exp(Value **a, int c) { return make_float(exp(azr_double_arg(a, c, 0, 0))); }
static Value *b_math_sqrt(Value **a, int c) { return make_float(sqrt(azr_double_arg(a, c, 0, 0))); }
static Value *b_math_pow(Value **a, int c) { return make_float(pow(azr_double_arg(a, c, 0, 0), azr_double_arg(a, c, 1, 1))); }
static Value *b_math_isclose(Value **a, int c) {
    double x = azr_double_arg(a, c, 0, 0), y = azr_double_arg(a, c, 1, 0), tol = azr_double_arg(a, c, 2, 1e-9);
    return make_bool(fabs(x - y) <= tol);
}
static Value *b_math_degrees(Value **a, int c) { return make_float(azr_double_arg(a, c, 0, 0) * 180.0 / 3.141592653589793); }
static Value *b_math_radians(Value **a, int c) { return make_float(azr_double_arg(a, c, 0, 0) * 3.141592653589793 / 180.0); }
static Value *b_math_trunc(Value **a, int c) { return make_int((long long)azr_double_arg(a, c, 0, 0)); }
static Value *b_math_fmod(Value **a, int c) { return make_float(fmod(azr_double_arg(a, c, 0, 0), azr_double_arg(a, c, 1, 1))); }
static Value *b_math_hypot(Value **a, int c) { return make_float(hypot(azr_double_arg(a, c, 0, 0), azr_double_arg(a, c, 1, 0))); }
static Value *b_math_copysign(Value **a, int c) { return make_float(copysign(azr_double_arg(a, c, 0, 0), azr_double_arg(a, c, 1, 1))); }
static Value *b_math_isfinite(Value **a, int c) { return make_bool(isfinite(azr_double_arg(a, c, 0, 0))); }
static Value *b_math_isinf(Value **a, int c) { return make_bool(isinf(azr_double_arg(a, c, 0, 0))); }
static Value *b_math_isnan(Value **a, int c) { return make_bool(isnan(azr_double_arg(a, c, 0, 0))); }

static long long azr_gcd(long long a, long long b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b) { long long t = a % b; a = b; b = t; }
    return a;
}

static Value *b_math_gcd(Value **a, int c) {
    long long g = c > 0 ? azr_int_arg(a, c, 0, 0) : 0;
    for (int i = 1; i < c; i++) g = azr_gcd(g, azr_int_arg(a, c, i, 0));
    return make_int(g < 0 ? -g : g);
}

static Value *b_math_lcm(Value **a, int c) {
    long long out = 1;
    if (c == 0) return make_int(1);
    for (int i = 0; i < c; i++) {
        long long n = azr_int_arg(a, c, i, 0), g = azr_gcd(out, n);
        if (n == 0 || out == 0) return make_int(0);
        out = llabs(out / g * n);
    }
    return make_int(out);
}

static long long azr_factorial_int(long long n) {
    long long out = 1;
    if (n < 0) return 0;
    for (long long i = 2; i <= n; i++) out *= i;
    return out;
}

static Value *b_math_comb(Value **a, int c) {
    long long n = azr_int_arg(a, c, 0, 0), k = azr_int_arg(a, c, 1, 0);
    long long out = 1;
    if (k < 0 || n < 0 || k > n) return make_int(0);
    if (k > n - k) k = n - k;
    for (long long i = 1; i <= k; i++) out = out * (n - k + i) / i;
    return make_int(out);
}

static Value *b_math_perm(Value **a, int c) {
    long long n = azr_int_arg(a, c, 0, 0), k = azr_int_arg(a, c, 1, n);
    long long out = 1;
    if (k < 0 || n < 0 || k > n) return make_int(0);
    for (long long i = 0; i < k; i++) out *= (n - i);
    return make_int(out);
}

static Value *b_math_factorial(Value **a, int c) {
    return make_int(azr_factorial_int(azr_int_arg(a, c, 0, 0)));
}

static Value *b_counter(Value **args, int argc) {
    Value *out = make_dict(); int i = 0;
    if (argc < 1) return out;
    if (args[0]->type == VAL_LIST || args[0]->type == VAL_TUPLE) {
        for (i = 0; i < args[0]->list.count; i++) {
            char *k = value_to_string(args[0]->list.items[i]);
            Value *old = dict_get(out, k);
            dict_set(out, k, make_int(old ? old->int_val + 1 : 1));
            free(k);
        }
    } else if (args[0]->type == VAL_STRING) {
        const char *s = args[0]->str_val;
        for (i = 0; s[i]; i++) {
            char k[2] = { s[i], 0 };
            Value *old = dict_get(out, k);
            dict_set(out, k, make_int(old ? old->int_val + 1 : 1));
        }
    }
    return out;
}

static Value *b_defaultdict(Value **args, int argc) {
    Value *out = make_dict();
    if (argc > 0) dict_set(out, "__default__", value_copy(args[0]));
    return out;
}

static Value *b_deque(Value **args, int argc) {
    Value *out = make_list(); int i = 0;
    if (argc > 0 && (args[0]->type == VAL_LIST || args[0]->type == VAL_TUPLE)) {
        for (i = 0; i < args[0]->list.count; i++) list_append(out, value_copy(args[0]->list.items[i]));
    }
    return out;
}

static Value *b_deque_appendleft(Value **args, int argc) {
    Value *list = argc > 0 ? args[0] : NULL;
    int i = 0;
    if (!list || list->type != VAL_LIST || argc < 2) return make_none();
    if (list->list.count >= list->list.capacity) {
        list->list.capacity *= 2;
        list->list.items = (Value **)realloc(list->list.items, (size_t)list->list.capacity * sizeof(Value *));
    }
    for (i = list->list.count; i > 0; i--) list->list.items[i] = list->list.items[i - 1];
    list->list.items[0] = value_copy(args[1]);
    list->list.count++;
    return make_none();
}

static Value *b_deque_popleft(Value **args, int argc) {
    Value *list = argc > 0 ? args[0] : NULL, *out = NULL;
    int i = 0;
    if (!list || list->type != VAL_LIST || list->list.count == 0) return make_none();
    out = list->list.items[0];
    for (i = 1; i < list->list.count; i++) list->list.items[i - 1] = list->list.items[i];
    list->list.count--;
    return out;
}

static Value *b_deque_rotate(Value **args, int argc) {
    Value *list = argc > 0 ? args[0] : NULL;
    long long n = azr_int_arg(args, argc, 1, 1);
    if (!list || list->type != VAL_LIST || list->list.count == 0) return make_none();
    n %= list->list.count;
    if (n < 0) n += list->list.count;
    for (long long r = 0; r < n; r++) {
        Value *last = list->list.items[list->list.count - 1];
        for (int i = list->list.count - 1; i > 0; i--) list->list.items[i] = list->list.items[i - 1];
        list->list.items[0] = last;
    }
    return make_none();
}

static Value *b_counter_elements(Value **args, int argc) {
    Value *out = make_list(), *d = argc > 0 ? args[0] : NULL;
    if (!d || d->type != VAL_DICT) return out;
    for (int i = 0; i < d->dict.count; i++) {
        long long n = d->dict.values[i]->type == VAL_INT ? d->dict.values[i]->int_val : 0;
        for (long long j = 0; j < n; j++) list_append(out, make_string(d->dict.keys[i]));
    }
    return out;
}

static Value *b_counter_total(Value **args, int argc) {
    Value *d = argc > 0 ? args[0] : NULL;
    long long total = 0;
    if (!d || d->type != VAL_DICT) return make_int(0);
    for (int i = 0; i < d->dict.count; i++) if (d->dict.values[i]->type == VAL_INT) total += d->dict.values[i]->int_val;
    return make_int(total);
}

static Value *b_parse_args(Value **args, int argc) {
    Value *out = make_dict(); int i = 0;
    if (argc < 1 || args[0]->type != VAL_LIST) return out;
    dict_set(out, "_", make_list());
    for (i = 0; i < args[0]->list.count; i++) {
        char *item = value_to_string(args[0]->list.items[i]);
        if (strncmp(item, "--", 2) == 0) {
            char *eq = strchr(item + 2, '=');
            if (eq) { *eq = 0; dict_set(out, item + 2, make_string(eq + 1)); }
            else if (i + 1 < args[0]->list.count) {
                char *next = value_to_string(args[0]->list.items[i + 1]);
                if (strncmp(next, "-", 1) != 0) { dict_set(out, item + 2, make_string(next)); i++; }
                else dict_set(out, item + 2, make_bool(1));
                free(next);
            } else dict_set(out, item + 2, make_bool(1));
        } else if (item[0] == '-' && item[1] && item[2]) {
            for (int j = 1; item[j]; j++) {
                char key[2] = { item[j], 0 };
                dict_set(out, key, make_bool(1));
            }
        } else if (item[0] == '-' && item[1]) {
            char key[2] = { item[1], 0 };
            if (i + 1 < args[0]->list.count) {
                char *next = value_to_string(args[0]->list.items[i + 1]);
                if (strncmp(next, "-", 1) != 0) { dict_set(out, key, make_string(next)); i++; }
                else dict_set(out, key, make_bool(1));
                free(next);
            } else dict_set(out, key, make_bool(1));
        } else {
            list_append(dict_get(out, "_"), make_string(item));
        }
        free(item);
    }
    return out;
}

static Value *b_parse_known_args(Value **args, int argc) {
    Value *pair = make_list();
    list_append(pair, b_parse_args(args, argc));
    list_append(pair, make_list());
    return pair;
}

static int log_level_value(const char *level) {
    if (strcmp(level, "DEBUG") == 0) return 10;
    if (strcmp(level, "INFO") == 0) return 20;
    if (strcmp(level, "WARNING") == 0 || strcmp(level, "WARN") == 0) return 30;
    if (strcmp(level, "ERROR") == 0) return 40;
    if (strcmp(level, "CRITICAL") == 0) return 50;
    return 20;
}

static Value *b_log(Value **args, int argc) {
    const char *level = azr_str_arg(args, argc, 0, "INFO");
    char *msg = argc > 1 ? value_to_string(args[1]) : azr_strdup("");
    int level_num = log_level_value(level);
    if (level_num >= g_log_level) {
        FILE *f = g_log_file[0] ? fopen(g_log_file, "a") : stderr;
        if (f) {
            fprintf(f, "[%s] %s\n", level, msg);
            if (f != stderr) fclose(f);
        }
    }
    free(msg); return make_none();
}

static Value *b_log_config(Value **args, int argc) {
    if (argc > 0 && args[0]->type == VAL_DICT) {
        Value *level = dict_get(args[0], "level"), *filename = dict_get(args[0], "filename");
        if (level && level->type == VAL_STRING) g_log_level = log_level_value(level->str_val);
        if (filename && filename->type == VAL_STRING) {
            strncpy(g_log_file, filename->str_val, sizeof(g_log_file) - 1);
            g_log_file[sizeof(g_log_file) - 1] = 0;
        }
    } else {
        const char *level = azr_str_arg(args, argc, 0, "");
        const char *filename = azr_str_arg(args, argc, 1, "");
        if (level[0]) g_log_level = log_level_value(level);
        if (filename[0]) {
            strncpy(g_log_file, filename, sizeof(g_log_file) - 1);
            g_log_file[sizeof(g_log_file) - 1] = 0;
        }
    }
    return make_none();
}

static Value *b_thread_count(Value **args, int argc) {
    (void)args; (void)argc; return make_int(1);
}

void azr_register_data_time(Environment *env) {
    azr_reg(env, "__random_int", b_rand_int);
    azr_reg(env, "__random_choice", b_rand_choice);
    azr_reg(env, "__random_seed", b_rand_seed);
    azr_reg(env, "__random_shuffle", b_rand_shuffle);
    azr_reg(env, "__random_sample", b_rand_sample);
    azr_reg(env, "__random_choices", b_rand_choices);
    azr_reg(env, "__random_getrandbits", b_rand_getrandbits);
    azr_reg(env, "__random_expovariate", b_rand_expovariate);
    azr_reg(env, "__random_gauss", b_rand_gauss);
    azr_reg(env, "__random_triangular", b_rand_triangular);
    azr_reg(env, "__datetime_iso", b_now_iso);
    azr_reg(env, "__datetime_format", b_time_format);
    azr_reg(env, "__datetime_parts", b_datetime_parts);
    azr_reg(env, "__datetime_parse_iso", b_datetime_parse_iso);
    azr_reg(env, "__math_sin", b_math_sin); azr_reg(env, "__math_cos", b_math_cos);
    azr_reg(env, "__math_tan", b_math_tan); azr_reg(env, "__math_asin", b_math_asin);
    azr_reg(env, "__math_acos", b_math_acos); azr_reg(env, "__math_atan", b_math_atan);
    azr_reg(env, "__math_atan2", b_math_atan2);
    azr_reg(env, "__math_log", b_math_log); azr_reg(env, "__math_log10", b_math_log10);
    azr_reg(env, "__math_exp", b_math_exp); azr_reg(env, "__math_sqrt", b_math_sqrt);
    azr_reg(env, "__math_pow", b_math_pow); azr_reg(env, "__math_isclose", b_math_isclose);
    azr_reg(env, "__math_degrees", b_math_degrees); azr_reg(env, "__math_radians", b_math_radians);
    azr_reg(env, "__math_trunc", b_math_trunc); azr_reg(env, "__math_fmod", b_math_fmod);
    azr_reg(env, "__math_hypot", b_math_hypot); azr_reg(env, "__math_copysign", b_math_copysign);
    azr_reg(env, "__math_isfinite", b_math_isfinite); azr_reg(env, "__math_isinf", b_math_isinf);
    azr_reg(env, "__math_isnan", b_math_isnan); azr_reg(env, "__math_gcd", b_math_gcd);
    azr_reg(env, "__math_lcm", b_math_lcm);
    azr_reg(env, "__math_comb", b_math_comb); azr_reg(env, "__math_perm", b_math_perm);
    azr_reg(env, "__math_factorial_native", b_math_factorial);
    azr_reg(env, "__collections_counter", b_counter);
    azr_reg(env, "__collections_defaultdict", b_defaultdict);
    azr_reg(env, "__collections_deque", b_deque);
    azr_reg(env, "__collections_deque_appendleft", b_deque_appendleft);
    azr_reg(env, "__collections_deque_popleft", b_deque_popleft);
    azr_reg(env, "__collections_deque_rotate", b_deque_rotate);
    azr_reg(env, "__collections_counter_elements", b_counter_elements);
    azr_reg(env, "__collections_counter_total", b_counter_total);
    azr_reg(env, "__argparse_parse", b_parse_args); azr_reg(env, "__argparse_parse_known", b_parse_known_args);
    azr_reg(env, "__logging_log", b_log);
    azr_reg(env, "__logging_config", b_log_config); azr_reg(env, "__thread_active_count", b_thread_count);
}
