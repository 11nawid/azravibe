#include "common.h"

static Value *b_csv_parse(Value **args, int argc) {
    const char *s = azr_str_arg(args, argc, 0, "");
    Value *rows = make_list(), *row = make_list();
    char *cell = NULL; int len = 0, cap = 32, quoted = 0;
    cell = (char *)calloc((size_t)cap, 1);
    for (; *s; s++) {
        char c = *s;
        if (quoted) {
            if (c == '"' && s[1] == '"') { c = '"'; s++; }
            else if (c == '"') { quoted = 0; continue; }
        } else {
            if (c == '"') { quoted = 1; continue; }
            if (c == ',') { list_append(row, make_string(cell)); len = 0; cell[0] = 0; continue; }
            if (c == '\r') continue;
            if (c == '\n') {
                list_append(row, make_string(cell)); list_append(rows, row);
                row = make_list(); len = 0; cell[0] = 0; continue;
            }
        }
        if (len + 2 >= cap) { cap *= 2; cell = (char *)realloc(cell, (size_t)cap); }
        cell[len++] = c; cell[len] = 0;
    }
    if (len || row->list.count) { list_append(row, make_string(cell)); list_append(rows, row); }
    else value_free(row);
    free(cell);
    return rows;
}

static void add_text(char **out, int *len, int *cap, const char *s) {
    int add = (int)strlen(s);
    if (*len + add + 1 > *cap) { while (*len + add + 1 > *cap) *cap *= 2; *out = (char *)realloc(*out, (size_t)*cap); }
    memcpy(*out + *len, s, (size_t)add + 1); *len += add;
}

static void add_cell(char **out, int *len, int *cap, Value *v) {
    char *s = value_to_string(v); int quote = strpbrk(s, ",\"\n\r") != NULL; int i = 0;
    if (quote) add_text(out, len, cap, "\"");
    for (i = 0; s[i]; i++) {
        char c[3] = { s[i], 0, 0 };
        if (s[i] == '"') { c[0] = '"'; c[1] = '"'; }
        add_text(out, len, cap, c);
    }
    if (quote) add_text(out, len, cap, "\"");
    free(s);
}

static Value *b_csv_stringify(Value **args, int argc) {
    int cap = 256, len = 0, r = 0, c = 0;
    char *out = (char *)calloc((size_t)cap, 1);
    Value *rows = argc ? args[0] : NULL, *v = NULL;
    if (!rows || rows->type != VAL_LIST) { v = make_string(""); free(out); return v; }
    for (r = 0; r < rows->list.count; r++) {
        Value *row = rows->list.items[r];
        if (r) add_text(&out, &len, &cap, "\n");
        if (row->type == VAL_LIST || row->type == VAL_TUPLE) {
            for (c = 0; c < row->list.count; c++) {
                if (c) add_text(&out, &len, &cap, ",");
                add_cell(&out, &len, &cap, row->list.items[c]);
            }
        } else add_cell(&out, &len, &cap, row);
    }
    v = make_string(out); free(out); return v;
}

void azr_register_csv(Environment *env) {
    azr_reg(env, "__csv_parse", b_csv_parse);
    azr_reg(env, "__csv_stringify", b_csv_stringify);
}
