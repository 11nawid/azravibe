#ifndef AZR_UTF8_H
#define AZR_UTF8_H

#include <stdint.h>

int utf8_charlen(unsigned char byte);
int utf8_strlen(const char *str);
uint32_t utf8_codepoint(const char *str, int *bytes_read);
int utf8_is_farsi(uint32_t codepoint);
int utf8_match(const char *src, const char *keyword);
char *azr_strdup(const char *s);
char *azr_strndup(const char *s, int n);

#endif
