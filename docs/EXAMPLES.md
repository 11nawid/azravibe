# Azravibe Examples

Working code examples demonstrating Azravibe features. Every example uses valid syntax.

---

## Hello World

```azr
بنویس("سلام دنیا!")
```

---

## Variables

```azr
نام = "آذرویب"
سن = 1
نسخه = 0.1
فعال = درسته

بنویس("زبان {نام} نسخه {نسخه}")
بنویس("وضعیت: {فعال}")
```

---

## Calculator

```azr
کار ماشین_حساب(oper, a, b):
    اگه oper == "+":
        بده a + b
    وگرنه اگه oper == "-":
        بده a - b
    وگرنه اگه oper == "*":
        بده a * b
    وگرنه اگه oper == "/":
        اگه b == 0:
            بینداز "تقسیم بر صفر ممکن نیست"
        بده a / b
    وگرنه:
        بینداز "عملگر نامعتبر"

بنویس(ماشین_حساب("+", 10، 5))    # 15
بنویس(ماشین_حساب("*", 10، 5))    # 50
بنویس(ماشین_حساب("/", 10، 3))    # 3.333...
```

---

## Fibonacci

```azr
کار فیبو(n):
    اگه n <= 1:
        بده n
    بده فیبو(n - 1) + فیبو(n - 2)

برای هر i توی محدوده(15):
    بنویس("فیبو({i}) = {فیبو(i)}")
```

---

## Conditions

```azr
سن = 25

اگه سن >= 65:
    بنویس("بازنشسته")
وگرنه اگه سن >= 18:
    بنویس("بزرگسال")
وگرنه اگه سن >= 13:
    بنویس("نوجوان")
وگرنه:
    بنویس("کودک")
```

---

## Loop with Break and Continue

```azr
# Find first even number > 10
برای هر i توی محدوده(1، 20):
    اگه i % 2 != 0:
        ادامه
    اگه i > 10:
        بنویس("اولین عدد زوج بزرگتر از 10: {i}")
        بزن بیرون
```

---

## List Operations

```azr
# Build and manipulate a list
اعداد = []

برای هر i توی محدوده(10):
    اضافه(اعداد، i * i)

مرتب(اعداد)
بنویس("اعداد: {اعداد}")

برعکس(اعداد)
بنویس("معکوس: {اعداد}")

بنویس("طول: {طول(اعداد)}")
```

---

## Dictionary

```azr
کاربر = {
    "نام": "نوید",
    "سن": 25,
    "شهر": "تهران",
    "فعال": درسته
}

# Access with key
بنویس(کاربر["نام"])

# Access with dot notation
بنویس(کاربر.سن)

# Iterate
برای هر کلید توی کلیدها(کاربر):
    بنویس("{کلید}: {کاربر[کلید]}")
```

---

## Functions

```azr
# Function with defaults and varargs
کار مجموع_با_شروع(شروع=0، *اعداد):
    نتیجه = شروع
    برای هر عدد توی اعداد:
        نتیجه += عدد
    بده نتیجة

بنویس(مجموع_با_شروع())              # 0
بنویس(مجموع_با_شروع(10، 1، 2، 3))  # 16

# Closure
کار ضرب_کن(ضریب):
    کار ضرب(x):
        بده x * ضریب
    بده ضرب

دوبرابر = ضرب_کن(2)
سه_برابر = ضرب_کن(3)
بنویس(دوبرابر(5))    # 10
بنویس(سه_برابر(5))   # 15

# Lambda
مرتب_شده = [10، 5، 8، 3، 1]
مرتب(مرتب_شده)
بنویس(مرتب_شده)  # [1, 3, 5, 8, 10]
```

---

## Classes

```azr
نوع حساب_بانکی:
    کار سازنده(صاحب، اولیه=0):
        خود.صاحب = صاحب
        خود.مانده = اولیه

    کار واریز(مبلغ):
        خود.مانده += مبلغ
        بده خود.مانده

    کار برداشت(مبلغ):
        اگه مبلغ > خود.مانده:
            بینداز "موجودی ناکافی"
        خود.مانده -= مبلغ
        بده خود.مانده

    کار نمایش():
        بده "حساب {خود.صاحب}: {خود.مانده} تومان"

حساب = حساب_بانکی("نوید"، 1000000)
بنویس(حساب.نمایش())
حساب.واریز(500000)
بنویس(حساب.نمایش())
حساب.برداشت(200000)
بنویس(حساب.نمایش())
```

---

## Inheritance

```azr
نوع شکل:
    کار سازنده():
        هیچی

    کار مساحت():
        بده 0

نوع دایره(شکل):
    کار سازنده(شعاع):
        خود.شعاع = شعاع

    کار مساحت():
        بده 3.14159 * خود.شعاع ** 2

نوع مستطیل(شکل):
    کار سازنده(طول، عرض):
        خود.طول = طول
        خود.عرض = عرض

    کار مساحت():
        بده خود.طول * خود.عرض

د = دایره(5)
بنویس("مساحت دایره: {د.مساحت()}")

م = مستطیل(4، 6)
بنویس("مساحت مستطیل: {م.مساحت()}")
```

---

## Error Handling

```azr
کار تقسیم(a, b):
    اگه b == 0:
        بینداز "تقسیم بر صفر"
    بده a / b

# With try/except
سعی کن:
    بنویس(تقسیم(10، 0))
اگه خطا به عنوان e:
    بنویس("گرفتم: {e}")

# Multiple operations
سعی کن:
    اعداد = [1، 2، 3]
    بنویس(اعداد[10])
اگه خطا به عنوان e:
    بنویس("خطا: {e}")
بالاخره:
    بنویس("تمام شد")
```

