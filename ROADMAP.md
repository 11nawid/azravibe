# Roadmap

This roadmap describes planned future development for Azravibe. Items are organized by priority and rough timeline.

**Note:** This is a single-developer project. Timelines are approximate and depend on available time.

---

## Near-Term (v0.2.x)

Improvements to the existing foundation.

### Language

- [ ] Expanded test suite covering all language features
- [ ] Improved error messages with source code context (show the offending line)
- [ ] Step parameter for slicing (`[start:end:step]`)
- [ ] Fix circular reference detection in garbage collection
- [ ] Improve string interpolation edge cases

### Standard Library

- [ ] Fill in `فایل` (file) module properly
- [ ] Add missing stdlib functions based on community needs
- [ ] Improve consistency of function naming conventions

### CLI

- [ ] `--help` output improvements
- [ ] Better REPL experience (history, tab completion)
- [ ] `--test` flag improvements (pass/fail reporting, exit codes)

### Testing

- [ ] Add tests for all language features
- [ ] Add tests for all standard library modules
- [ ] Add regression tests for reported bugs
- [ ] Automated test runner with pass/fail reporting

---

## Medium-Term (v0.3.x)

Significant improvements to the developer experience.

### Language

- [ ] Decorators
- [ ] Multiple inheritance (MRO)
- [ ] Walrus operator (`:=`)
- [ ] Match/case (pattern matching)
- [ ] Improved generator protocols

### IDE

- [ ] Language Server Protocol (LSP) for autocompletion and diagnostics
- [ ] Git integration (status, diff, commit, branch)
- [ ] Inline diagnostics as you type
- [ ] Code formatting
- [ ] Refactoring tools (rename, extract)
- [ ] Extension/plugin system
- [ ] Debugger (breakpoints, step through, variable inspection)
- [ ] Better outline panel with navigation
- [ ] Remote workspace support

### Tooling

- [ ] Package manager (`azr pkg add`, `azr pkg remove`, `azr pkg publish`)
- [ ] Formatter / linter
- [ ] Documentation generator
- [ ] Project templates / scaffolding

---

## Long-Term (v1.0)

Major milestones for a production-quality language.

### Language

- [ ] Bytecode compiler and VM (replacing tree-walk interpreter)
- [ ] Just-in-time compilation (JIT) for performance-critical code
- [ ] True async/await support
- [ ] Proper threading with GIL or actor model
- [ ] Advanced type system or type annotations
- [ ] Optimizer passes

### Standard Library

- [ ] Native socket module (no external dependencies)
- [ ] Native SQLite bindings (no CLI dependency)
- [ ] Native HTTP server framework
- [ ] Web framework for web applications
- [ ] GUI application framework
- [ ] Database connectivity beyond SQLite
- [ ] Cryptography module (native, not shell-based)
- [ ] Regular expression module
- [ ] Unit testing framework
- [ ] Logging with multiple handlers and formatters

### Platform

- [ ] Improved macOS support and testing
- [ ] Improved Linux support and testing
- [ ] ARM architecture support
- [ ] WebAssembly compilation target
- [ ] Cross-platform installer improvements

### Community

- [ ] Community package registry
- [ ] Contribution documentation improvements
- [ ] Example projects and tutorials
- [ ] Persian-language documentation
- [ ] Community Discord or forum

---

## Contributing to the Roadmap

The roadmap is a living document. If you would like to suggest changes or priorities:

1. Open a [GitHub Discussion](https://github.com/11nawid/azravibe/discussions) with your proposal
2. Explain why the feature is important to you
3. Describe any use cases that depend on it

Roadmap items are not commitments. They represent the direction of the project and may change based on community feedback and available resources.
