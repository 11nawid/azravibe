#include "console.h"

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <wchar.h>
#endif

#ifdef _WIN32
static void azr_console_try_apply_font(HANDLE handle) {
    CONSOLE_FONT_INFOEX font_info;

    if (handle == INVALID_HANDLE_VALUE || handle == NULL) return;

    ZeroMemory(&font_info, sizeof(font_info));
    font_info.cbSize = sizeof(font_info);
    if (!GetCurrentConsoleFontEx(handle, FALSE, &font_info)) return;

    if (wcscmp(font_info.FaceName, L"Vazir Code") == 0) return;

    wcscpy_s(font_info.FaceName, LF_FACESIZE, L"Vazir Code");
    font_info.FontFamily = FF_MODERN;
    if (font_info.dwFontSize.X < 10) font_info.dwFontSize.X = 10;
    if (font_info.dwFontSize.Y < 22) font_info.dwFontSize.Y = 22;
    SetCurrentConsoleFontEx(handle, FALSE, &font_info);
}
#endif

static int azr_console_write_bytes(FILE *stream, const char *text, int len) {
    if (!text) return 0;
    if (len < 0) len = (int)strlen(text);

#ifdef _WIN32
    if (stream == stdout || stream == stderr) {
        HANDLE handle = GetStdHandle(stream == stdout ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
        DWORD mode = 0;
        if (handle != INVALID_HANDLE_VALUE && handle != NULL && GetConsoleMode(handle, &mode)) {
            int wide_len = MultiByteToWideChar(CP_UTF8, 0, text, len, NULL, 0);
            if (wide_len > 0) {
                WCHAR *wide = (WCHAR *)malloc((size_t)wide_len * sizeof(WCHAR));
                DWORD written = 0;
                if (wide) {
                    MultiByteToWideChar(CP_UTF8, 0, text, len, wide, wide_len);
                    if (WriteConsoleW(handle, wide, (DWORD)wide_len, &written, NULL)) {
                        free(wide);
                        return len;
                    }
                    free(wide);
                }
            }
        }
    }
#endif

    return (int)fwrite(text, 1, (size_t)len, stream);
}

void azr_console_init(void) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    azr_console_try_apply_font(GetStdHandle(STD_OUTPUT_HANDLE));
    azr_console_try_apply_font(GetStdHandle(STD_ERROR_HANDLE));
#endif
}

int azr_console_vprintf(FILE *stream, const char *format, va_list args) {
    va_list copy;
    int needed = 0;
    char *buffer = NULL;
    int written = 0;

    va_copy(copy, args);
    needed = vsnprintf(NULL, 0, format, copy);
    va_end(copy);
    if (needed < 0) return needed;

    buffer = (char *)malloc((size_t)needed + 1);
    if (!buffer) return -1;

    vsnprintf(buffer, (size_t)needed + 1, format, args);
    written = azr_console_write_bytes(stream, buffer, needed);
    free(buffer);
    return written;
}

int azr_console_printf(FILE *stream, const char *format, ...) {
    va_list args;
    int written = 0;
    va_start(args, format);
    written = azr_console_vprintf(stream, format, args);
    va_end(args);
    return written;
}
