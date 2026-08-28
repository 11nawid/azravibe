cmake_minimum_required(VERSION 3.16)
project(azravibe_lang_v1 C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(azravibe
    src/main.c src/lexer.c src/parser.c src/ast.c
    src/interpreter.c src/environment.c src/value.c
    src/builtins.c src/error.c src/utf8.c src/version.c src/project.c src/native.c src/console.c
    src/stdlib/stdlib_native.c src/stdlib/process_utils.c
    src/stdlib/json_native.c src/stdlib/csv_native.c
    src/stdlib/crypto_native.c src/stdlib/os_path_native.c
    src/stdlib/net_process_native.c src/stdlib/data_time_native.c)

if(UNIX AND NOT APPLE)
    target_link_libraries(azravibe PRIVATE m dl)
else()
    target_link_libraries(azravibe PRIVATE m)
endif()

target_compile_options(azravibe PRIVATE -Wall -Wextra)

if(WIN32)
    add_custom_command(
        TARGET azravibe POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:azravibe>
                $<TARGET_FILE_DIR:azravibe>/azr.exe)
endif()
