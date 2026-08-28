#include "common.h"

static int is_unreserved(int c) {
    return isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~';
}

static Value *b_url_encode(Value **args, int argc) {
    const unsigned char *s = (const unsigned char *)azr_str_arg(args, argc, 0, "");
    int cap = (int)strlen((const char *)s) * 3 + 1, k = 0;
    char *out = (char *)calloc((size_t)cap, 1);
    for (; *s; s++) {
        if (is_unreserved(*s)) out[k++] = (char)*s;
        else { snprintf(out + k, 4, "%%%02X", *s); k += 3; }
    }
    { Value *v = make_string(out); free(out); return v; }
}

static int hexv(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static Value *b_url_decode(Value **args, int argc) {
    const char *s = azr_str_arg(args, argc, 0, "");
    char *out = (char *)calloc(strlen(s) + 1, 1); int k = 0;
    for (; *s; s++) {
        if (*s == '%' && hexv(s[1]) >= 0 && hexv(s[2]) >= 0) {
            out[k++] = (char)((hexv(s[1]) << 4) | hexv(s[2])); s += 2;
        } else out[k++] = *s == '+' ? ' ' : *s;
    }
    { Value *v = make_string(out); free(out); return v; }
}

static Value *b_url_parse(Value **args, int argc) {
    const char *url = azr_str_arg(args, argc, 0, "");
    const char *scheme_end = strstr(url, "://");
    const char *host_start = scheme_end ? scheme_end + 3 : url;
    const char *path_start = strchr(host_start, '/');
    const char *query_start = strchr(host_start, '?');
    const char *frag_start = strchr(host_start, '#');
    Value *out = make_dict();
    if (scheme_end) dict_set(out, "scheme", make_string(azr_strndup(url, (int)(scheme_end - url))));
    else dict_set(out, "scheme", make_string(""));
    if (frag_start) dict_set(out, "fragment", make_string(frag_start + 1));
    else dict_set(out, "fragment", make_string(""));
    if (!path_start || (query_start && query_start < path_start)) path_start = query_start;
    if (!path_start || (frag_start && frag_start < path_start)) path_start = frag_start;
    if (path_start) dict_set(out, "host", make_string(azr_strndup(host_start, (int)(path_start - host_start))));
    else dict_set(out, "host", make_string(host_start));
    dict_set(out, "netloc", value_copy(dict_get(out, "host")));
    if (path_start && *path_start == '/') {
        const char *q = strchr(path_start, '?');
        const char *f = strchr(path_start, '#');
        const char *end = q ? q : (f ? f : path_start + strlen(path_start));
        dict_set(out, "path", make_string(azr_strndup(path_start, (int)(end - path_start))));
        if (q) dict_set(out, "query", make_string(azr_strndup(q + 1, (int)((f ? f : q + strlen(q)) - q - 1))));
        else dict_set(out, "query", make_string(""));
    } else {
        dict_set(out, "path", make_string(""));
        if (query_start) dict_set(out, "query", make_string(azr_strndup(query_start + 1, (int)((frag_start ? frag_start : query_start + strlen(query_start)) - query_start - 1))));
        else dict_set(out, "query", make_string(""));
    }
    return out;
}

static Value *b_url_parse_qsl(Value **args, int argc) {
    const char *s = azr_str_arg(args, argc, 0, "");
    Value *out = make_list();
    char *buf = azr_strdup(s), *p = buf;
    while (p && *p) {
        char *amp = strchr(p, '&'), *eq = strchr(p, '='), *k = NULL, *v = NULL;
        if (amp) *amp = 0;
        if (eq) { *eq = 0; k = p; v = eq + 1; } else { k = p; v = ""; }
        Value *pair = make_list();
        Value *ka[1] = { make_string(k) }, *va[1] = { make_string(v) };
        list_append(pair, b_url_decode(ka, 1)); list_append(pair, b_url_decode(va, 1));
        value_free(ka[0]); value_free(va[0]); list_append(out, pair);
        p = amp ? amp + 1 : NULL;
    }
    free(buf); return out;
}

static Value *b_url_parse_qs(Value **args, int argc) {
    Value *pairs = b_url_parse_qsl(args, argc), *out = make_dict();
    for (int i = 0; i < pairs->list.count; i++) {
        Value *pair = pairs->list.items[i], *bucket = NULL;
        if (pair->type != VAL_LIST || pair->list.count < 2 || pair->list.items[0]->type != VAL_STRING) continue;
        bucket = dict_get(out, pair->list.items[0]->str_val);
        if (!bucket) { bucket = make_list(); dict_set(out, pair->list.items[0]->str_val, bucket); }
        list_append(bucket, value_copy(pair->list.items[1]));
    }
    value_free(pairs); return out;
}

static Value *run_capture(Value **args, int argc, const char *prefix) {
    char *q = NULL, *cmd = NULL, *out = NULL; Value *v = NULL;
    const char *target = azr_str_arg(args, argc, 0, "");
    q = azr_shell_quote(target);
    cmd = (char *)calloc(strlen(prefix) + strlen(q) + 32, 1);
    sprintf(cmd, "%s %s", prefix, q);
    out = azr_read_process(cmd);
    v = make_string(out);
    free(q); free(cmd); free(out);
    return v;
}

static Value *b_http_get(Value **args, int argc) {
    return run_capture(args, argc, "curl -L -s");
}

static Value *b_http_post(Value **args, int argc) {
    char *url = azr_shell_quote(azr_str_arg(args, argc, 0, ""));
    char *body = azr_shell_quote(azr_str_arg(args, argc, 1, ""));
    char *cmd = (char *)calloc(strlen(url) + strlen(body) + 80, 1);
    char *out = NULL; Value *v = NULL;
    sprintf(cmd, "curl -L -s -X POST -H \"Content-Type: application/json\" -d %s %s", body, url);
    out = azr_read_process(cmd); v = make_string(out);
    free(url); free(body); free(cmd); free(out); return v;
}

static Value *b_http_request(Value **args, int argc) {
    char *method = azr_shell_quote(azr_str_arg(args, argc, 0, "GET"));
    char *url = azr_shell_quote(azr_str_arg(args, argc, 1, ""));
    char *body = azr_shell_quote(azr_str_arg(args, argc, 2, ""));
    char *cmd = (char *)calloc(strlen(method) + strlen(url) + strlen(body) + 96, 1);
    char *out = NULL; Value *v = NULL;
    sprintf(cmd, "curl -L -s -X %s -H \"Content-Type: application/json\" -d %s %s", method, body, url);
    out = azr_read_process(cmd); v = make_string(out);
    free(method); free(url); free(body); free(cmd); free(out); return v;
}

static Value *b_http_head(Value **args, int argc) {
    return run_capture(args, argc, "curl -L -s -I");
}

static Value *b_http_status(Value **args, int argc) {
    char *url = azr_shell_quote(azr_str_arg(args, argc, 0, ""));
    char *cmd = (char *)calloc(strlen(url) + 80, 1);
    char *out = NULL; Value *v = NULL;
    sprintf(cmd, "curl -L -s -o NUL -w %%{http_code} %s", url);
    out = azr_read_process(cmd); v = make_int(strtoll(out, NULL, 10));
    free(url); free(cmd); free(out); return v;
}

static Value *b_http_download(Value **args, int argc) {
    char *url = azr_shell_quote(azr_str_arg(args, argc, 0, ""));
    char *path = azr_shell_quote(azr_str_arg(args, argc, 1, ""));
    char *cmd = (char *)calloc(strlen(url) + strlen(path) + 64, 1);
    int code = 0;
    sprintf(cmd, "curl -L -s -o %s %s", path, url);
    code = system(cmd);
    free(url); free(path); free(cmd); return make_bool(code == 0);
}

static Value *b_socket_tcp_check(Value **args, int argc) {
    char cmd[512];
    const char *host = azr_str_arg(args, argc, 0, "127.0.0.1");
    long long port = azr_int_arg(args, argc, 1, 80);
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "powershell -NoProfile -Command \"try{$c=New-Object Net.Sockets.TcpClient('%s',%lld);$c.Close();'1'}catch{'0'}\"", host, port);
#else
    snprintf(cmd, sizeof(cmd), "nc -z %s %lld >/dev/null 2>&1 && echo 1 || echo 0", host, port);
#endif
    { char *out = azr_read_process(cmd); Value *v = make_bool(out[0] == '1'); free(out); return v; }
}

