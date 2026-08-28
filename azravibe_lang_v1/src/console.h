#ifndef AZR_CONSOLE_H
#define AZR_CONSOLE_H

#include <stdarg.h>
#include <stdio.h>

void azr_console_init(void);
int azr_console_printf(FILE *stream, const char *format, ...);
int azr_console_vprintf(FILE *stream, const char *format, va_list args);

#endif
