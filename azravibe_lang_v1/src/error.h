#ifndef AZR_ERROR_H
#define AZR_ERROR_H

char *error_syntax(int line, const char *detail);
char *error_undefined(const char *name);
char *error_type(const char *detail);
char *error_div_zero(void);
char *error_index(int index, int len);
char *error_not_callable(const char *name);
char *error_wrong_args(const char *name, int expected, int got);
char *error_file_not_found(const char *filename);
char *error_import(const char *module);
void error_print_message(const char *message);

#endif
