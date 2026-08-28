#include "common.h"

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

char *azr_read_process(const char *cmd) {
    FILE *p = POPEN(cmd, "r");
    char buf[512];
    size_t len = 0, cap = 1024;
    char *out = (char *)calloc(cap, 1);
    if (!p) return out;
    while (fgets(buf, sizeof(buf), p)) {
        size_t add = strlen(buf);
        if (len + add + 1 > cap) {
            while (len + add + 1 > cap) cap *= 2;
            out = (char *)realloc(out, cap);
        }
        memcpy(out + len, buf, add + 1);
        len += add;
    }
    PCLOSE(p);
    return out;
}

char *azr_shell_quote(const char *s) {
    size_t len = strlen(s), cap = len * 2 + 3, k = 1;
    char *out = (char *)calloc(cap, 1);
    size_t i = 0;
    out[0] = '"';
    for (i = 0; i < len; i++) {
        if (s[i] == '"' || s[i] == '\\') out[k++] = '\\';
        out[k++] = s[i];
    }
    out[k++] = '"';
    out[k] = 0;
    return out;
}
