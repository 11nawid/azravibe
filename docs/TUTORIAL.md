# Azravibe Tutorial

A structured learning path covering every feature of the Azravibe language. Each section builds on the previous one.

---

## Table of Contents

1. [Hello World](#1-hello-world)
2. [Variables](#2-variables)
3. [Data Types](#3-data-types)
4. [Operators](#4-operators)
5. [Strings](#5-strings)
6. [Control Flow](#6-control-flow)
7. [Loops](#7-loops)
8. [Functions](#8-functions)
9. [Data Structures](#9-data-structures)
10. [Classes](#10-classes)
11. [Error Handling](#11-error-handling)
12. [Modules and Imports](#12-modules-and-imports)
13. [Advanced Features](#13-advanced-features)

---

## 1. Hello World

The simplest Azravibe program:

```azr
بنویس("سلام دنیا!")
```

`بنویس` is the print function. It outputs its argument to the console, followed by a newline.

---

## 2. Variables

Variables are created by assignment. No declaration keyword is required.

```azr
نام = "آذرویب"
سن = 25
قیمت = 19.99
فعال = درسته
```

Variable names can use Persian/Arabic characters, Latin characters, digits, and underscores. They cannot start with a digit.

```azr
# Valid variable names
نام_کاربر = "ali"
myName = "Ali"
_<unnamed>_ = 1
شماره_۱ = 100

# Invalid variable names
# 1st = 1      # Cannot start with a digit
# my-name = 1  # Hyphens not allowed
```

### Multiple Assignment

```azr
a = b = c = 0
بنویس(a, b, c)  # 0 0 0
```

### Augmented Assignment

```azr
x = 10
x += 5    # x is now 15
x -= 3    # x is now 12
x *= 2    # x is now 24
x /= 4    # x is now 6.0
```

---

## 3. Data Types

Azravibe has dynamic typing. Every value has a type determined at runtime.

### Integer

```azr
سن = 25
بزرگ = 9999999
منفی = -42
```

### Float

```azr
قیمت = 19.99
نرخ = 3.14159
```

### String

```azr
نام = "آذرویب"
پیام = 'سلام'
خالی = ""
```

Strings support escape sequences: `\n` (newline), `\t` (tab), `\\` (backslash), `\"` (double quote), `\x41` (hex), `\u0622` (unicode), `\U00000622` (unicode).

### Boolean

```azr
فعال = درسته    # true
غیرفعال = غلطه  # false
```

### None

```azr
نتیجه = هیچ     # none/null
```

### Type Checking

```azr
بنویس(نوعش(25))        # عدد صحیح
بنویس(نوعش(3.14))      # عدد اعشاری
بنویس(نوعش("سلام"))    # رشته
بنویس(نوعش(درسته))     # بولی
بنویس(نوعش(هیچ))       # هیچ
```

### Type Conversion

```azr
بنویس(عدد_صحیح(3.7))       # 3 (truncates)
بنویس(عدد_صحیح("42"))      # 42
بنویس(عدد_اعشاری(5))       # 5.0
بنویس(متن(100))            # "100"
بنویس(بولی(1))             # درسته
بنویس(بولی(0))             # غلطه
بنویس(بولی(""))            # غلطه
```

---

## 4. Operators

### Arithmetic

```azr
بنویس(10 + 3)     # 13
بنویس(10 - 3)     # 7
بنویس(10 * 3)     # 30
بنویس(10 / 3)     # 3.33333...
بنویس(10 // 3)    # 3 (floor division)
بنویس(10 % 3)     # 1 (modulo)
بنویس(2 ** 8)     # 256 (power)
```

### Comparison

```azr
5 == 5       # درسته
5 != 3       # درسته
5 > 3        # درسته
5 < 3        # غلطه
5 >= 5       # درسته
5 <= 4       # غلطه
```

### Logical

```azr
درسته و غلطه     # غلطه (and)
درسته یا غلطه     # درسته (or)
نه غلطه           # درسته (not)
```

### String Operations

```azr
بنویس("سلام" + " دنیا")     # سلام دنیا
بنویس("ها" * 3)              # هاهاهاآ
```

### Membership

```azr
بنویس(تکرار_داره("hello" + "e"))   # درسته
```

---

## 5. Strings

### String Interpolation

Embed expressions inside strings using `{...}`:

```azr
نام = "آذرویب"
سن = 1
بنویس("زبان {نام} نسخه {سن} ساله است.")
# Output: زبان آذرویب نسخه 1 ساله است.
```

To include literal braces, escape them:

```azr
بنویس("آکولاد {{نمونه}}")
# Output: آکولاد {نمونه}
```

### String Slicing

```azr
متن = "سلام دنیا"
بنویس(متن[0])      # س
بنویس(متن[2:5])    # لام
```

### String Methods

```azr
متن = "  سلام  "
بنویس(بزرگ("hello"))              # HELLO
بنویس(کوچک("HELLO"))             # hello
بنویس(فاصله_نگیر(متن))          # سلام (stripped)
بنویس(تکرار_داره("hello" + "e")) # درسته (contains)
```

---

## 6. Control Flow

### If / Elif / Else

```azr
سن = 20

اگه سن >= 18:
    بنویس("بزرگسال")
وگرنه اگه سن >= 13:
    بنویس("نوجوان")
وگرنه:
    بنویس("کودک")
```

### Nested Conditions

```azr
نمره = 85

اگه نمره >= 90:
    بنویس("عالی")
وگرنه:
    اگه نمره >= 70:
        بنویس("خوب")
    وگرنه:
        بنویس("نیاز به تلاش بیشتر")
```

### Ternary Expression

```azr
وضعیت = درسته
بنویس("بله" اگه وضعیت وگرنه "نه")
```

Also using `? :` syntax:

```azr
بنویس(وضعیت ? "بله" : "نه")
```

---

## 7. Loops

### While Loop

```azr
شمارنده = 0
تا وقتی شمارنده < 5:
    بنویس(شمارنده)
    شمارنده += 1
```

### For-Each Loop

```azr
برای هر میوه توی ["سیب"، "موز"، "پرتقال"]:
    بنویس(میوه)
```

### Range

```azr
# One argument: 0 to n-1
برای هر i توی محدوده(5):
    بنویس(i)

# Two arguments: start to stop-1
برای هر i توی محدوده(2، 6):
    بنویس(i)

# Three arguments: start, stop, step
برای هر i توی محدوده(0، 10، 2):
    بنویس(i)
```

### Break and Continue

```azr
برای هر i توی محدوده(10):
    اگه i == 5:
        بزن بیرون
    بنویس(i)

برای هر i توی محدوده(10):
    اگه i % 2 == 0:
        ادامه
    بنویس(i)
```

---

## 8. Functions

### Defining Functions

```azr
کار جمع(a, b):
    بده a + b

بنویس(جمع(3, 4))  # 7
```

### Default Arguments

```azr
کار سلام(نام="دوست"):
    بنویس("سلام {نام}!")

سلام()              # سلام دوست!
سلام("آذرویب")     # سلام آذرویب!
```

### Varargs

```azr
کار مجموع(*اعداد):
    نتیجه = 0
    برای هر عدد توی اعداد:
        نتیجه += عدد
    بده نتیجه

بنویس(مجموع(1، 2، 3، 4))  # 10
```

### Closures

```azr
کار ضرب‌کن(ضریب):
    کار ضرب(x):
        بده x * ضریب
    بده ضرب

دوبرابر = ضرب‌کن(2)
بنویس(دوبرابر(5))  # 10
```

### Lambda

```azr
دوبرابر = لامبدا(x): x * 2
بنویس(دوبرابر(6))  # 12
```

---

## 9. Data Structures

### Lists

```azr
اعداد = [1، 2، 3، 4، 5]
بنویس(اعداد[0])       # 1
بنویس(اعداد[-1])      # 5
بنویس(اعداد[1:3])     # [2, 3]

اضافه(اعداد، 6)        # Add 6 to the end
بنویس(طول(اعداد))     # 6

مرتب(اعداد)            # Sort
برعکس(اعداد)           # Reverse
```

### Tuples

```azr
جفت = (10، 20)
بنویس(جفت[0])    # 10
# جفت[0] = 5     # Error: tuples are immutable
```

### Dictionaries

```azr
کاربر = {"نام": "نوید"، "سن": 25، "شهر": "تهران"}

بنویس(کاربر["نام"])      # نوید
بنویس(کاربر.نام)          # نوید (dot notation)

کاربر["سن"] = 26          # Update
پاک_کن(کاربر، "شهر")     # Delete key

بنویس(کلیدها(کاربر))      # ["نام", "سن"]
بنویس(مقدارها(کاربر))     # ["نوید", 26]
بنویس(دارد(کاربر، "نام")) # درسته
```

### Sets

```azr
مجموعه = {1، 2، 3}
عضو_اضافه(مجموعه، 4)
بنویس(دارد(مجموعه، 2))  # درسته
```

---

## 10. Classes

### Basic Class

```azr
نوع نفر:
    کار سازنده(نام، سن):
        خود.نام = نام
        خود.سن = سن

    کار سلام():
        بده "سلام، من {خود.نام} هستم و {خود.سن} سالمه."

    کار سن_بعد():
        بده خود.سن + 1

ن = نفر("نوید"، 25)
بنویس(ن.سلام())        # سلام، من نوید هستم و 25 سالمه.
بنویس(ن.سن_بعد())      # 26
```

### Inheritance

```azr
نوع حیوان:
    کار سازنده(نام):
        خود.نام = نام

    کار صدا():
        بده "..."

نوع سگ(حیوان):
    کار صدا():
        بده "ور ور!"

    کار بازی():
        بده "{خود.نام} در حال بازیه"

س = سگ("دوستی")
بنویس(س.صدا())    # ور وور!
بنویس(س.بازی())   # دوستی در حال بازیه
```

### Super

```azr
نوع پایه:
    کار سازنده():
        خود.رنگ = "سفید"

نوع رنگی(پایه):
    کار سازنده(رنگ):
        ابر()           # Call parent constructor
        خود.رنگ = رنگ

ن = رنگی("آبی")
بنویس(ن.رنگ)    # آبی
```

---

## 11. Error Handling

```azr
سعی کن:
    نتیجه = 10 / 0
اگه خطا به عنوان e:
    بنویس("خطا: {e}")
بالاخره:
    بنویس("تمام شد")
```

### Raising Exceptions

```azr
بینداز "یک خطای سفارشی"
```

### Built-in Exception Types

```azr
سعی کن:
    x = متغیر_ناموجود
اگه خطا به عنوان e:
    بنویس(نوعش(e))
```

Built-in exception types include:
- `خطا` (base error)
- `خطای_نام` (name error)
- `خطای_نوع` (type error)
- `خطای_تقسیم_بر_صفر` (division by zero)
- `خطای_شاخص` (index error)
- `خطای_فایل` (file error)
- `خطای_واردکردن` (import error)
- `خطای_فراخوانی` (call error)

---

## 12. Modules and Imports

### Importing a Module

```azr
بیار math
بنویس(math.pi)
بنویس(math.sqrt(16))
```

### From Import

```azr
از math بیار pi، sqrt
بنویس(pi)
بنویس(sqrt(25))
```

### Import with Alias

```azr
بیار math به عنوان ریاضی
بنویس(ریاضی.pi)
```

### Creating a Module

Create a file called `utils.azr`:

```azr
کار جمع_لیست(لیست):
    نتیجه = 0
    برای هر عدد توی لیست:
        نتیجه += عدد
    بده نتیجة

کار میانگین(لیست):
    بده جمع_لیست(لیست) / طول(لیست)
```

Import it:

```azr
از utils بیار جمع_لیست، میانگین
بنویس(جمع_لیست([1، 2، 3]))
بنویس(میانگین([10، 20، 30]))
```

### Project Packages

With a project manifest (`azr.project`), Azravibe automatically searches local package directories for imports.

---

## 13. Advanced Features

### String Interpolation with Expressions

```azr
بنویس("2 + 2 = {2 + 2}")
بنویس("طول نام: {طول("آذرویب")}")
```

### With Statement

```azr
نوع زمینه:
    کار ورود():
        بده "داخل"

    کار خروج():
        بنویس("بسته شد")

با زمینه() به عنوان x:
    بنویس(x)
```

### Comprehensions

```azr
# List comprehension
مربعات = [x ** 2 برای هر x توی محدوده(10)]

# Dict comprehension
مربع_دیکت = {x: x ** 2 برای هر x توی محدوده(5)}
```

### Generators with Yield

```azr
کار شمارنده(n):
    i = 0
    تا وقتی i < n:
        تحویل i
        i += 1

برای هر عدد توی شمارنده(5):
    بنویس(عدد)
```

### Ternary with Question Mark

```azr
نتیجه = درسته ? "بله" : "نه"
```

---

## What's Next?

- [Language Reference](REFERENCE.md) -- Complete technical reference
- [Keyword Reference](KEYWORDS.md) -- All Farsi-to-English keywords
- [Examples](EXAMPLES.md) -- More working code examples
- [Standard Library](REFERENCE.md#standard-library) -- All standard library modules
