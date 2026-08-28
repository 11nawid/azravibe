#include "console.h"
#include "error.h"
#include "utf8.h"

#include <stdio.h>
#include <stdlib.h>

/* Formats a heap string with a fixed maximum size for runtime errors. */
static char *fmt(const char *format, const char *a, int b, int c) {
    char buf[1024];
    snprintf(buf, sizeof(buf), format, a ? a : "", b, c);
    return azr_strdup(buf);
}

/* Creates a natural Farsi syntax error message. */
char *error_syntax(int line, const char *detail) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "خطای نوشتاری در خط %d: %s", line, detail ? detail : "دستور نامعتبره");
    return azr_strdup(buf);
}

/* Creates an undefined variable error message. */
char *error_undefined(const char *name) {
    return fmt("خطا: متغیر '%s' تعریف نشده", name, 0, 0);
}

/* Creates a type error message. */
char *error_type(const char *detail) {
    return fmt("خطای نوع: %s", detail, 0, 0);
}

/* Creates a division by zero error message. */
char *error_div_zero(void) {
    return azr_strdup("خطا: تقسیم بر صفر ممکن نیست");
}

/* Creates a list index range error message. */
char *error_index(int index, int len) {
    char buf[256];
    snprintf(buf, sizeof(buf), "خطا: اندیس %d از محدوده لیست خارجه (طول: %d)", index, len);
    return azr_strdup(buf);
}

/* Creates a non-callable value error message. */
char *error_not_callable(const char *name) {
    return fmt("خطا: '%s' قابل فراخوانی نیست", name, 0, 0);
}

/* Creates a wrong argument count error message. */
char *error_wrong_args(const char *name, int expected, int got) {
    char buf[512];
    snprintf(buf, sizeof(buf), "خطا: کار '%s' به %d آرگومان نیاز داره، %d داده شد", name, expected, got);
    return azr_strdup(buf);
}

/* Creates a file-not-found error message. */
char *error_file_not_found(const char *filename) {
    return fmt("خطا: فایل '%s' پیدا نشد", filename, 0, 0);
}

/* Creates an import failure error message. */
char *error_import(const char *module) {
    return fmt("خطا: نمیشه '%s' رو بیار کرد", module, 0, 0);
}

/* Prints an error message to stderr in Farsi. */
void error_print_message(const char *message) {
    azr_console_printf(stderr, "%s\n", message ? message : "خطای ناشناخته");
}
