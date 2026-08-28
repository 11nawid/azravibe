# Azravibe Foundation Status

This file tracks the minimum foundation needed for a public, Python-style v1 language release.

## Complete in v1 foundation

- Interpreter executable: `azravibe.exe`
- `.azr` file execution: `azravibe file.azr`
- REPL: `azravibe`
- Variables and dynamic values
- Functions, default arguments, varargs, closures, and lambdas
- `if` / `else` / `elif`
- `while` and `for each` loops
- Lists, tuples, dictionaries, sets, strings, numbers, booleans, none, and bytes
- Modules and imports for `.azr` files
- Package-style `__init__.azr` modules
- Runtime exceptions and `try` / `except` / `finally`
- Classes, instances, attributes, inheritance, and `super`
- Built-in functions
- Standard library modules in `stdlib/`
- Project manifests with `azr.project`
- Local package installation with `--pkg-install`
- Native C extension loading through `azr_module_init`

## Still needed before a serious public release

- Larger regression test suite for every feature above
- More complete examples in `examples/`
- Stronger traceback formatting with source context
- Public package manager commands such as `azr pkg add`, `azr pkg remove`, and `azr pkg publish`
- IDE integration for package install, run, test, and native-extension docs

## Release rule

Azravibe does not need to equal modern Python before release. It needs a stable foundation where users can run `.azr` files, import modules, write packages, use the standard library, extend with C when needed, and learn from clear examples.