static Value *b_socket_resolve(Value **args, int argc) {
    char cmd[512];
    const char *host = azr_str_arg(args, argc, 0, "localhost");
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "powershell -NoProfile -Command \"[Net.Dns]::GetHostAddresses('%s')[0].IPAddressToString\"", host);
#else
    snprintf(cmd, sizeof(cmd), "getent hosts %s | awk '{print $1; exit}'", host);
#endif
    { char *out = azr_read_process(cmd); out[strcspn(out, "\r\n")] = 0; Value *v = make_string(out); free(out); return v; }
}

static Value *b_socket_hostname(Value **args, int argc) {
    (void)args; (void)argc;
#ifdef _WIN32
    { char *out = azr_read_process("hostname"); out[strcspn(out, "\r\n")] = 0; Value *v = make_string(out); free(out); return v; }
#else
    { char *out = azr_read_process("hostname"); out[strcspn(out, "\r\n")] = 0; Value *v = make_string(out); free(out); return v; }
#endif
}

static Value *b_sqlite_query(Value **args, int argc) {
    char *db = azr_shell_quote(azr_str_arg(args, argc, 0, ""));
    char *sql = azr_shell_quote(azr_str_arg(args, argc, 1, ""));
    char *cmd = (char *)calloc(strlen(db) + strlen(sql) + 80, 1);
    char *out = NULL; Value *v = NULL;
    sprintf(cmd, "sqlite3 -header -csv %s %s", db, sql);
    out = azr_read_process(cmd); v = make_string(out);
    free(db); free(sql); free(cmd); free(out); return v;
}

