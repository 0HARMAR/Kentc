
# Kent 编译器 v1.0 / Kent Compiler v1.0

Kent 是一个极简的编译型语言，专为内存控制与基础算术设计。它采用接近汇编风格的语法，让你能够精准地定义变量、操作内存、进行打印与寻址。

Kent is a minimalist compiled language designed for memory control and basic arithmetic. It uses an assembly-like syntax to precisely define variables, manipulate memory, and perform output and address lookup.

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
- Supports basic arithmetic with standard precedence

---

## 🧪 示例程序 / Example Program

```c++
set int a = 5 at 0x200
a = 2
print a
find a
mov 1 to 0x204
mov a to 0x208

a = 1 + 2 * 3 -10 /5

print a

set int b = 100 at 0x300
b = 2 * 3 + 20/2
print b
find b

set int c = 200 at 0x400
c = 2 + 3 * 3
print c
```

### ✅ 输出 / Output

```
2
200
5
16
300
11
```

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
```

### 🚀 编译 Kent 程序 / Compile Kent programs

```bash
./kentc ../examples/test.kent -o test.out
./test.out
```

## 📦 版本 / Version

当前版本：**Kent v1.0**  
Current Version: **Kent v1.0**

---

## 🧑‍💻 作者 / Author

Made with ❤️ by [harmar]  