---

## Modules

```azr
# Create utils.azr:
# کار جمع(a، b):
#     بده a + b

# In main.azr:
از utils بیار جمع
بنویس(جمع(3، 4))

# Or import the whole module
بیار math
بنویس(math.sqrt(16))
بنویس(math.pi)
```

---

## File Operations

```azr
# Write a file
فایل_بنویس("output.txt"، "سلام دنیا!\nاین یک فایل تستی است.")

# Read it back
محتوا = فایل_بخوان("output.txt")
بنویس(محتوا)

# Check existence
اگه وجود_دارد("output.txt"):
    بنویس("فایل وجود دارد")

# List directory
فایلها = پوشه_لیست(".")
بنویس(فایلها)
```

---

## String Processing

```azr
متن = "  سلام دنیای آذرویب  "

# Trim
بنویس(فاصله_نگیر(متن))

# Case
بنویس(بزرگ("hello"))
بنویس(کوچک("HELLO"))

# Split and join
کلمات = جدا_کن("سلام-دنیای-آذرویب"، "-")
بنویس(بچسبان(" | "، کلمات))

# String interpolation with expressions
x = 10
y = 20
بنویس("{x} + {y} = {x + y}")
```

---

## Data Processing with JSON

```azr
بیار json

# Create data
داده = {
    "دانشجوها": [
        {"نام": "علی"، "نمره": 18},
        {"نام": "زهرا"، "نمره": 20},
        {"نام": "محمد"، "نمره": 15}
    ]
}

# Serialize
json_string = json.dumps(داده)
بنویس(json_string)

# Write to file
json.dump(داده، "students.json")

# Read back
بارگذاری = json.load("students.json")
بنویس(بارگذاری.دانشجوها[0].نام)
```

---

## Data Processing with CSV

```azr
بیار csv

# Write CSV
سطرها = [
    ["نام"، "سن"، "شهر"],
    ["علی"، "25"، "تهران"],
    ["زهرا"، "22"، "اصفهان"]
]

csv.dump(سطرها، "people.csv")

# Read CSV
خوانده_شده = csv.load("people.csv")
برای هر سطر توی خوانده_شده:
    بنویس(سطر)

# Convert to dicts
دیکشنری_ها = csv.rows_to_dicts(خوانده_شده)
بنویس(دیکشنری_ها[0].نام)
```

---

## HTTP Request

```azr
بیار http

# GET request
پاسخ = http.get("https://httpbin.org/get")
بنویس(طول(پاسخ))

# GET + parse JSON
داده = http.get_json("https://httpbin.org/get")
بنویس(داده)
```

---

## SQLite Database

```azr
بیار sqlite

# Connect
db = sqlite.connect("test.db")

# Create table and insert
sqlite.exec(db، "CREATE TABLE IF NOT EXISTS users (name TEXT، age INTEGER)")
sqlite.exec(db، "INSERT INTO users VALUES ('Ali'، 25)")
sqlite.exec(db، "INSERT INTO users VALUES ('Sara'، 22)")

# Query
نتیجه = sqlite.query(db، "SELECT * FROM users")
بنویس(نتیجه)

# List tables
جدولها = sqlite.tables(db)
بنویس(جدولها)
```

---

## Datetime

```azr
بیار datetime

# Current time
بنویس(datetime.now())
بنویس(datetime.today())

# Extract parts
زمان = datetime.timestamp()
بنویس("سال: {datetime.year(زمان)}")
بنویس("ماه: {datetime.month(زمان)}")
بنویس("روز: {datetime.day(زمان)}")
```

---

## Random Numbers

```azr
بیار random

# Random integers
برای هر _ توی محدوده(5):
    بنویس(random.randint(1، 100))

# Random choice from list
رنگها = ["قرمز"، "سبز"، "آبی"، "زرد"]
بنویس(random.choice(رنگها))

# Shuffle
اعداد = [1، 2، 3، 4، 5]
random.shuffle(اعداد)
بنویس(اعداد)
```

---

## Small CLI Application

```azr
بیار argparse
بیار json

# Parse command-line arguments
پارسر = argparse.ArgumentParser()
آرگومان‌ها = پارسر.parse_args(sys.argv)

# Simple counter program
شمارنده = 0
تعداد = 10

اگه طول(sys.argv) > 1:
    تعداد = عدد_صحیح(sys.argv[1])

تا وقتی شمارنده < تعداد:
    شمارنده += 1
    بنویس("{شمارنده}: سلام")
```

---

## With Statement

```azr
نوع فایل_نویس:
    کار سازنده(مسیر، محتوا):
        خود.مسیر = مسیر
        خود.محتوا = محتوا

    کار ورود():
        فایل_بنویس(خود.مسیر، خود.محتوا)
        بده خود

    کار خروج():
        بنویس("فایل بسته شد")

با فایل_نویس("test.txt"، "سلام") به عنوان ف:
    بنویس("نوشت: {ف.مسیر}")
```

---

## Summing List Values

```azr
بیار لیست

اعداد = [10، 20، 30، 40، 50]

مجموع = لیست.مجموع(اعداد)
بنویس("مجموع: {مجموع}")        # 150

میانگین = لیست.میانگین(اعداد)
بنویس("میانگین: {میانگین}")    # 30.0
```
