# Project Status

Honest assessment of Azravibe's current state. This page describes what works, what is experimental, and what is planned.

**Current version: v.0.1**

---

## Currently Available

These features are implemented and working.

### Language Core

- Tree-walk interpreter written in C (C11 standard)
- Indentation-based block syntax (like Python)
- Dynamic typing with 24 runtime value types
- UTF-8 source files with full Persian/Arabic character support
- Persian/Farsi keywords with English aliases (dual-language)
- Both Latin comma (`,`) and Persian comma (`،`) as separators

### Variables and Types

- Integer (64-bit signed)
- Float (64-bit double-precision)
- String (UTF-8 with interpolation)
- Boolean (`درسته`/`غلطه`)
- None (`هیچ`)
- Bytes

### Data Structures

- Lists (mutable, dynamic arrays)
- Tuples (immutable)
- Dictionaries (key-value, dot notation access)
- Sets
- List comprehensions

### Control Flow

- `if` / `elif` / `else` (`اگه` / `وگرنه اگه` / `وگرنه`)
- `while` loops (`تا وقتی`)
- `for-each` loops (`برای هر ... توی`)
- `break` / `continue`
- Ternary expressions

### Functions

- Function definitions with `کار`/`def`
- Default arguments
- Named arguments
- Varargs (`*args`)
- Closures
- Lambda expressions (`لامبدا`)
- Generators with `yield` (`تحویل`)

### Classes and OOP

- Class definitions with `نوع`/`class`
- Constructor (`سازنده`/`__init__`)
- Instance methods with implicit `خود`/`self`
- Inheritance (`از`/`extends`)
- `super()` support (`ابر()`)
- Magic/dunder methods (`__str__`, `__eq__`, `__call__`, `__getitem__`, `__enter__`, `__exit__`)
- Class attributes and methods

### Error Handling

- `try` / `except` / `finally` (`سعی کن` / `اگه خطا` / `بالاخره`)
- `raise` (`بینداز`)
- Built-in exception types (8 types + base error)

### Modules and Imports

- `import` (`بیار`)
- `from ... import` (`از ... بیار`)
- `import ... as` (`بیار ... به عنوان`)
- Package support (`__init__.azr` files)
- Project manifests (`azr.project`)
- Local package installation

### Standard Library (23 Modules)

| Module | Status |
|---|---|
| `math` / `ریاضی` | Fully implemented (26 C + 6 Azravibe functions) |
| `json` | Fully implemented (C parser, Azravibe wrappers) |
| `csv` | Fully implemented (C parser, Azravibe wrappers) |
| `datetime` / `زمان` | Fully implemented (4 C + 12 Azravibe functions) |
| `os` | Fully implemented (10 C + 3 Azravibe functions) |
| `path` | Fully implemented (8 C + 4 Azravibe functions) |
| `sys` | Implemented (2 C + 2 Azravibe functions, some hardcoded stubs) |
| `random` / `تصادفی` | Fully implemented (10 C + 3 Azravibe functions) |
| `base64` | Fully implemented (4 C functions) |
| `hashlib` | Implemented (6 C functions, OS-tool dependent) |
| `http` | Implemented (6 C functions, requires `curl`) |
| `url` | Fully implemented (5 C + 7 Azravibe functions) |
| `sqlite` | Implemented (3 C + 5 Azravibe functions, requires `sqlite3` CLI) |
| `socket` | Implemented (3 C + 2 Azravibe functions, shell-based) |
| `subprocess` | Implemented (3 C + 2 Azravibe functions) |
| `collections` | Fully implemented (8 C + 8 Azravibe functions) |
| `logging` | Implemented (2 C + 6 Azravibe functions) |
| `argparse` | Implemented (2 C + 2 Azravibe functions) |
| `متن` (text) | Fully implemented (3 pure Azravibe functions) |
| `لیست` (list) | Fully implemented (3 pure Azravibe functions) |
| `threading` | Stub (no real threading) |
| `فایل` (file) | Stub (use built-in `فایل_بخوان`/`فایل_بنویس` instead) |
| `collections` | Fully implemented |

### Built-in Functions

52 built-in functions covering I/O, type conversion, math, string operations, list operations, dict operations, set operations, file I/O, and system operations.

### CLI

