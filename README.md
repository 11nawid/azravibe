# Azravibe

**A Persian/Farsi-oriented programming language and development environment.**

[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Version](https://img.shields.io/badge/version-v.0.1-orange.svg)](CHANGELOG.md)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)](#installation)
[![Language](https://img.shields.io/badge/built_with-C-blue.svg)](azravibe_lang_v1/)
[![Story](https://img.shields.io/badge/read%20my%20story-%F0%9F%93%96-8A2BE2.svg)](STORY.md)

[![Support on Instagram](https://img.shields.io/badge/Support%20the%20Dev-%40n1n.nawid-E4405F?logo=instagram&logoColor=white)](https://www.instagram.com/1n1.nawid)

---

Azravibe (آذرویب) is an interpreted programming language where you write code using natural Persian/Farsi keywords. It follows a familiar Python-like syntax but replaces English keywords with their Farsi equivalents, making programming accessible to Persian speakers in their native language.

The project includes a language interpreter written in C and a desktop IDE built with Electron, React, and CodeMirror, featuring full RTL (right-to-left) support, syntax highlighting, and an integrated terminal.

**Azravibe is early-stage software (v.0.1).** It works and can be used to build CLI programs, but it is not yet a complete general-purpose ecosystem. See [Project Status](docs/STATUS.md) for details.

---

## Quick Example

```azr
# Define a function
کار فیبو(n):
    اگه n <= 1:
        بده n
    وگرنه:
        بده فیبو(n - 1) + فیبو(n - 2)

# Call it in a loop
برای هر i توی محدوده(10):
    بنویس(فیبو(i))
```

Save this as `fibo.azr` and run:

```bash
azravibe fibo.azr
```

Output:

```
0
1
1
2
3
5
8
13
21
34
```

---

## What Makes Azravibe Different

- **Persian-first syntax.** All keywords are Farsi: `اگه` (if), `کار` (def), `تا وقتی` (while), `بنویس` (print), and more.
- **RTL-native IDE.** The Azravibe IDE renders code and UI right-to-left with Persian fonts.
- **Familiar structure.** Indentation-based blocks, dynamic typing, functions with defaults and varargs, classes, modules, and exceptions -- all concepts that transfer from Python.
- **Dual-language keywords.** Every Persian keyword has an English alias (`اگه`/`if`, `کار`/`def`), so bilingual developers can use either.
- **Standard library.** Modules for math, JSON, CSV, HTTP, datetime, random, filesystem operations, and more.
- **Native C extensions.** Performance-critical or OS-bound modules can be written in C and loaded at runtime.

---

## Current Capabilities

| Feature | Status |
|---|---|
| Variables, types, operators | Working |
| Functions (defaults, varargs, closures, lambdas) | Working |
| Control flow (if/elif/else, while, for) | Working |
| Data structures (lists, tuples, dicts, sets, bytes) | Working |
| Classes, inheritance, `super()` | Working |
| Try/except/finally, exceptions | Working |
| String interpolation (`"سلام {نام}"`) | Working |
| Modules and imports (`بیار`, `از...بیار`) | Working |
| Project manifests and local packages | Working |
| Standard library (23 modules) | Working |
| REPL (command line) | Working |
| IDE with RTL support | Working |
| Native C extension API | Working |

See [Project Status](docs/STATUS.md) for what is planned but not yet implemented.

---

## Installation

### Windows (Installer)

Download and run the Windows installer from the [Releases](https://github.com/11nawid/azravibe/releases) page. The installer:

- Installs `azravibe.exe` and `azr.exe` (compatibility alias)
- Installs the standard library
- Installs the Vazir Code font for Persian rendering
- Optionally adds Azravibe to your `PATH`
- Creates Start Menu shortcuts

### Building from Source

**Prerequisites:** A C11 compiler (GCC, Clang, or MinGW on Windows)

```bash
cd azravibe_lang_v1
make
sudo make install    # Linux/macOS
```

On Windows with MinGW:

```powershell
cd azravibe_lang_v1
mingw32-make
```

### IDE

The IDE is a separate Electron application. See [IDE Documentation](docs/IDE.md) for build and installation instructions.

---

## Getting Started

1. **Install** Azravibe (see above)
2. **Create a file** called `hello.azr`:

```azr
بنویس("سلام دنیا!")
```

3. **Run it:**

```bash
azravibe hello.azr
```

That's it. See [Getting Started](docs/GETTING-STARTED.md) for a complete walkthrough, or [Tutorial](docs/TUTORIAL.md) to learn the language.

---

## Documentation

| Document | Description |
|---|---|
| [Getting Started](docs/GETTING-STARTED.md) | Installation, first project, running code |
| [Tutorial](docs/TUTORIAL.md) | Learn Azravibe step by step |
| [Language Reference](docs/REFERENCE.md) | Complete technical reference |
| [Keyword Reference](docs/KEYWORDS.md) | Farsi-to-English keyword mapping |
| [Examples](docs/EXAMPLES.md) | Working code examples |
| [IDE Guide](docs/IDE.md) | Azravibe IDE documentation |
| [CLI Reference](docs/CLI.md) | Command-line interface reference |
| [Standard Library](docs/REFERENCE.md#standard-library) | Standard library modules |
| [Troubleshooting](docs/TROUBLESHOOTING.md) | Common errors and solutions |
| [Project Status](docs/STATUS.md) | What works, what is experimental, what is planned |
| [Roadmap](ROADMAP.md) | Future development plans |
| [Testing](docs/TESTING.md) | How to run and write tests |

---

## Project Structure

```
azravibe/
├── azravibe_lang_v1/     # Language interpreter (C)
│   ├── src/              # C source code
│   ├── stdlib/           # Standard library modules (.azr + native C)
│   ├── tests/            # Language tests
│   ├── installer/        # Windows installer scripts
│   └── Makefile          # Build system
├── azravibe_ide/         # IDE (Electron + React + TypeScript)
│   └── src/              # TypeScript/React source code
├── docs/                 # Documentation
├── CONTRIBUTING.md       # Contribution guide
├── CHANGELOG.md          # Version history
├── ROADMAP.md            # Development roadmap
├── CODE_OF_CONDUCT.md    # Community guidelines
├── SECURITY.md           # Security policy
└── LICENSE               # MIT License
```

---

## Contributing

Contributions are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md) for how to:

- Report bugs
- Suggest features
- Modify the compiler or IDE
- Add tests
- Improve documentation

---

## Community

- **Issues:** [GitHub Issues](https://github.com/11nawid/azravibe/issues)
- **Discussions:** [GitHub Discussions](https://github.com/11nawid/azravibe/discussions)
- **The Story:** [Why I Built Azravibe](STORY.md)

---

## Support the Developer

Azravibe is a personal learning project, built in my spare time and shared for free. If you like it, found it useful, or want to follow along with what I build next, consider supporting me:

[![Support on Instagram](https://img.shields.io/badge/Support%20the%20Dev-%40n1n.nawid-E4405F?logo=instagram&logoColor=white)](https://www.instagram.com/1n1.nawid)

Every follow, share, and message genuinely means a lot.

---

## License

Azravibe is licensed under the [MIT License](LICENSE).

Copyright (c) 2026 Azravibe
