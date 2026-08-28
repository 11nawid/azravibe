CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -O2
UNAME_S := $(shell uname -s 2>/dev/null)
ifeq ($(OS),Windows_NT)
LDLIBS = -lm
else ifneq (,$(findstring MINGW,$(UNAME_S)))
LDLIBS = -lm
else ifneq (,$(findstring MSYS,$(UNAME_S)))
LDLIBS = -lm
else
LDLIBS = -lm -ldl
endif
SRCS = src/main.c src/lexer.c src/parser.c src/ast.c \
       src/interpreter.c src/environment.c src/value.c \
       src/builtins.c src/error.c src/utf8.c src/version.c src/project.c src/native.c src/console.c \
       src/stdlib/stdlib_native.c src/stdlib/process_utils.c \
       src/stdlib/json_native.c src/stdlib/csv_native.c \
       src/stdlib/crypto_native.c src/stdlib/os_path_native.c \
       src/stdlib/net_process_native.c src/stdlib/data_time_native.c
TARGET = azravibe.exe
COMPAT_TARGET = azr.exe

all: $(TARGET) $(COMPAT_TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDLIBS)

$(COMPAT_TARGET): $(TARGET)
	cp $(TARGET) $(COMPAT_TARGET)

clean:
	rm -f $(TARGET) $(COMPAT_TARGET)

test: $(TARGET)
	./$(TARGET) --test tests

install: $(TARGET) $(COMPAT_TARGET)
	cp $(TARGET) /usr/local/bin/azravibe
	cp $(COMPAT_TARGET) /usr/local/bin/azr

.PHONY: all clean test install
