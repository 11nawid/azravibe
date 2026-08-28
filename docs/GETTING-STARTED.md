# Getting Started with Azravibe

A complete beginner-friendly guide to installing Azravibe, writing your first program, and understanding how everything works.

---

## Prerequisites

- **Operating System:** Windows 10+, macOS, or Linux
- **For building from source:** A C11 compiler (GCC, Clang, or MinGW)
- **For the IDE:** Node.js 18+ and npm (for building the IDE from source)

---

## Installation

### Option 1: Windows Installer (Recommended)

1. Download the installer from the [Releases](https://github.com/azravibe/azravibe/releases) page
2. Run `Azravibe-Setup-{version}.exe`
3. Follow the installation wizard:
   - Choose the installation directory (default: `%LOCALAPPDATA%\Programs\Azravibe`)
   - Select "Add to PATH" to use `azravibe` from any terminal
   - Optionally create a desktop shortcut
4. The installer also installs the **Vazir Code** font for Persian text rendering

After installation, open a new terminal and verify:

```bash
azravibe --version
```

You should see:

```
قدرت Azravibe v.0.1
```

### Option 2: Build from Source

#### Linux / macOS

```bash
# Clone the repository
git clone https://github.com/azravibe/azravibe.git
cd azravibe/azravibe_lang_v1

# Build
make

# Install (optional, puts azravibe in /usr/local/bin)
sudo make install

# Verify
./azravibe --version
```

#### Windows (MinGW)

```powershell
# Clone the repository
git clone https://github.com/azravibe/azravibe.git
cd azravibe\azravibe_lang_v1

# Build
mingw32-make

# Verify
.\azravibe.exe --version
```

#### Windows (CMake)

```powershell
cd azravibe\azravibe_lang_v1
mkdir build
cd build
cmake ..
cmake --build .
```

### Option 3: PowerShell Installer (Windows)

```powershell
cd azravibe\azravibe_lang_v1
powershell -ExecutionPolicy Bypass -File installer\install.ps1
```

This copies executables, installs the stdlib, adds Azravibe to your PATH, and installs the Vazir Code font.

---

## Your First Program

### Step 1: Create a file

Create a new file called `hello.azr` with this content:

```azr
بنویس("سلام دنیا!")
```

The `.azr` extension is the standard file extension for Azravibe source files.

### Step 2: Run the program

Open a terminal and run:

```bash
azravibe hello.azr
```

Output:

```
سلام دنیا!
```

Congratulations -- you just ran your first Azravibe program.

### Step 3: Try another example

Create `welcome.azr`:

```azr
نام = بگیر("نام شما: ")
بنویس("سلام {نام}! خوش آمدید به آذرویب.")
```

Run it:

```bash
azravibe welcome.azr
```

The program prompts for your name, then greets you using string interpolation.

---

## Using the REPL

The REPL (Read-Eval-Print Loop) lets you run Azravibe code interactively.

Start it:

```bash
azravibe
```

You will see the Azravibe banner and a prompt. Type code and press Enter:

```
>>> بنویس(2 + 3)
5
>>> نام = "آذرویب"
>>> بنویس("زبان {نام}")
زبان آذرویب
>>> محدوده(5)
[0, 1, 2, 3, 4]
```

Press `Ctrl+C` or type `خروج()` to leave the REPL.

> **Tip:** To evaluate a snippet without creating a file, use the interactive REPL:
>
> ```bash
> azravibe
>>> بنویس(2 * 21)
> 42
> ```

---

## Creating a Project

Azravibe supports a simple project manifest system for managing local packages.

```bash
# Initialize a new project
azravibe --pkg-init my_app MyApp 0.1.0
```

This creates:

```
my_app/
├── azr.project        # Project manifest
├── azr.lock           # Lock file
└── .azr_env/
    └── packages/      # Local package directory
```

The `azr.project` manifest format:

```text
name = "my_app"
version = "0.1.0"
package_dir = ".azr_env/packages"
path = ".azr_env/packages"
```

### Installing Local Packages

```bash
azravibe --pkg-install my_app ../shared_package
```

This copies the package into the project's package directory and updates the manifest and lock file.

### Running Project Files

```bash
azravibe main.azr
```

When running inside a project directory, Azravibe automatically searches the project's package paths for imports.

---

## Understanding the Output

When you run a program, Azravibe:

1. Reads the `.azr` source file
2. Tokenizes it (lexer)
3. Parses tokens into an AST (abstract syntax tree)
4. Interprets the AST directly (tree-walk interpreter)
5. Produces output to the console

If there is an error, Azravibe prints a message in Farsi with a line number:

```
خطای نوشتاری در خط 3: متغیر 'x' تعریف نشده
```

This means "Syntax error on line 3: variable 'x' is not defined."

---

## File Extension Convention

All Azravibe source files use the `.azr` extension. The IDE associates this extension automatically.

---

## Next Steps

- [Tutorial](TUTORIAL.md) -- Learn the Azravibe language step by step
- [Examples](EXAMPLES.md) -- Working code examples
- [Language Reference](REFERENCE.md) -- Complete technical reference
- [IDE Guide](IDE.md) -- Using the Azravibe IDE
- [CLI Reference](CLI.md) -- All command-line options
