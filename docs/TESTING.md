# Testing

Overview of testing in the Azravibe project.

---

## Current Test State

Azravibe has a minimal test suite. The tests exist in `azravibe_lang_v1/tests/` and cover basic language features. There are currently 3 test files, each very short.

**Honest assessment:** The test coverage is low. Many language features (classes, inheritance, try/except, closures, imports, modules, stdlib usage) are not tested. Expanding the test suite is a priority for the project.

---

## Existing Tests

### test_1.azr

Tests basic list creation, variable assignment, list indexing, and Persian numerals.

### test_foundation_core.azr

Tests function definition, return values, empty lists, for-each loops with range, list append, dictionary literals, boolean values, string keys, dot notation access, and nested function calls.

### test_builtin_aliases_2.azr

Tests that both Persian and English aliases work for `print`, `len`, and `range`.

---

## Running Tests

### Using the --test flag

From the language directory:

```bash
cd azravibe_lang_v1
./azravibe --test tests
```

This runs all `.azr` files in the `tests/` directory.

### Running Individual Tests

```bash
./azravibe tests/test_1.azr
./azravibe tests/test_foundation_core.azr
./azravibe tests/test_builtin_aliases_2.azr
```

### Using Make

```bash
cd azravibe_lang_v1
make test
```

---

## Writing Tests

Tests are regular `.azr` files placed in the `tests/` directory. They can use `بنویس` (print) to output results.

A good test should:

1. Exercise a specific feature
2. Print output that can be verified
3. Have a descriptive filename

Example test file:

```azr
# test_loop_break.azr
# Tests that break exits a loop correctly

نتیجه = []
برای هر i توی محدوده(10):
    اگه i == 5:
        بزن بیرون
    اضافه(نتیجه، i)

بنویس(نتیجه)
# Expected output: [0, 1, 2, 3, 4]
```

---

## Features Needing Test Coverage

The following areas need additional tests:

| Feature | Current Coverage |
|---|---|
| Variables and assignment | Partial |
| Data types | Partial |
| Operators | Minimal |
| Control flow (if/elif/else) | Not tested |
| Loops (while, for) | Partial |
| Functions | Partial |
| Closures | Not tested |
| Lambda | Not tested |
| Classes | Not tested |
| Inheritance | Not tested |
| Try/except/finally | Not tested |
| Modules/imports | Not tested |
| String interpolation | Not tested |
| List comprehensions | Not tested |
| Ternary expressions | Not tested |
| Standard library | Not tested |
| Error messages | Not tested |
| Edge cases | Not tested |

---

## Testing Recommendations

To improve test coverage, the project needs:

1. **Unit tests per feature:** One test file per language feature
2. **Regression tests:** Tests for reported bugs
3. **Error case tests:** Tests that verify error messages for invalid code
4. **Stdlib tests:** Tests for each standard library module
5. **Integration tests:** Tests that exercise multiple features together
6. **A test runner:** A script that runs all tests and reports pass/fail

Until a formal test framework is available, tests can be structured as print-and-verify programs.
