# آذرویب — زبان برنامه‌نویسی فارسی

![Language](https://img.shields.io/badge/language-C-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)
![Version](https://img.shields.io/badge/version-v.0.1-orange.svg)

آذرویب یک زبان برنامه‌نویسی تفسیری فارسی‌محور است که با C نوشته شده و برای نوشتن کد با کلمه‌های طبیعی و روزمره فارسی ساخته شده است.

Azravibe is a Farsi-native interpreted programming language written in C. It feels familiar like Python, but its keywords are natural everyday Persian.

## نمونه سریع / Quick Example

Python:

```python
def fib(n):
    if n <= 1:
        return n
    return fib(n - 1) + fib(n - 2)

for i in range(10):
    print(fib(i))
```

azravibe_lang_v1:

```azr
کار فیبو(n):
    اگه n <= 1:
        بده n
    وگرنه:
        بده فیبو(n - 1) + فیبو(n - 2)

برای هر i توی محدوده(10):
    بنویس(فیبو(i))
```

## نصب / Installation

```bash
make
sudo make install
```

Windows with MinGW:

```powershell
mingw32-make
.\azravibe.exe examples\سلام_دنیا.azr
```

Windows one-file installer:

```powershell
powershell -ExecutionPolicy Bypass -File .\installer\build-installer.ps1
```

Reset the local Windows install before retesting:

```powershell
powershell -ExecutionPolicy Bypass -File .\installer\reset-install.ps1
```

## استفاده / Usage

Run a file:

```bash
./azravibe examples/سلام_دنیا.azr
./azravibe examples/فیبوناچی.azr --debug
```

Start the REPL:

```bash
./azravibe
```

Project and local package commands:

```bash
./azravibe --pkg-init my_app MyApp 0.1.0
./azravibe --pkg-install my_app ../shared_pkg
./azravibe my_app/main.azr
```

The Windows installer now uses a real Inno Setup wizard. It installs
`azravibe.exe`, keeps a compatibility alias `azr.exe`, includes a proper
uninstaller entry, creates Start Menu shortcuts, optionally creates a desktop
shortcut, adds the install folder to the user `PATH` when selected, and
upgrades the same installation directory in place on newer releases. The same
installer also ships the dedicated Azravibe console UI and installs the
required `Vazir Code` font so Persian text renders cleanly there by default.

Native C extension modules can be imported when a `.dll`, `.so`, or `.dylib`
with `azr_module_init` is present on the import path. See
`docs/NATIVE_EXTENSIONS.md`.

## جدول کلیدواژه‌ها / Keyword Reference

| فارسی | English |
|---|---|
| اگه | if |
| وگرنه | else |
| وگرنه اگه | elif |
| تا وقتی | while |
| برای هر | for |
| توی | in |
| کار | def |
| بده / برگشت | return |
| بنویس | print |
| بگیر | input |
| بیار | import |
| درسته | true |
| غلطه | false |
| هیچ | none |
| و | and |
| یا | or |
| نه | not |
| بزن بیرون | break |
| ادامه | continue |
| هیچی | pass |
| نوع | class placeholder |
| سعی کن | try |
| اگه خطا | except |
| بالاخره | finally |
| با | with |
| به عنوان | as |
| طول | len |
| از / تا | range words |

Both `,` and `،` work as separators.

## توابع ساختنی / Built-ins

| دسته | توابع |
|---|---|
| ورودی/خروجی | بنویس، بگیر |
| تبدیل نوع | عدد_صحیح، عدد_اعشاری، متن، بولی |
| بررسی نوع | نوعش |
| لیست | طول، اضافه، حذف، مرتب، برعکس، پیدا |
| متن | برش، بزرگ، کوچک، تکرار_داره، جدا_کن، بچسبان، فاصله_نگیر |
| ریاضی | مطلق، گرد، بالا، پایین، بیشترین، کمترین |
| تکرار | محدوده |
| ابزار | خروج، تصادفی، زمان |

## معماری / Architecture

```text
[.azr source] -> [Lexer] -> [Tokens]
[Tokens]      -> [Parser] -> [AST]
[AST]         -> [Interpreter] -> [Output]
```

## پروژه و بسته محلی / Project Manifest And Local Packages

Azravibe now supports a simple project manifest named `azr.project` plus an isolated local package directory at `.azr_env/packages`.

Example manifest:

```text
name = "demo"
version = "0.1.0"
package_dir = ".azr_env/packages"
path = ".azr_env/packages"
path = "extras"
dependency = "hello|0.1.0|../hello"
```

- `name` and `version` define project metadata.
- `package_dir` defines the isolated dependency folder.
- `path` adds extra per-project import roots.
- `dependency` records locally installed packages as `name|version|source_path`.

The CLI writes a matching `azr.lock` file with installed package records. When you run a script inside a project, Azravibe automatically prefers the project package roots before the normal import search path.

## افزونه‌های Native C / Native C Extensions

Azravibe exposes a small native ABI for performance-heavy modules and OS bindings.
Native modules export:

```c
AZR_NATIVE_EXPORT int azr_module_init(Environment *module_env, const AzrNativeAPI *api);
```

The runtime calls this function during `بیار module_name`, and the module can
register built-in functions into its module environment. See:

- `src/azravibe_native.h`
- `examples/native_math.c`
- `examples/native_demo.azr`
- `docs/NATIVE_EXTENSIONS.md`

## مثال‌ها / Examples

- `examples/سلام_دنیا.azr`
- `examples/ماشین_حساب.azr`
- `examples/فیبوناچی.azr`
- `examples/مرتب_سازی.azr`
- `examples/بازی_عدد.azr`

## Roadmap

- Better diagnostic source-line pointers
- Package manager for `.azr` modules
- Bytecode VM backend after the tree-walk interpreter stabilizes
- Richer class/object runtime

## Contributing

Pull requests are welcome. Please keep the language Farsi-first, keep all C code warning-clean under `-Wall -Wextra -std=c11`, and add `.azr` tests for every language change.

## License

MIT License

Copyright (c) 2026 Azravibe

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files, to deal in the Software
without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