- Run files (`azravibe file.azr [--debug]`)
- Test runner (`azravibe --test <folder>`)
- REPL (interactive mode)
- Project initialization (`azravibe --pkg-init <dir> <name> [version]`)
- Local package installation (`azravibe --pkg-install <dir> <local_path>`)
- GUI REPL mode (`--gui-repl`, internal)
- Help and version output

### IDE

- Electron-based desktop application
- Full RTL (right-to-left) support
- Persian/English UI toggle
- Syntax highlighting for `.azr` files (46 keywords, 52 builtins)
- Autocomplete for keywords and builtins
- Integrated terminal (xterm.js + node-pty)
- File explorer with context menus, drag-and-drop, inline creation
- Tab management with drag reorder
- Workspace search and replace
- Outline panel (code symbols)
- Diagnostics (basic bracket/string matching)
- Custom minimap
- Three color themes
- Splash screen
- Multiple terminal instances
- Settings panel
- Command palette and quick open
- Console/REPL mode with chat-style UI
- File association (`.azr` files open in IDE)
- Keyboard shortcuts (40+ shortcuts)

### Native C Extension API

- Load `.dll`/`.so`/`.dylib` modules at runtime
- Stable ABI with `AzrNativeAPI` function table
- Documented in `docs/NATIVE_EXTENSIONS.md`

### Platform Support

- Windows (primary development platform)
- Linux (builds with GCC/Clang)
- macOS (builds with Clang)

### Installer

- Inno Setup wizard for Windows
- PowerShell installer for Windows
- Makefile install target for Linux/macOS

---

## Experimental

These features work but may change, have limitations, or need further testing.

### String Interpolation

String interpolation (`"text {expr}"`) works but is implemented via re-lexing and re-parsing at runtime. Complex expressions inside interpolation may have edge cases.

### Comprehensions

List comprehensions (`[x for x in items]`) are implemented. Dict and set comprehensions are partially supported in the AST but may not work correctly in all cases.

### `with` Statement

The context manager (`با ... به عنوان ...`) works but requires objects to define `ورود`/`خروج` or `__enter__`/`__exit__` methods. Not all edge cases are handled.

### Hash Functions (hashlib)

Hash functions depend on OS tools (`certutil` on Windows, `sha256sum`/`md5sum` on Unix). If these tools are unavailable, a non-cryptographic FNV-1a fallback is used. Do not rely on these for security purposes.

### HTTP Module

The HTTP module shells out to `curl`. It works for basic requests but is not a native HTTP implementation. Requires `curl` to be installed.

### SQLite Module

SQLite operations shell out to the `sqlite3` CLI. Requires `sqlite3` to be installed. Not a native SQLite binding.

### Socket Module

Socket operations use OS commands (PowerShell on Windows, `nc`/`getent` on Unix) for connectivity checks and DNS resolution. Not a raw socket API.

### Project System

The `azr.project` manifest and local package system works for simple cases. It does not support version resolution, dependency graphs, or remote packages.

---

## Known Limitations

### Interpreter Performance

The interpreter is a tree-walk interpreter (no bytecode compilation). Performance is adequate for small to medium programs but not suitable for compute-heavy workloads.

### Threading

The `threading` module is a stub. `Thread()` creates a dict object but does not run anything. `active_count()` always returns 1. There is no real thread support.

### File Module (`فایل.azr`)

The `فایل` stdlib module is a complete stub that only returns a description string. Use the built-in `فایل_بخوان`/`فایل_بنویس` functions instead.

### Slicing

Slicing supports `[start:end]` but does not support a step parameter (`[start:end:step]`).

### Memory Management

Uses reference counting. Circular references are not detected and will cause memory leaks.

### Error Messages

Error messages are in Farsi but do not include source context (surrounding code lines). Stack traces are basic.

### Environment Lookups

Variable lookups in the environment use linear array scanning (O(n) per lookup). This is not a performance concern for typical programs but is worth noting.

---

## What Is NOT Implemented

The following features do not exist yet:

- Bytecode VM / compiler (currently tree-walk only)
- Real threading or async/await
- Native sockets or networking
- Native SQLite bindings
- GUI application development
- Web server framework
- Package manager for remote packages
- Language server protocol (LSP)
- Debugger
- REPL history persistence
- Regular expressions (mentioned in docs but not implemented in current build)
- Advanced type system or static analysis
- Optimization passes
- Cross-compilation
- Plugin marketplace
