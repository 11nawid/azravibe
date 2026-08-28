# Contributing to Azravibe

Thank you for your interest in contributing to Azravibe. This guide explains how to get started.

---

## Ways to Contribute

- **Report bugs** -- File an issue on GitHub
- **Suggest features** -- Open a feature request issue
- **Fix bugs** -- Submit a pull request with a fix
- **Add tests** -- Write tests for untested features
- **Improve documentation** -- Fix typos, add examples, clarify explanations
- **Add examples** -- Create working Azravibe programs
- **Build tools** -- Create editor plugins, linters, or other tooling

---

## Reporting Bugs

When reporting a bug, please include:

1. **Operating system** and version (e.g., Windows 11, Ubuntu 22.04, macOS 14)
2. **Azravibe version** (`azravibe --version`)
3. **Steps to reproduce** the issue
4. **Expected behavior** -- What you expected to happen
5. **Actual behavior** -- What actually happened
6. **Minimal code example** -- The smallest `.azr` file that reproduces the bug
7. **Error message** -- The exact error output

File issues at: [GitHub Issues](https://github.com/azravibe/azravibe/issues)

---

## Suggesting Features

Feature requests are welcome. Please describe:

1. **The problem** you are trying to solve
2. **Your proposed solution**
3. **Alternatives** you considered
4. **Current workaround** (if any)

---

## Setting Up the Development Environment

### Prerequisites

- C11 compiler (GCC, Clang, or MinGW)
- Make or CMake
- Git
- For IDE work: Node.js 18+ and npm

### Building the Language

```bash
git clone https://github.com/azravibe/azravibe.git
cd azravibe/azravibe_lang_v1
make
```

Verify the build:

```bash
./azravibe --version
./azravibe --test tests
```

### Building the IDE

```bash
cd ../azravibe_ide
npm install
npm run dev
```

---

## Project Architecture

### Language Interpreter (`azravibe_lang_v1/`)

The interpreter follows a classic pipeline:

```
Source File → Lexer → Tokens → Parser → AST → Interpreter → Output
```

Key source files:

| File | Purpose |
|---|---|
| `src/lexer.c` | Tokenizer (handles Persian UTF-8 keywords) |
| `src/parser.c` | Recursive-descent parser |
| `src/ast.c` | Abstract syntax tree nodes |
| `src/interpreter.c` | Interpreter shell (includes .inc files) |
| `src/interpreter_statements.inc` | Statement execution |
| `src/interpreter_expressions.inc` | Expression evaluation |
| `src/interpreter_iteration.inc` | Iterators, generators, f-strings |
| `src/interpreter_members.inc` | Member access, method calls, classes |
| `src/interpreter_runtime.inc` | Runtime plumbing, error handling |
| `src/value.c` | Runtime value types |
| `src/environment.c` | Variable scoping |
| `src/builtins.c` | Built-in functions |
| `src/main.c` | CLI entry point |
| `src/project.c` | Project manifest and package management |
| `src/error.c` | Error message factories |
| `src/native.c` | Native extension loading |
| `src/version.c` | Version detection |
| `src/utf8.c` | UTF-8 utilities |
| `src/console.c` | Console output with Persian font support |
| `src/stdlib/` | Native C implementations for stdlib modules |

### IDE (`azravibe_ide/`)

Built with Electron + React + TypeScript:

| Directory | Purpose |
|---|---|
| `src/main/` | Electron main process |
| `src/preload/` | Preload bridge (context isolation) |
| `src/renderer/` | React UI application |
| `src/renderer/src/components/` | UI components |
| `src/renderer/src/components/code-editor/` | CodeMirror integration |
| `src/renderer/src/store/` | Zustand state management |

---

## Making Changes

### Language Changes

1. Modify the relevant C source file in `src/`
2. Rebuild with `make`
3. Test your changes with existing tests: `./azravibe --test tests`
4. Add a new test file in `tests/` for your feature
5. Ensure the build is warning-clean: `make` should produce no warnings with `-Wall -Wextra -std=c11`

### IDE Changes

1. Navigate to `azravibe_ide/`
2. Run `npm run dev` for development mode with hot reload
3. Test in the development environment
4. Ensure `npm run build` succeeds without errors

### Documentation Changes

Documentation lives in the `docs/` directory. Follow the existing style:
- Clear headings
- Working code examples
- Consistent terminology (use the actual Persian keywords)
- No made-up features

---

## Coding Standards

### C Code

- Standard: C11
- Compiler flags: `-Wall -Wextra -std=c11`
- No warnings allowed in the build
- Persian strings in error messages
- UTF-8 encoding for all source files
- No external dependencies beyond standard C library and platform APIs

### TypeScript/React Code

- TypeScript in strict mode
- React functional components
- Zustand for state management
- Tailwind CSS for styling
- ESLint conventions

### General

- No secrets, API keys, or personal information in code
- No committed binary files (use releases)
- No generated files in version control

---

## Pull Request Process

1. **Fork** the repository
2. **Create a branch** for your change (`git checkout -b my-feature`)
3. **Make your changes** following the coding standards
4. **Test** your changes thoroughly
5. **Commit** with a clear message describing the change
6. **Push** to your fork
7. **Open a pull request** with:
   - A description of what the change does
   - Why the change is needed
   - How to test the change
   - Any related issues

### Commit Messages

Write clear, descriptive commit messages:

```
Add string reverse function to متن module

Adds a reverse() function that reverses a string by iterating
characters and prepending. Includes a test file.
```

---

## Adding Tests

Create a new `.azr` file in `azravibe_lang_v1/tests/`:

```azr
# test_descriptive_name.azr
# Tests [feature description]

[... test code ...]
بنویس(result)
# Expected: [expected output]
```

Test files should:
- Have a descriptive filename starting with `test_`
- Test one feature or behavior
- Print output that can be verified
- Include a comment explaining the expected output

---

## Code of Conduct

Please be respectful and constructive in all interactions. See [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md).

---

## Questions?

Open a [GitHub Discussion](https://github.com/azravibe/azravibe/discussions) or file an issue.
