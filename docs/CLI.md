# Azravibe CLI Reference

Complete reference for the `azravibe` command-line interface.

---

## Synopsis

```
azravibe
azravibe <file.azr> [--debug]
azravibe --test <folder>
azravibe --pkg-init <dir> <name> [version]
azravibe --pkg-install <dir> <local_path>
azravibe --version
azravibe --help
azravibe --gui-repl
```

---

## Run a File

```bash
azravibe file.azr
azravibe path/to/script.azr
azravibe script.azr --debug
```

Runs the specified `.azr` source file. The interpreter reads, tokenizes, parses, and executes the program. Output is printed to stdout. Errors are printed with line numbers.

When a file runs inside a project directory (one containing `azr.project`), the interpreter automatically adds the project's package import roots to the module search path.

### The `--debug` flag

```bash
azravibe script.azr --debug
```

Enables debug mode for the run. This flag is passed as the second argument after the file name. (Currently it enables debug-level tracing within the interpreter; the exact behavior may be extended.)

---

## Test Runner (`--test`)

```bash
azravibe --test <folder>
```

Runs every `.azr` file in the given folder as a test. For each file it prints:

```
[قبول] test_name.azr    # passed
[خطا]  test_name.azr    # failed
```

At the end it prints a summary:

```
تست‌ها: N اجرا، M خطا
```

(meaning: "Tests: N run, M errors") and exits with code `1` if any test failed, `0` otherwise.

Example:

```bash
azravibe --test tests
```

---

## Project Initialization (`--pkg-init`)

```bash
azravibe --pkg-init <dir> <name> [version]
```

Initializes a new local project in `<dir>` with the given `<name>`. Optional `version` defaults to `0.1.0`.

This creates:

```
<dir>/
├── azr.project        # Project manifest
├── azr.lock           # Lock file
└── .azr_env/
    └── packages/      # Local package directory
```

On success it prints:

```
پروژه آماده شد: <dir>
```

("Project ready: <dir>")

Example:

```bash
azravibe --pkg-init my_app MyApp 0.2.0
```

---

## Local Package Installation (`--pkg-install`)

```bash
azravibe --pkg-install <dir> <local_path>
```

Installs the local package at `<local_path>` into the project at `<dir>`. The package source can be:

- A directory containing a `azr.project` manifest
- A directory containing an `__init__.azr`
- A single `.azr` file

The package is copied into the project's `package_dir` (default: `.azr_env/packages/`), the `azr.project` manifest is updated with a `dependency` record, and `azr.lock` is updated with the installed record.

On success it prints:

```
بسته محلی نصب شد در <dir>
```

("Local package installed in <dir>")

Example:

```bash
azravibe --pkg-install my_app ../shared_utils
```

---

## Version (`--version` / `-v`)

```bash
azravibe --version
```

Prints the version string. The version is read from `version.txt` next to the executable (or falls back to `v.0.1`).

Example output:

```
قدرت Azravibe v.0.1
```

---

## Help (`--help` / `-h`)

```bash
azravibe --help
```

Prints usage information:

```
azravibe v.0.1
استفاده:
  azravibe <file.azr> [--debug]
  azravibe --test <folder>
  azravibe --pkg-init <dir> <name> [version]
  azravibe --pkg-install <dir> <local_path>
  azravibe --version
  azravibe --help
```

---

## REPL (Interactive Mode)

```bash
azravibe
```

With no arguments, launches the interactive REPL (Read-Eval-Print Loop). The REPL:

- Prints a banner with the version
- Provides a prompt
- Evaluates expressions and statements
- Reports errors with line numbers
- Supports Persian and English keywords

Type `خروج()` or press `Ctrl+C` to exit.

---

## GUI REPL (`--gui-repl`)

```bash
azravibe --gui-repl
```

Internal command used by the Azravibe IDE to spawn a REPL session. The REPL communicates with the IDE via special markers in stdout/stderr:

- `[[AZR_READY]]` -- REPL is ready for input
- `[[AZR_MORE]]` -- REPL expects continuation of a multi-line block

Not intended for direct use from the command line.

---

## Options Summary

| Option | Description |
|---|---|
| *(no arguments)* | Start the interactive REPL |
| `<file.azr> [--debug]` | Run a source file, optionally with debug mode |
| `--test <folder>` | Run all `.azr` files in a folder as tests |
| `--pkg-init <dir> <name> [version]` | Initialize a new project |
| `--pkg-install <dir> <local_path>` | Install a local package into a project |
| `--version` / `-v` | Print version |
| `--help` / `-h` | Print usage |
| `--gui-repl` | Launch GUI-compatible REPL (internal, used by the IDE) |

---

## Exit Codes

| Code | Description |
|---|---|
| `0` | Success (or all tests passed) |
| `1` | Error (syntax, runtime, file not found) or any test failed |

---

## Environment

### stdlib Path

The interpreter locates the `stdlib/` directory relative to the executable path (`argv[0]`). The stdlib must be in `<exe_dir>/stdlib/`.

### Project Import Roots

Projects use an `azr.project` manifest. Example:

```text
name = "demo"
version = "0.1.0"
package_dir = ".azr_env/packages"
path = ".azr_env/packages"
path = "extras"
dependency = "hello|0.1.0|../hello"
```

- `name` and `version` define project metadata
- `package_dir` defines the isolated dependency folder
- `path` adds extra per-project import roots (repeatable)
- `dependency` records locally installed packages as `name|version|source_path`

When running a script inside a project, Azravibe prefers the project package roots before the normal import search path. A matching `azr.lock` file records installed packages.

---

## Examples

### Basic Execution

```bash
azravibe hello.azr
```

### Running Tests

```bash
azravibe --test tests
```

### Project Workflow

```bash
# Create a project
azravibe --pkg-init my_app MyApp 0.1.0

# Create a source file
echo 'بنویس("سلام از پروژه!")' > my_app/main.azr

# Run it
azravibe my_app/main.azr
```

### Installing a Package

```bash
azravibe --pkg-install my_app ../shared_utils
```

---

## Common Errors

| Error | Cause | Solution |
|---|---|---|
| File not found | File does not exist | Check the path and filename |
| `خطای نوشتاری در خط N: ...` | Syntax error | Fix the syntax on the indicated line |
| `خطا: متغیر 'X' تعریف نشده` | Undefined variable | Define the variable before use |
| `خطای نوع: ...` | Wrong type used | Check the types of your values |
| `خطا: تقسیم بر صفر ممکن نیست` | Division by zero | Ensure the divisor is not zero |

For more troubleshooting help, see [Troubleshooting](TROUBLESHOOTING.md).
