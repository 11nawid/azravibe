# Changelog

All notable changes to Azravibe will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/), and this project adheres to [Semantic Versioning](https://semver.org/).

---

## [0.1.0] - 2026-08-28

### Initial Public Release

This is the first public release of Azravibe. It is early-stage software intended for evaluation, experimentation, and early adoption.

#### Language

- Tree-walk interpreter written in C (C11)
- Persian/Farsi keywords with English aliases
- Indentation-based block syntax
- Dynamic typing with 24 runtime value types
- Full UTF-8 support with Persian character handling
- Both Latin and Persian comma as separators

#### Data Types

- Integer (64-bit), Float, String, Boolean, None, Bytes
- Lists, Tuples, Dictionaries, Sets
- List comprehensions

#### Control Flow

- if / elif / else
- while loops
- for-each loops with range()
- break / continue
- Ternary expressions

#### Functions

- Function definitions with default and named arguments
- Varargs (*args)
- Closures
- Lambda expressions
- Generators with yield

#### Classes

- Class definitions with inheritance
- Constructors (سازنده / __init__)
- Implicit self (خود)
- super() support
- Magic/dunder methods (__str__, __eq__, __call__, __getitem__, __enter__, __exit__)

#### Error Handling

- try / except / finally
- raise
- 8 built-in exception types

#### Modules and Imports

- import, from-import, import-as
- Package support (__init__.azr)
- Project manifests (azr.project)
- Local package installation

#### Standard Library (23 modules)

- math / ریاضی, json, csv, datetime / زمان, os, path, sys
- random / تصادفی, base64, hashlib, http, url, sqlite
- socket, subprocess, collections, logging, argparse
- متن (text), لیست (list), فایل (file), threading

#### CLI

- File execution, inline code execution, REPL
- Project initialization and local package installation
- Help and version output

#### IDE

- Electron + React + TypeScript desktop application
- Full RTL support
- Syntax highlighting (46 keywords, 52 builtins)
- Autocomplete
- Integrated terminal (xterm.js + node-pty)
- File explorer with context menus and drag-drop
- Workspace search and replace
- Diagnostics panel
- Custom minimap
- Three color themes
- Console/REPL mode
- 40+ keyboard shortcuts
- Settings panel

#### Native C Extension API

- Load .dll/.so/.dylib modules at runtime
- Stable ABI with AzrNativeAPI

#### Platform Support

- Windows (primary)
- Linux
- macOS

#### Known Limitations

- Tree-walk interpreter (no bytecode VM)
- No real threading
- No native socket or SQLite bindings
- HTTP module requires curl
- Minimal test suite
- Basic error messages without source context
- Memory leaks with circular references
