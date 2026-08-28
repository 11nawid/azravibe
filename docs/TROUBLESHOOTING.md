# Troubleshooting

Common errors and solutions for Azravibe.

---

## Installation Problems

### "azravibe is not recognized as a command"

**Cause:** Azravibe is not in your system PATH.

**Solution (Windows):**
- Re-run the installer and select "Add to PATH"
- Or manually add the install directory to your PATH environment variable
- Open a new terminal after changing PATH

**Solution (Linux/macOS):**
```bash
sudo make install
# Or add the directory to PATH manually:
export PATH="$PATH:/path/to/azravibe_lang_v1"
```

### "version.txt not found"

**Cause:** The `versions.txt` file is missing from the Azravibe directory.

**Solution:** Ensure `versions.txt` exists next to `azravibe.exe`. The file should contain a single line with the version string (e.g., `v.0.1`).

### Build Fails with "gcc not found"

**Cause:** C compiler is not installed or not in PATH.

**Solution:**
- **Windows:** Install MinGW-w64 or use MSYS2
- **Linux:** `sudo apt install gcc` or equivalent
- **macOS:** `xcode-select --install`

---

## Runtime Errors

### `خطای نوشتاری در خط N: ...` (Syntax Error)

**Cause:** The parser encountered invalid syntax on the specified line.

**Common causes:**
- Missing colon after `اگه`, `کار`, `تا وقتی`, `برای هر`, `نوع`, etc.
- Mismatched parentheses or brackets
- Incorrect indentation
- Missing comma between function arguments

**Example:**
```
کار جمع(a b)    # Missing comma
```

**Fix:**
```
کار جمع(a, b)   # Added comma
```

### `خطا: متغیر 'X' تعریف نشده` (Undefined Variable)

**Cause:** You are using a variable that has not been assigned a value.

**Solution:** Assign a value to the variable before using it:

```azr
x = 10        # Define it first
بنویس(x)     # Then use it
```

### `خطای نوع: ...` (Type Error)

**Cause:** An operation was applied to a value of the wrong type.

**Common cases:**
- Adding a number and a string: `"hello" + 5`
- Calling a non-callable value: `5()`
- Indexing a non-indexable value: `5[0]`

**Solution:** Convert types or check your logic:

```azr
بنویس("hello" + متن(5))    # Convert 5 to string first
```

### `خطا: تقسیم بر صفر ممکن نیست` (Division by Zero)

**Cause:** Division or modulo with zero divisor.

**Solution:** Check your divisor before dividing:

```azr
اگه b != 0:
    نتیجه = a / b
وگرنه:
    بنویس("cannot divide by zero")
```

### `خطا: اندیس N از محدوده لیست خارجه (طول: M)` (Index Out of Range)

**Cause:** You are accessing a list/string/tuple at an index that does not exist.

**Solution:** Check that the index is within bounds. Remember indices start at 0 and end at length-1.

### `خطا: 'X' قابل فراخوانی نیست` (Not Callable)

**Cause:** You tried to call a value that is not a function.

**Example:**
```azr
x = 5
x()     # Error: 5 is not callable
```

### `خطا: کار 'X' به N آرگومان نیاز داره، M داده شد` (Wrong Argument Count)

**Cause:** A function was called with the wrong number of arguments.

**Solution:** Check the function's expected signature and provide the correct number of arguments.

### `خطای فایل: ...` (File Error)

**Cause:** A file operation failed (file not found, permission denied, etc.).

**Solution:**
- Verify the file path is correct
- Check that the file exists
- Ensure you have read/write permissions

---

## Import Errors

### `خطای واردکردن: ماژول 'X' پیدا نشد` (Import Error)

**Cause:** The specified module cannot be found in the import search path.

**Solutions:**
- Check that the `.azr` file exists in the current directory
- If using stdlib, verify the `stdlib/` directory is next to the executable
- If using project packages, verify `azr.project` has the correct `path` entries
- Check for typos in the module name

### Module imports but variables are undefined

**Cause:** The module file may have a syntax error.

**Solution:** Try running the module file directly:
```bash
azravibe module.azr
```

---

## IDE Problems

### IDE Does Not Start

**Solutions:**
- Ensure Node.js 18+ is installed (for building from source)
- Run `npm install` in the `azravibe_ide` directory
- Check the console output for error messages
- Try `npm run dev` for development mode with more detailed output

### F5 (Run) Does Not Work

**Cause:** The Azravibe compiler is not found by the IDE.

**Solution:**
1. Ensure `azravibe` is in your system PATH
2. Or set the compiler path in IDE Settings under "Compiler Path"
3. The default compiler path is `azravibe` (expects it on PATH)

### Persian Text Does Not Render

**Cause:** The Vazir Code or Vazirmatn font is not installed.

**Solution:**
- The Windows installer includes font installation
- For manual installs, copy the fonts from `installer/fonts/` to your system fonts directory
- The IDE bundles Vazirmatn for the UI and Vazir-Code for the terminal

### RTL Layout Looks Wrong

**Solution:**
- Check that the editor direction is set to RTL in Settings
- Some mixed-direction content may display unexpectedly -- this is a known limitation
- Try switching to LTR and back to reset the layout

### Terminal Shows Errors on Launch

**Cause:** The terminal shell is not available.

**Solution:**
- On Windows: Ensure Git Bash or PowerShell is available
- On Linux/macOS: Ensure bash is available
- The terminal tries Git Bash first on Windows, then falls back to PowerShell

---

## Performance Issues

### Slow File Tree

Large directories with many files may be slow to scan. The IDE limits directory scanning depth to 20 levels and ignores common directories like `node_modules`, `.git`, `dist`, `out`, and `build`.

### Slow Syntax Highlighting

Very large files (1000+ lines) may cause syntax highlighting to lag. This is a known limitation of the tree-walk approach.

---

## Platform-Specific Issues

### Windows

- **Chinese/Japanese characters in path:** Some Windows builds may have issues with non-ASCII paths. Use Latin-character paths if possible.
- **Long paths:** Windows has a 260-character path limit by default. Enable long paths in Windows settings if needed.

### Linux

- **Permission denied:** Ensure the executable has execute permission: `chmod +x azravibe`
- **Missing shared libraries:** If you see linker errors, ensure `libm` and `libdl` are available (usually pre-installed)

### macOS

- **Gatekeeper warnings:** macOS may block unsigned executables. Right-click and select "Open" to bypass, or sign the binary.
- **Homebrew C paths:** If using Homebrew GCC, ensure the correct version is in PATH

---

## Getting Help

If your problem is not listed here:

1. Check the [GitHub Issues](https://github.com/11nawid/azravibe/issues) for similar reports
2. Open a new issue with:
   - Your operating system and version
   - Azravibe version (`azravibe --version`)
   - The exact error message
   - A minimal code example that reproduces the issue
