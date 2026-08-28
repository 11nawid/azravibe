#include "utf8.h"

#include <stdlib.h>
#include <string.h>

/* Returns the byte length of the UTF-8 character beginning with byte. */
int utf8_charlen(unsigned char byte) {
    if ((byte & 0x80) == 0) return 1;
    if ((byte & 0xE0) == 0xC0) return 2;
    if ((byte & 0xF0) == 0xE0) return 3;
    if ((byte & 0xF8) == 0xF0) return 4;
    return 1;
}

/* Counts Unicode codepoints in a UTF-8 string rather than raw bytes. */
int utf8_strlen(const char *str) {
    int count = 0;
    int i = 0;
    while (str && str[i]) {
        i += utf8_charlen((unsigned char)str[i]);
        count++;
    }
    return count;
}

/* Decodes one UTF-8 codepoint and reports how many bytes were consumed. */
uint32_t utf8_codepoint(const char *str, int *bytes_read) {
    unsigned char b0 = (unsigned char)str[0];
    int len = utf8_charlen(b0);
    uint32_t cp = 0;
    if (len == 1) cp = b0;
    else if (len == 2) cp = ((uint32_t)(b0 & 0x1F) << 6) | (uint32_t)(str[1] & 0x3F);
    else if (len == 3) cp = ((uint32_t)(b0 & 0x0F) << 12) | ((uint32_t)(str[1] & 0x3F) << 6) | (uint32_t)(str[2] & 0x3F);
    else cp = ((uint32_t)(b0 & 0x07) << 18) | ((uint32_t)(str[1] & 0x3F) << 12) | ((uint32_t)(str[2] & 0x3F) << 6) | (uint32_t)(str[3] & 0x3F);
    if (bytes_read) *bytes_read = len;
    return cp;
}

/* Returns true for Arabic/Farsi Unicode blocks commonly used in Persian text. */
int utf8_is_farsi(uint32_t codepoint) {
    return (codepoint >= 0x0600 && codepoint <= 0x06FF) ||
           (codepoint >= 0xFB50 && codepoint <= 0xFDFF) ||
           (codepoint >= 0xFE70 && codepoint <= 0xFEFF);
}

/* Compares two UTF-8 strings by their byte sequence, which is correct for exact keywords. */
int utf8_match(const char *src, const char *keyword) {
    return strcmp(src, keyword) == 0;
}

/* Duplicates a C string with malloc for portable C11 builds. */
char *azr_strdup(const char *s) {
    if (!s) s = "";
    size_t n = strlen(s) + 1;
    char *out = (char *)malloc(n);
    if (out) memcpy(out, s, n);
    return out;
}

/* Duplicates exactly n bytes and appends a null terminator. */
char *azr_strndup(const char *s, int n) {
    if (n < 0) n = 0;
    char *out = (char *)malloc((size_t)n + 1);
    if (!out) return NULL;
    memcpy(out, s, (size_t)n);
    out[n] = '\0';
    return out;
}
