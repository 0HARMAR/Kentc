
# Kent 编译器 v1.0 / Kent Compiler v1.0

Kent 是一个极简的编译型语言，它直接面向的对象是数据与操作，程序员可以在无抽象的情况下使用字节数据，你可以随心所欲地操作它们。

---

## 📌 语法指南 / Syntax Guide

### 🔹 变量定义与赋值 Variable Definition and Assignment

```c++
set int a = 1 at 0x200
a = 2
```

- 在地址 `0x200` 定义变量 `a`，初始值为 `1`，随后赋值为 `2`
- Defines variable `a` at address `0x200` with initial value `1`, then assigns `2`

---

### 🔹 内存操作 Memory Operations

```c++
mov 10 to 0x1000
mov a to 0x200
```

- 将 `10` 存入地址 `0x1000`
- 将变量 `a` 的值存入地址 `0x200`

**规则 Rules:**
- `data` 必须是十进制数或变量名 (decimal number or variable)
- `address` 必须是十六进制 (must be hexadecimal)

---

### 🔹 打印与查找 Print and Find

```c++
print a
print 0x1000
find a
```

- 打印变量 `a` 的值
- 打印地址 `0x1000` 的值
- 查找变量 `a` 所在的地址

---

### 🔹 算术运算 Arithmetic Operations

```c++
a = 1 + 2 * 3 - 10 / 5
```

- 支持 `+ - * /` 运算，遵循数学优先级

---

### 🔹 分支操作 SELECTOR

```c++
set int c1 = 1 at 0x100

selector(c1 == 1){
    print c1
    set int c2 = 2 at 0x200
    selector(c2 == 2){
        print c2
    }
}
```

- 支持 == 运算
- 支持嵌套的分支语句

---

### 🔹 循环操作 LOOPER
```c++
set int a = 100 at 0x100

looper(5) {
    a = a + 1
    looper(10) {
        a = a + 2
    }
    a = a + 1
    looper(10) {
        a = a + 1
        looper(10) {
            a = a + 1
        }
    }
    a = a + 2
}

print "a is " + a
```

- 支持指定次数的循环操作

---

### 🔹 函数 FUNCTION
```c++
set int add1 = 10 at 0x100
set int add2 = 20 at 0x104
function add (int a, int b) -> int{
    return a + b
}
set int result = add(add1, add2) at 0x108
print "result is " + result

function fact (int num) -> int {
    selector(num == 1) {
        return 1
    }

    return num * fact(num - 1)
}

set int fact_result = fact(6) at 0x200
print "fact result is " + fact_result
```

- 实现基本函数调用
- 实现递归

---

### 输入操作 INPUT
```c++
set int b = 200 at 0x200

print "please input a add num: "

set byte addNum = 0 at 0x300
in(1,0x300)

selector(addNum == '1') {
    b = b + 1
}

selector(addNum == '2') {
    b = b + 2
}

print "b is " + b
```

- 可指定输入字符数与地址

---

## ⚙️ 构建与运行 / Build & Run (Linux + CMake)

### 🛠️ 步骤 / Steps

```bash
# 1. 克隆 Kent 源码 / Clone Kent source
git clone https://github.com/0HARMAR/kentc.git
cd kentc

# 2. 创建构建目录 / Create build directory
mkdir build && cd build

# 3. 使用 CMake 配置项目 / Configure project with CMake
cmake ..

./kentc -v
```

### 🚀 编译 Kent 程序 / Compile Kent programs

```bash
./kentc demo.kent -o demo
./demo
```

## 📦 版本 / Version

当前版本：**Kent v4.1**  
Current Version: **Kent v4.1**

---

## 🧑‍💻 作者 / Author

Made with ❤️ by [harmar]

