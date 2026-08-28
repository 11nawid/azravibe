#include "common.h"

typedef struct { const char *s; int pos; } JsonParser;

static void js_ws(JsonParser *p) { while (isspace((unsigned char)p->s[p->pos])) p->pos++; }

static char *js_parse_string(JsonParser *p) {
    char *out = NULL;
    int len = 0, cap = 32;
    if (p->s[p->pos] != '"') return azr_strdup("");
    out = (char *)calloc((size_t)cap, 1);
    p->pos++;
    while (p->s[p->pos] && p->s[p->pos] != '"') {
        char c = p->s[p->pos++];
        if (c == '\\') {
            c = p->s[p->pos++];
            if (c == 'n') c = '\n';
            else if (c == 't') c = '\t';
            else if (c == 'r') c = '\r';
            else if (c == 'b') c = '\b';
            else if (c == 'f') c = '\f';
        }
        if (len + 2 >= cap) { cap *= 2; out = (char *)realloc(out, (size_t)cap); }
        out[len++] = c; out[len] = 0;
    }
    if (p->s[p->pos] == '"') p->pos++;
    return out;
}

static Value *js_value(JsonParser *p);

static Value *js_array(JsonParser *p) {
    Value *out = make_list();
    p->pos++; js_ws(p);
    if (p->s[p->pos] == ']') { p->pos++; return out; }
    while (p->s[p->pos]) {
        list_append(out, js_value(p)); js_ws(p);
        if (p->s[p->pos] == ',') { p->pos++; js_ws(p); continue; }
        if (p->s[p->pos] == ']') { p->pos++; break; }
        break;
    }
    return out;
}

static Value *js_object(JsonParser *p) {
    Value *out = make_dict();
    p->pos++; js_ws(p);
    if (p->s[p->pos] == '}') { p->pos++; return out; }
    while (p->s[p->pos]) {
        char *key = js_parse_string(p);
        js_ws(p);
        if (p->s[p->pos] == ':') p->pos++;
        js_ws(p);
        dict_set(out, key, js_value(p));
        free(key); js_ws(p);
        if (p->s[p->pos] == ',') { p->pos++; js_ws(p); continue; }
        if (p->s[p->pos] == '}') { p->pos++; break; }
        break;
    }
    return out;
}

static Value *js_number(JsonParser *p) {
    int start = p->pos, is_float = 0;
    if (p->s[p->pos] == '-') p->pos++;
    while (isdigit((unsigned char)p->s[p->pos])) p->pos++;
    if (p->s[p->pos] == '.') { is_float = 1; p->pos++; while (isdigit((unsigned char)p->s[p->pos])) p->pos++; }
    if (p->s[p->pos] == 'e' || p->s[p->pos] == 'E') {
        is_float = 1; p->pos++;
        if (p->s[p->pos] == '+' || p->s[p->pos] == '-') p->pos++;
        while (isdigit((unsigned char)p->s[p->pos])) p->pos++;
    }
    {
        char *raw = azr_strndup(p->s + start, p->pos - start);
        Value *v = is_float ? make_float(strtod(raw, NULL)) : make_int(strtoll(raw, NULL, 10));
        free(raw); return v;
    }
}

static Value *js_value(JsonParser *p) {
    js_ws(p);
    if (p->s[p->pos] == '"') { char *s = js_parse_string(p); Value *v = make_string(s); free(s); return v; }
    if (p->s[p->pos] == '[') return js_array(p);
    if (p->s[p->pos] == '{') return js_object(p);
    if (strncmp(p->s + p->pos, "true", 4) == 0) { p->pos += 4; return make_bool(1); }
    if (strncmp(p->s + p->pos, "false", 5) == 0) { p->pos += 5; return make_bool(0); }
    if (strncmp(p->s + p->pos, "null", 4) == 0) { p->pos += 4; return make_none(); }
    return js_number(p);
}

static void js_append(char **out, int *len, int *cap, const char *s) {
    int add = (int)strlen(s);
    if (*len + add + 1 > *cap) { while (*len + add + 1 > *cap) *cap *= 2; *out = (char *)realloc(*out, (size_t)*cap); }
    memcpy(*out + *len, s, (size_t)add + 1); *len += add;
}

static void js_dump_value(Value *v, char **out, int *len, int *cap) {
    char buf[128]; int i = 0;
    if (!v || v->type == VAL_NONE) { js_append(out, len, cap, "null"); return; }
    if (v->type == VAL_BOOL) { js_append(out, len, cap, v->bool_val ? "true" : "false"); return; }
    if (v->type == VAL_INT) { snprintf(buf, sizeof(buf), "%lld", v->int_val); js_append(out, len, cap, buf); return; }
    if (v->type == VAL_FLOAT) { snprintf(buf, sizeof(buf), "%.15g", v->float_val); js_append(out, len, cap, buf); return; }
    if (v->type == VAL_STRING) {
        js_append(out, len, cap, "\"");
        for (i = 0; v->str_val[i]; i++) {
            char c[3] = { v->str_val[i], 0, 0 };
            if (v->str_val[i] == '"' || v->str_val[i] == '\\') { c[0] = '\\'; c[1] = v->str_val[i]; }
            else if (v->str_val[i] == '\n') { c[0] = '\\'; c[1] = 'n'; }
            js_append(out, len, cap, c);
        }
        js_append(out, len, cap, "\""); return;
    }
    if (v->type == VAL_LIST || v->type == VAL_TUPLE || v->type == VAL_SET) {
        int count = v->type == VAL_SET ? v->set.count : v->list.count;
        js_append(out, len, cap, "[");
        for (i = 0; i < count; i++) { if (i) js_append(out, len, cap, ","); js_dump_value(v->type == VAL_SET ? v->set.items[i] : v->list.items[i], out, len, cap); }
        js_append(out, len, cap, "]"); return;
    }
    if (v->type == VAL_DICT) {
        js_append(out, len, cap, "{");
        for (i = 0; i < v->dict.count; i++) {
            Value *k = make_string(v->dict.keys[i]);
            if (i) js_append(out, len, cap, ",");
            js_dump_value(k, out, len, cap); js_append(out, len, cap, ":"); js_dump_value(v->dict.values[i], out, len, cap);
            value_free(k);
        }
        js_append(out, len, cap, "}"); return;
    }
    { char *s = value_to_string(v); Value *tmp = make_string(s); free(s); js_dump_value(tmp, out, len, cap); value_free(tmp); }
}

static Value *b_json_loads(Value **args, int argc) {
    JsonParser p = { azr_str_arg(args, argc, 0, ""), 0 };
    return js_value(&p);
}

static Value *b_json_dumps(Value **args, int argc) {
    int len = 0, cap = 256; char *out = (char *)calloc((size_t)cap, 1); Value *v = NULL;
    if (argc > 0) js_dump_value(args[0], &out, &len, &cap);
    v = make_string(out); free(out); return v;
}

void azr_register_json(Environment *env) {
    azr_reg(env, "__json_loads", b_json_loads);
    azr_reg(env, "__json_dumps", b_json_dumps);
}
