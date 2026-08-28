#include "common.h"

static const char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static Value *b_b64_encode(Value **args, int argc) {
    const unsigned char *data = NULL; int n = 0, i = 0, k = 0; char *out = NULL; Value *v = NULL;
    if (argc < 1) return make_string("");
    if (args[0]->type == VAL_BYTES) { data = args[0]->bytes.data; n = args[0]->bytes.count; }
    else { char *s = value_to_string(args[0]); data = (unsigned char *)s; n = (int)strlen(s); }
    out = (char *)calloc((size_t)((n + 2) / 3 * 4 + 1), 1);
    for (i = 0; i < n; i += 3) {
        int a = data[i], b = i + 1 < n ? data[i + 1] : 0, c = i + 2 < n ? data[i + 2] : 0;
        out[k++] = b64[a >> 2];
        out[k++] = b64[((a & 3) << 4) | (b >> 4)];
        out[k++] = i + 1 < n ? b64[((b & 15) << 2) | (c >> 6)] : '=';
        out[k++] = i + 2 < n ? b64[c & 63] : '=';
    }
    v = make_string(out); free(out);
    if (argc && args[0]->type != VAL_BYTES) free((void *)data);
    return v;
}

static int b64_index(char c) {
    const char *p = strchr(b64, c);
    return p ? (int)(p - b64) : -1;
}

static Value *b_b64_decode(Value **args, int argc) {
    const char *s = azr_str_arg(args, argc, 0, "");
    int len = (int)strlen(s), i = 0, k = 0;
    unsigned char *out = (unsigned char *)calloc((size_t)len, 1);
    for (i = 0; i < len;) {
        int a = b64_index(s[i++]), b = b64_index(s[i++]);
        int c = s[i] == '=' ? -1 : b64_index(s[i]); i++;
        int d = s[i] == '=' ? -1 : b64_index(s[i]); i++;
        if (a < 0 || b < 0) break;
        out[k++] = (unsigned char)((a << 2) | (b >> 4));
        if (c >= 0) out[k++] = (unsigned char)(((b & 15) << 4) | (c >> 2));
        if (d >= 0 && c >= 0) out[k++] = (unsigned char)(((c & 3) << 6) | d);
    }
    { Value *v = make_bytes(out, k); free(out); return v; }
}

static Value *b_b64_urlsafe_encode(Value **args, int argc) {
    Value *v = b_b64_encode(args, argc);
    if (v->type == VAL_STRING) {
        for (int i = 0; v->str_val[i]; i++) {
            if (v->str_val[i] == '+') v->str_val[i] = '-';
            else if (v->str_val[i] == '/') v->str_val[i] = '_';
        }
    }
    return v;
}

static Value *b_b64_urlsafe_decode(Value **args, int argc) {
    char *copy = azr_strdup(azr_str_arg(args, argc, 0, ""));
    Value *tmp_args[1];
    Value *out = NULL;
    for (int i = 0; copy[i]; i++) {
        if (copy[i] == '-') copy[i] = '+';
        else if (copy[i] == '_') copy[i] = '/';
    }
    tmp_args[0] = make_string(copy);
    out = b_b64_decode(tmp_args, 1);
    value_free(tmp_args[0]);
    free(copy);
    return out;
}

static unsigned long long fnv1a(const unsigned char *s, int n) {
    unsigned long long h = 1469598103934665603ULL; int i = 0;
    for (i = 0; i < n; i++) { h ^= s[i]; h *= 1099511628211ULL; }
    return h;
}

static Value *fallback_hex(Value **args, int argc, const char *salt) {
    char buf[65]; char *text = argc ? value_to_string(args[0]) : azr_strdup("");
    unsigned long long a = fnv1a((unsigned char *)text, (int)strlen(text));
    unsigned long long b = fnv1a((unsigned char *)salt, (int)strlen(salt)) ^ (a << 1);
    snprintf(buf, sizeof(buf), "%016llx%016llx%016llx%016llx", a, b, a ^ b, a + b);
    free(text); return make_string(buf);
}

static Value *hash_file_with_tool(const char *text, const char *algo, int chars) {
    char tmp[L_tmpnam + 8], cmd[512];
    FILE *f = NULL;
    char *out = NULL, *p = NULL, *hex = NULL;
    tmpnam(tmp);
    f = fopen(tmp, "wb");
    if (!f) return NULL;
    fwrite(text, 1, strlen(text), f); fclose(f);
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "certutil -hashfile \"%s\" %s 2>NUL", tmp, algo);
#else
    snprintf(cmd, sizeof(cmd), "%ssum \"%s\" 2>/dev/null", strcmp(algo, "MD5") == 0 ? "md5" : "sha256", tmp);
#endif
    out = azr_read_process(cmd);
    remove(tmp);
    for (p = out; *p; p++) {
        int n = 0;
        while (isxdigit((unsigned char)p[n])) n++;
        if (n >= chars) {
            hex = azr_strndup(p, chars);
            for (n = 0; hex[n]; n++) hex[n] = (char)tolower((unsigned char)hex[n]);
            free(out);
            { Value *v = make_string(hex); free(hex); return v; }
        }
    }
    free(out);
    return NULL;
}

static Value *hash_real(Value **args, int argc, const char *algo, int chars) {
    char *text = argc ? value_to_string(args[0]) : azr_strdup("");
    Value *v = hash_file_with_tool(text, algo, chars);
    free(text);
    if (v) return v;
    v = fallback_hex(args, argc, algo);
    if (chars == 32) {
        char *s = value_to_string(v);
        char *short_hex = azr_strndup(s, 32);
        value_free(v);
        v = make_string(short_hex);
        free(short_hex); free(s);
    }
    return v;
}

static Value *b_hash_sha256(Value **args, int argc) { return hash_real(args, argc, "SHA256", 64); }
static Value *b_hash_md5(Value **args, int argc) { return hash_real(args, argc, "MD5", 32); }
static Value *b_hash_sha1(Value **args, int argc) { return hash_real(args, argc, "SHA1", 40); }
static Value *b_hash_sha384(Value **args, int argc) { return hash_real(args, argc, "SHA384", 96); }
static Value *b_hash_sha512(Value **args, int argc) { return hash_real(args, argc, "SHA512", 128); }
static Value *b_hash_hash(Value **args, int argc) { return make_int((long long)fnv1a((unsigned char *)azr_str_arg(args, argc, 0, ""), (int)strlen(azr_str_arg(args, argc, 0, "")))); }

void azr_register_crypto(Environment *env) {
    azr_reg(env, "__base64_encode", b_b64_encode);
    azr_reg(env, "__base64_decode", b_b64_decode);
    azr_reg(env, "__base64_urlsafe_encode", b_b64_urlsafe_encode);
    azr_reg(env, "__base64_urlsafe_decode", b_b64_urlsafe_decode);
    azr_reg(env, "__hash_sha256", b_hash_sha256);
    azr_reg(env, "__hash_md5", b_hash_md5);
    azr_reg(env, "__hash_sha1", b_hash_sha1);
    azr_reg(env, "__hash_sha384", b_hash_sha384);
    azr_reg(env, "__hash_sha512", b_hash_sha512);
    azr_reg(env, "__hash_value", b_hash_hash);
}
