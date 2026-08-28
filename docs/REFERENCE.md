# Azravibe Language Reference

A complete technical reference for the Azravibe programming language.

---

## Table of Contents

1. [Lexical Structure](#1-lexical-structure)
2. [Data Types](#2-data-types)
3. [Variables](#3-variables)
4. [Operators](#4-operators)
5. [Statements](#5-statements)
6. [Expressions](#6-expressions)
7. [Functions](#7-functions)
8. [Classes](#8-classes)
9. [Error Handling](#9-error-handling)
10. [Modules and Imports](#10-modules-and-imports)
11. [Built-in Functions](#11-built-in-functions)
12. [Standard Library](#12-standard-library)

---

## 1. Lexical Structure

### Source Files

Azravibe source files use the `.azr` extension. Files are UTF-8 encoded.

### Comments

```azr
# This is a single-line comment

"""
This is a multi-line comment
(multiline triple-quoted strings are treated as comments)
"""
```

### Indentation

Azravibe uses indentation (4 spaces recommended) to define code blocks, similar to Python. The lexer emits `INDENT` and `DEDENT` tokens based on whitespace changes at line starts.

### Keywords

See [KEYWORDS.md](KEYWORDS.md) for the complete keyword table. Every Persian keyword also has an English alias.

### Identifiers

Identifiers can contain:
- Latin letters (a-z, A-Z)
- Persian/Arabic characters (Unicode range U+0600-U+06FF)
- Digits (0-9)
- Underscore (`_`)
- Zero-width non-joiner (`\u200C`) for Persian compound words

Identifiers cannot start with a digit.

### Literals

| Literal | Example | Type |
|---|---|---|
| Integer | `42`, `-10`, `9999999` | `عدد صحیح` |
| Float | `3.14`, `-0.5`, `1e10` | `عدد اعشاری` |
| String | `"hello"`, `"سلام"` | `رشته` |
| Boolean | `درسته` / `غلطه` | `بولی` |
| None | `هیچ` | `هیچ` |

### Commas

Both Latin comma (`,`) and Arabic/Persian comma (`،`) are accepted as separators throughout the language. They are interchangeable.

---

## 2. Data Types

Azravibe has 24 runtime value types:

### Primitive Types

| Type | Persian Name | Description | Example |
|---|---|---|---|
| Integer | `عدد صحیح` | 64-bit signed integer | `42` |
| Float | `عدد اعشاری` | 64-bit double-precision float | `3.14` |
| String | `رشته` | UTF-8 text | `"سلام"` |
| Boolean | `بولی` | True or false | `درسته` |
| None | `هیچ` | Null/void value | `هیچ` |
| Bytes | `بایت` | Raw byte sequence | `بایت("abc")` |

### Collection Types

| Type | Persian Name | Description | Example |
|---|---|---|---|
| List | `لیست` | Mutable ordered sequence | `[1، 2، 3]` |
| Tuple | `چندتایی` | Immutable ordered sequence | `(1، 2)` |
| Dictionary | `دیکشنری` | Key-value mapping | `{"a": 1}` |
| Set | `مجموعه` | Unordered unique values | `{1، 2، 3}` |

### Callable Types

| Type | Description | Example |
|---|---|---|
| Function | User-defined or built-in function | `کار foo(): ...` |
| Lambda | Anonymous function | `لامبدا(x): x * 2` |
| Class | Class object | `نوع Foo:` |
| Instance | Class instance | `Foo()` |
| Bound Method | Method bound to instance | `obj.method` |

### Module Types

| Type | Description |
|---|---|
| Module | Imported module object |
| Exception | Exception/error object |
| Iterator | Iterator over a collection |
| Generator | Generator with yield |
| Super | Reference to parent class |

### Type Checking

Use `نوعش(value)` to get the type name as a string:

```azr
بنویس(نوعش(42))        # "عدد صحیح"
بنویس(نوعش("hello"))   # "رشته"
بنویس(نوعش([1، 2]))    # "لیست"
```

### Type Conversion

```azr
عدد_صحیح(x)    # Convert to integer (truncates floats, parses strings)
عدد_اعشاری(x)  # Convert to float
متن(x)         # Convert to string
بولی(x)        # Convert to boolean
بایت(x)        # Convert to bytes
```

### Truthiness

The following values are falsy:
- `هیچ` (none)
- `غلطه` (false)
- `0` (integer zero)
- `0.0` (float zero)
- `""` (empty string)
- `[]` (empty list)
- `{}` (empty dict/set)

Everything else is truthy.

---

## 3. Variables

### Assignment

```azr
x = 10
نام = "آذرویب"
```

### Multiple Assignment

```azr
a = b = c = 0
```

### Augmented Assignment

| Operator | Persian | Description |
|---|---|---|
| `=` | مساوی | Assignment |
| `+=` | اضافه مساوی | Add and assign |
| `-=` | کم مساوی | Subtract and assign |
| `*=` | ضرب مساوی | Multiply and assign |
| `/=` | تقسیم مساوی | Divide and assign |

### Scope

Variables follow lexical scoping. Inner blocks can read outer variables, but assignments create new local bindings:

```azr
x = 10
اگه درسته:
    x = 20     # Creates a new local x
بنویس(x)       # Still 10
```

---

## 4. Operators

### Arithmetic Operators

| Operator | Description | Example |
|---|---|---|
| `+` | Addition | `5 + 3` = 8 |
| `-` | Subtraction | `5 - 3` = 2 |
| `*` | Multiplication | `5 * 3` = 15 |
| `/` | Division (always float) | `5 / 2` = 2.5 |
| `//` | Floor division | `5 // 2` = 2 |
| `%` | Modulo | `5 % 2` = 1 |
| `**` | Power | `2 ** 8` = 256 |
| `-` | Unary negation | `-x` |

### Comparison Operators

| Operator | Description |
|---|---|
| `==` | Equal |
| `!=` | Not equal |
| `>` | Greater than |
| `<` | Less than |
| `>=` | Greater than or equal |
| `<=` | Less than or equal |

### Logical Operators

| Persian | English | Description |
|---|---|---|
| `و` | `and` | Logical AND (short-circuit) |
| `یا` | `or` | Logical OR (short-circuit) |
| `نه` | `not` | Logical NOT |

### Membership Operators

| Operator | Description |
|---|---|
| `توی` / `in` | Membership test for strings, lists, dicts, sets |

### String Operators

| Operator | Description |
|---|---|
| `+` | Concatenation |
| `*` | Repetition |

### Ternary Expressions

```azr
نتیجه = "بله" اگه شرط وگرنه "نه"
نتیجه = شرط ? "بله" : "نه"
```

### Operator Precedence

From lowest to highest:

1. `یا` (or)
2. `و` (and)
3. `نه` (not)
4. `==`, `!=`, `>`, `<`, `>=`, `<=`, `توی`
5. `+`, `-`
6. `*`, `/`, `//`, `%`
7. `**`
8. Unary `-`, `نه`
9. Calls, indexing, attribute access

---

## 5. Statements

### If / Elif / Else

```azr
اگه شرط:
    ...
وگرنه اگه شرط_دوم:
    ...
وگرنه:
    ...
```

### While Loop

```azr
تا وقتی شرط:
    ...
```

### For-Each Loop

```azr
برای هر متغیر توی مجموعه:
    ...
```

### Function Definition

```azr
کار نام(پارامترها):
    ...
    بده مقدار
```

### Class Definition

```azr
نوع نام:
    ...
```

### Return

```azr
بده مقدار
برگرد مقدار      # Same as بده
```

### Yield (Generator)

```azr
تحویل مقدار
```

### Break / Continue

```azr
بزن بیرون         # Break
ادامه             # Continue
```

### Pass (No-op)

```azr
هیچی
```

### Try / Except / Finally

```azr
سعی کن:
    ...
اگه خطا به عنوان e:
    ...
بالاخره:
    ...
```

### With / As

```azr
با عبارت به عنوان نام:
    ...
```

### Raise Exception

```azr
بینداز مقدار
```

---

## 6. Expressions

### Literal Expressions

```azr
42          # Integer
3.14        # Float
"hello"     # String
درسته       # True
غلطه       # False
هیچ         # None
```

### List Literal

```azr
[1، 2، 3]
```

### Tuple Literal

```azr
(1، 2)
```

### Dictionary Literal

```azr
{"کلید": "مقدار"، "عدد": 42}
```

### Set Literal

```azr
{1، 2، 3}
```

### Function Call

```azr
نام(آرگومان‌ها)
نام(کلید=مقدار، ...)
```

### Indexing

```azr
لیست[0]
دیکشنری["کلید"]
```

### Slicing

```azr
متن[1:4]
لیست[0:10]
```

### Attribute Access

```azr
شیء.ویژگی
```

### Lambda

```azr
لامبدا(پارامترها): عبارت
```

### Comprehensions

```azr
[عبارت برای هر x توی مجموعه]
{کلید: مقدار برای هر x توی مجموعه}
```

---

## 7. Functions

### Basic Function

```azr
کار جمع(a, b):
    بده a + b
```

### Default Arguments

```azr
کار سلام(نام="دوست"):
    بنویس("سلام {نام}!")
```

### Named Arguments

```azr
کار پروفایل(نام، سن، شهر="نامشخص"):
    ...

پروفایل(سن=25، نام="نوید")
```

### Varargs (*args)

```azr
کار مجموع(*اعداد):
    ...
```

### Automatic Self

Methods in classes automatically receive `خود` (self) as the first parameter. You can declare it explicitly or omit it:

```azr
نوع Foo:
    کار bar():          # خود is implicit
        بنویس(خود.x)

    کار baz(خود):       # خود is explicit (also valid)
        بنویس(خود.x)
```

---

## 8. Classes

### Basic Class

```azr
نوع Person:
    کار سازنده(نام):
        خود.نام = نام

    کار hello():
        بده "سلام {خود.نام}"
```

### Inheritance

```azr
نوع Student(Person):
    کار study():
        بده "{خود.نام} در حال مطالعه"
```

The base class is given in parentheses after the class name.

### Super

```azr
نوع Child(Parent):
    کار سازنده():
        ابر()
```

### Dunder / Magic Methods

| Method | Persian Equivalent | Description |
|---|---|---|
| `__init__` | `سازنده` | Constructor |
| `__str__` | `متن` | String representation |
| `__eq__` | `مثل_هست` | Equality comparison |
| `__call__` | `فراخوان` | Makes instance callable |
| `__getitem__` | `شاخص_بگیر` | Index access |
| `__enter__` | `ورود` | Context manager enter |
| `__exit__` | `خروج` | Context manager exit |

---

## 9. Error Handling

### Try/Except/Finally

```azr
سعی کن:
    x = 10 / 0
اگه خطا به عنوان e:
    بنویس("خطا: {e}")
بالاخره:
    بنویس("تمام")
```

### Raising Exceptions

```azr
بینداز "یک پیام خطا"
```

### Built-in Exception Types

| Type | Persian Name | Description |
|---|---|---|
| Base error | `خطا` | Base exception class |
| Name error | `خطای_نام` | Undefined variable |
| Type error | `خطای_نوع` | Wrong type |
| Division by zero | `خطای_تقسیم_بر_صفر` | Division by zero |
| Index error | `خطای_شاخص` | Index out of range |
| File error | `خطای_فایل` | File operation failed |
| Import error | `خطای_واردکردن` | Module not found |
| Call error | `خطای_فراخوانی` | Invalid function call |
| Assertion error | `خطای_ادعا` | Assertion failed |
| Stop iteration | `توقف_تکرار` | Iterator exhausted |

---

## 10. Modules and Imports

### Import Module

```azr
بیار math
```

### Import with Alias

```azr
بیار math به عنوان ریاضی
```

### From Import

```azr
از math بیار pi، sqrt
```

### From Import with Alias

```azr
از math بیار pi به عنوان پی
```

### Module Search Path

Azravibe searches for modules in:
1. The current directory
2. Project import roots (from `azr.project` manifest)
3. The `stdlib/` directory next to the executable

### Package Modules

Directories with an `__init__.azr` file are treated as packages. Import the package and it runs `__init__.azr` first.

---

## 11. Built-in Functions

### I/O

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `print` | `بنویس` | `*args` | Print values separated by spaces with newline |
| `input` | `بگیر` | `prompt=""` | Read a line of input from the user |

### Type Conversion

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `int` | `عدد_صحیح` | `x` | Convert to integer |
| `float` | `عدد_اعشاری` | `x` | Convert to float |
| `str` | `متن` | `x` | Convert to string |
| `bool` | `بولی` | `x` | Convert to boolean |
| `bytes` | `بایت` | `x` | Convert to bytes |

### Type Inspection

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `type` | `نوعش` | `x` | Returns type name as string |
| `len` | `طول` | `x` | Returns length of string, list, tuple, set, dict, or bytes |

### Iteration Protocol

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `iter` | `تکرار` | `x` | Get an iterator from a collection |
| `next` | `بعدی` | `iterator` | Advance the iterator, return next value |

### Assertions

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `assert` | `ادعا` | `condition` | Assert a condition is true, raise `خطای_ادعا` if not |

### Decorators

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `classmethod` | `کلاس_متد` | `method` | Class method (uses `کلاس`/`cls` instead of instance) |
| `staticmethod` | `ایستا` | `method` | Static method (no self or cls) |
| `property` | `ویژگی` | `getter` | Property accessor |

### Regular Expressions

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `regex` | `regex` | `pattern` | Create a regex object |
| `regex match` | `regex_دارد` | `regex, text` | Test if regex matches text |

### Math

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `abs` | `مطلق` | `x` | Absolute value |
| `round` | `گرد` | `x` | Round to nearest integer |
| `ceil` | `بالا` | `x` | Round up |
| `floor` | `پایین` | `x` | Round down |
| `max` | `بیشترین` | `*args` | Maximum of arguments |
| `min` | `کمترین` | `*args` | Minimum of arguments |

### Iteration

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `range` | `محدوده` | `stop` or `start, stop[, step]` | Generate a range of integers |

### List Operations

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `append` | `اضافه` | `list, item` | Append item to list (returns modified list) |
| `delete` | `حذف` | `list, index` | Remove item at index |
| `sort` | `مرتب` | `list` | Sort list in place |
| `reverse` | `برعکس` | `list` | Reverse list in place |
| `find` | `پیدا` | `list, item` | Find index of item |

### String Methods

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `upper` | `بزرگ` | `s` | Convert to uppercase |
| `lower` | `کوچک` | `s` | Convert to lowercase |
| `strip` | `برش` / `فاصله_نگیر` | `s` | Strip whitespace |
| `split` | `جدا_کن` | `s, sep` | Split string |
| `join` | `بچسبان` | `sep, list` | Join strings |
| `contains` | `تکرار_داره` | `s, sub` | Check if substring exists |

### Dict Operations

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `keys` | `کلیدها` | `dict` | Get list of keys |
| `values` | `مقدارها` | `dict` | Get list of values |
| `has` | `دارد` | `dict, key` | Check if key exists |

### Set Operations

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `set_add` | `عضو_اضافه` | `set, item` | Add item to set |
| `union` | `اجتماع` | `set1, set2` | Set union |
| `intersection` | `اشتراک` | `set1, set2` | Set intersection |
| `difference` | `تفاضل` | `set1, set2` | Set difference |

### File I/O

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `read_file` | `فایل_بخوان` | `path` | Read UTF-8 text file |
| `write_file` | `فایل_بنویس` | `path, text` | Write UTF-8 text file |
| `file_exists` | `وجود_دارد` | `path` | Check if file/directory exists |
| `list_dir` | `پوشه_لیست` | `path` | List directory contents |
| `path_join` | `مسیر_بچسبان` | `*parts` | Join path components |

### System

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `environ` | `محیط` | `name` | Get environment variable |
| `system` | `فرمان` | `cmd` | Execute shell command |
| `exit` | `خروج` | `code=0` | Exit the program |
| `random` | `تصادفی` | `()` | Random float in [0, 1) |
| `time` | `زمان` | `()` | Current Unix timestamp |
| `now` | `زمان_اکنون` | `()` | Current datetime as ISO string |

### Aggregation

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `any` | `هر_کدام` | `iterable` | True if any element is truthy |
| `all` | `همه` | `iterable` | True if all elements are truthy |

### Equality Protocol

| Function | Persian | Parameters | Description |
|---|---|---|---|
| `equals` | `مثل_هست` | `a, b` | Structural equality |

---

## 12. Standard Library

### Math Module (`math` / `ریاضی`)

**Constants:** `pi`, `e`, `tau`

**Functions:**
`sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`, `log`, `log10`, `exp`, `sqrt`, `pow`, `trunc`, `fmod`, `hypot`, `copysign`, `isfinite`, `isinf`, `isnan`, `gcd`, `lcm`, `comb`, `perm`, `factorial`, `isclose`, `degrees`, `radians`, `prod`, `dist`

**Persian aliases:** `abs`=`مطلق`, `round`=`گرد`, `ceil`=`بالا`, `floor`=`پایین`, `max`=`بیشترین`, `min`=`کمترین`

**Pure Azravibe additions:**
- `prod(items)` -- Multiply all items
- `dist(a, b)` -- Euclidean distance

**Persian math module (`ریاضی`):**
- `توان(پایه، توان)` -- Power
- `جذر(x)` -- Square root
- `فاکتوریل(n)` -- Factorial (recursive)
- `اول_هست(n)` -- Primality test

---

### JSON Module (`json`)

| Function | Description |
|---|---|
| `loads(s)` / `parse(s)` | Parse JSON string to value |
| `dumps(v)` / `stringify(v)` | Serialize value to JSON string |
| `load(path)` | Read and parse JSON file |
| `dump(value, path)` | Serialize and write JSON file |
| `pretty(value)` | Alias for `dumps` |
| `clone(value)` | Deep clone via JSON round-trip |

---

### CSV Module (`csv`)

| Function | Description |
|---|---|
| `parse(text)` / `reader(text)` | Parse CSV string to list of lists |
| `stringify(rows)` / `writer(rows)` | Serialize list of lists to CSV |
| `load(path)` / `read(path)` | Read and parse CSV file |
| `dump(rows, path)` / `write(path, rows)` | Write CSV file |
| `rows_to_dicts(rows)` / `DictReader(rows)` | Convert rows to list of dicts using first row as header |
| `dicts_to_rows(items)` | Convert list of dicts to list of lists |
| `DictWriter(items)` | Serialize list of dicts to CSV string |

---

### DateTime Module (`datetime` / `زمان`)

| Function | Description |
|---|---|
| `now()` / `زمان_اکنون()` | Current time as ISO string |
| `timestamp()` / `زمان()` | Current Unix timestamp |
| `today()` / `date()` | Current date as "YYYY-MM-DD" |
| `strftime(ts, fmt)` | Format timestamp with format string |
| `year(ts)`, `month(ts)`, `day(ts)` | Extract date parts |
| `hour(ts)`, `minute(ts)`, `second(ts)` | Extract time parts |
| `weekday(ts)` | Day of week (0=Monday) |
| `isoformat(ts)` | Format as ISO 8601 |
| `parts(ts)` | Get dict with all date/time parts |
| `parse_iso(s)` | Parse ISO datetime string to timestamp |

---

### OS Module (`os`)

| Function | Description |
|---|---|
| `getcwd()` | Get current working directory |
| `chdir(path)` | Change working directory |
| `mkdir(path)` | Create directory |
| `remove(path)` / `unlink(path)` | Delete file |
| `rmdir(path)` | Remove directory |
| `rename(old, new)` | Rename/move |
| `getenv(name)` | Get environment variable |
| `setenv(name, value)` | Set environment variable |
| `exists(path)` | Check path exists |
| `listdir(path)` | List directory |
| `stat(path)` | Get file stats |
| `getsize(path)` | Get file size |
| `getmtime(path)` | Get modification time |
| `isfile(path)` | Check if path is file |
| `isdir(path)` | Check if path is directory |
| `walk(path)` | Recursive directory listing |

**Constants:** `name` (platform), `sep` (separator), `linesep` (line separator)

---

### Path Module (`path`)

| Function | Description |
|---|---|
| `join(*parts)` | Join path components |
| `exists(path)` | Check path exists |
| `basename(path)` | Get filename from path |
| `dirname(path)` | Get directory from path |
| `extname(path)` | Get file extension |
| `splitext(path)` | Split into (root, ext) |
| `split(path)` | Split into (dir, basename) |
| `isabs(path)` | Is absolute path |
| `abspath(path)` / `realpath(path)` | Get absolute path |
| `normpath(path)` | Normalize path |
| `expanduser(path)` | Expand `~` to home dir |
| `stem(path)` | Filename without extension |
| `parent(path)` | Alias for dirname |
| `suffix(path)` | Alias for extname |

---

### Random Module (`random` / `تصادفی`)

| Function | Description |
|---|---|
| `random()` / `تصادفی()` | Random float in [0, 1) |
| `randint(a, b)` | Random integer in [a, b] |
| `choice(seq)` | Random element from sequence |
| `seed(n)` | Seed RNG |
| `shuffle(list)` | Shuffle list in place |
| `sample(seq, k)` | Sample k items without replacement |
| `choices(seq, k)` | Sample k items with replacement |
| `uniform(a, b)` | Random float in [a, b) |
| `randrange(start, stop, step)` | Random int in range |
| `getrandbits(n)` | Random int with n bits |
| `expovariate(lam)` | Exponential distribution |
| `gauss(mu, sigma)` | Normal distribution |
| `triangular(low, high)` | Triangular distribution |

---

### HTTP Module (`http`)

Requires `curl` to be installed.

| Function | Description |
|---|---|
| `get(url)` | HTTP GET request |
| `post(url, body)` | HTTP POST request |
| `request(method, url, body)` | Generic HTTP request |
| `head(url)` | HTTP HEAD request |
| `status(url)` | Get HTTP status code |
| `download(url, path)` | Download file |
| `get_json(url)` | GET + JSON parse |
| `post_json(url, data)` | POST JSON data |
| `put(url, body)` | HTTP PUT |
| `patch(url, body)` | HTTP PATCH |
| `delete(url)` | HTTP DELETE |

---

### URL Module (`url`)

| Function | Description |
|---|---|
| `encode(s)` / `quote(s)` | Percent-encode string |
| `decode(s)` / `unquote(s)` | Percent-decode string |
| `parse(url)` / `urlparse(url)` | Parse URL into dict |
| `parse_qs(s)` | Parse query string to dict |
| `parse_qsl(s)` | Parse query string to list of pairs |
| `query(params)` | Build query string from dict |
| `join(base, path)` | Join URL paths |
| `urlunparse(parts)` | Reconstruct URL from parts |

---

### SQLite Module (`sqlite`)

Requires `sqlite3` CLI to be installed.

| Function | Description |
|---|---|
| `connect(path)` | Create connection dict |
| `query(db, sql)` | Execute SQL query (returns CSV) |
| `exec(db, sql)` | Execute SQL (no output) |
| `tables(db)` | List table names |
| `rows(path, sql)` | Query as list of lists |
| `scalar(path, sql)` | Query single value |
| `query_dicts(path, sql)` | Query as list of dicts |
| `executescript(path, script)` | Execute multiple statements |

---

### Socket Module (`socket`)

Uses OS commands (PowerShell on Windows, `nc`/`getent` on Unix). Not a raw socket API.

| Function | Description |
|---|---|
| `open(host, port)` / `create_connection(host, port)` | Check TCP connectivity |
| `can_connect(host, port)` | Check if TCP connection succeeds |
| `wait_for(host, port, tries)` | Retry connection check |
| `resolve(host)` | DNS resolution |
| `hostname()` | Get local hostname |

---

### Subprocess Module (`subprocess`)

| Function | Description |
|---|---|
| `call(cmd)` | Run command, return exit code |
| `getoutput(cmd)` | Run command, return stdout |
| `CompletedProcess(cmd)` | Run command, return dict with stdout/stderr/returncode/ok |
| `check_call(cmd)` | Run command, raise on failure |
| `capture(cmd)` | Alias for CompletedProcess |

---

### Collections Module (`collections`)

| Function | Description |
|---|---|
| `Counter(seq)` | Count items, return dict |
| `counter_elements(counter)` | Expand counter to repeated list |
| `counter_total(counter)` | Sum counter values |
| `defaultdict(default?)` | Dict with default value |
| `deque(items)` | List-based deque |
| `deque_appendleft(d, item)` | Prepend to deque |
| `deque_popleft(d)` | Remove from front |
| `deque_rotate(d, n)` | Rotate elements |
| `most_common(counter)` | Sort by frequency |
| `update(counter, items)` | Increment counts |
| `subtract(counter, items)` | Decrement counts |
| `ChainMap(a, b)` | Merge two dicts |
| `OrderedDict(pairs)` | Create from key-value pairs |

---

### Logging Module (`logging`)

| Function | Description |
|---|---|
| `debug(msg)` | Log at DEBUG level |
| `info(msg)` | Log at INFO level |
| `warning(msg)` | Log at WARNING level |
| `error(msg)` | Log at ERROR level |
| `critical(msg)` | Log at CRITICAL level |
| `config(level_or_dict, file?)` | Configure logging |

---

### Base64 Module (`base64`)

| Function | Description |
|---|---|
| `b64encode(s)` / `encode(s)` | Base64 encode |
| `b64decode(s)` / `decode(s)` | Base64 decode |
| `urlsafe_b64encode(s)` | URL-safe base64 encode |
| `urlsafe_b64decode(s)` | URL-safe base64 decode |

---

### Hashlib Module (`hashlib`)

| Function | Description |
|---|---|
| `hexdigest(value, algorithm?)` | Hash value (default: sha256) |

Supported algorithms: `md5`, `sha1`, `sha256`, `sha384`, `sha512`

---

### Argparse Module (`argparse`)

| Function | Description |
|---|---|
| `parse_args(args)` | Parse command-line args to dict |
| `ArgumentParser()` | Create parser dict with parse methods |

---

### Other Modules

**Text (`متن`):** `reverse(s)`, `repeat(s, n)`, `is_numeric(s)`

**List (`لیست`):** `sum(lst)`, `mean(lst)`, `max(lst)`

**File (`فایل`):** Stub -- returns description string only (use built-in `فایل_بخوان`/`فایل_بنویس` instead)

---

### Native C Extensions

Azravibe supports loading native C extension modules. A native module is a `.dll` (Windows), `.so` (Linux), or `.dylib` (macOS) that exports:

```c
AZR_NATIVE_EXPORT int azr_module_init(Environment *module_env, const AzrNativeAPI *api);
```

The `AzrNativeAPI` provides access to value creation, container manipulation, and environment operations.

See `docs/NATIVE_EXTENSIONS.md` in the language source for implementation details.