static Value *b_sqlite_exec(Value **args, int argc) {
    Value *out = b_sqlite_query(args, argc);
    value_free(out);
    return make_bool(1);
}

static Value *b_sqlite_tables(Value **args, int argc) {
    char *db = azr_shell_quote(azr_str_arg(args, argc, 0, ""));
    char *cmd = (char *)calloc(strlen(db) + 96, 1);
    char *raw = NULL, *p = NULL, *line = NULL;
    Value *out = make_list();
    sprintf(cmd, "sqlite3 -list %s \"select name from sqlite_master where type='table' order by name;\"", db);
    raw = azr_read_process(cmd);
    p = raw;
    while (*p) {
        line = p;
        while (*p && *p != '\r' && *p != '\n') p++;
        if (*p) { *p = 0; p++; if (*p == '\n') p++; }
        if (line[0]) list_append(out, make_string(line));
    }
    free(db); free(cmd); free(raw);
    return out;
}

static Value *b_subprocess_run(Value **args, int argc) {
    return make_int(system(azr_str_arg(args, argc, 0, "")));
}

static Value *b_subprocess_output(Value **args, int argc) {
    char *out = azr_read_process(azr_str_arg(args, argc, 0, ""));
    Value *v = make_string(out); free(out); return v;
}

static Value *b_subprocess_result(Value **args, int argc) {
    const char *cmd_text = azr_str_arg(args, argc, 0, "");
    char *out = azr_read_process(cmd_text);
    Value *v = make_dict();
    dict_set(v, "args", make_string(cmd_text));
    dict_set(v, "stdout", make_string(out));
    dict_set(v, "stderr", make_string(""));
    dict_set(v, "returncode", make_int(0));
    dict_set(v, "ok", make_bool(1));
    free(out); return v;
}

static Value *b_thread_sleep(Value **args, int argc) {
    double seconds = azr_double_arg(args, argc, 0, 0.0);
#ifdef _WIN32
    char cmd[80]; snprintf(cmd, sizeof(cmd), "powershell -NoProfile -Command Start-Sleep -Milliseconds %lld", (long long)(seconds * 1000)); system(cmd);
#else
    char cmd[80]; snprintf(cmd, sizeof(cmd), "sleep %.3f", seconds); system(cmd);
#endif
    return make_none();
}

void azr_register_net_process(Environment *env) {
    azr_reg(env, "__url_encode", b_url_encode);
    azr_reg(env, "__url_decode", b_url_decode);
    azr_reg(env, "__url_parse", b_url_parse);
    azr_reg(env, "__url_parse_qs", b_url_parse_qs);
    azr_reg(env, "__url_parse_qsl", b_url_parse_qsl);
    azr_reg(env, "__http_get", b_http_get);
    azr_reg(env, "__http_post", b_http_post);
    azr_reg(env, "__http_request", b_http_request);
    azr_reg(env, "__http_head", b_http_head);
    azr_reg(env, "__http_status", b_http_status);
    azr_reg(env, "__http_download", b_http_download);
    azr_reg(env, "__socket_tcp_check", b_socket_tcp_check);
    azr_reg(env, "__socket_resolve", b_socket_resolve);
    azr_reg(env, "__socket_hostname", b_socket_hostname);
    azr_reg(env, "__sqlite_query", b_sqlite_query);
    azr_reg(env, "__sqlite_exec", b_sqlite_exec);
    azr_reg(env, "__sqlite_tables", b_sqlite_tables);
    azr_reg(env, "__subprocess_run", b_subprocess_run);
    azr_reg(env, "__subprocess_output", b_subprocess_output);
    azr_reg(env, "__subprocess_result", b_subprocess_result);
    azr_reg(env, "__thread_sleep", b_thread_sleep);
}
