# Makefile 语法参考手册

> 本文档面向 Makefile 初学者，介绍 Makefile 的基本语法、规则和最佳实践。

---

## 目录

1. [什么是 Makefile](#什么是-makefile)
2. [基本结构](#基本结构)
3. [规则（Rules）](#规则rules)
4. [变量](#变量)
5. [自动变量](#自动变量)
6. [常用函数](#常用函数)
7. [条件判断](#条件判断)
8. [伪目标](#伪目标)
9. [模式规则](#模式规则)
10. [实用技巧](#实用技巧)
11. [完整示例](#完整示例)

---

## 什么是 Makefile

**Makefile** 是一个文本文件，定义了：
- 如何编译和链接程序
- 源文件之间的依赖关系
- 构建目标所需的命令

**make** 工具读取 Makefile，自动决定哪些文件需要重新编译，大大简化了构建过程。

---

## 基本结构

```makefile
# 这是注释

目标: 依赖文件
[TAB]命令
```

**关键要素**：
- **目标（Target）**：要生成的文件或执行的操作名称
- **依赖（Prerequisites）**：生成目标所需的文件
- **命令（Commands）**：生成目标需要执行的 shell 命令
- **必须用 Tab 键缩进**（不能用空格！）

### 简单示例

```makefile
hello: hello.c
	gcc hello.c -o hello
```

执行：
```bash
make hello    # 编译 hello.c 生成 hello
```

---

## 规则（Rules）

### 显式规则

明确定义目标和依赖：

```makefile
# 编译程序
app: main.o utils.o
	gcc main.o utils.o -o app

# 编译目标文件
main.o: main.c
	gcc -c main.c -o main.o

utils.o: utils.c
	gcc -c utils.c -o utils.o
```

### 多目标规则

```makefile
clean distclean:
	rm -f *.o
	rm -f app
```

### 多行命令

```makefile
clean:
	rm -f *.o
	rm -f app
	echo "Clean done!"
```

---

## 变量

### 变量定义方式

| 符号 | 名称 | 说明 | 示例 |
|------|------|------|------|
| `=` | 递归展开 | 使用时才展开 | `VAR = $(OTHER)` |
| `:=` | 简单展开 | 定义时立即展开 | `VAR := $(OTHER)` |
| `?=` | 条件赋值 | 未定义时才赋值 | `VAR ?= default` |
| `+=` | 追加 | 追加到现有值 | `VAR += more` |

### 变量定义示例

```makefile
# 递归展开（延迟展开）
VAR1 = $(VAR2)          # VAR2 的值在 VAR1 使用时才确定
VAR2 = hello

# 简单展开（立即展开）
VAR3 := $(VAR2)         # VAR3 立即获得 VAR2 的当前值
VAR2 = world            # VAR3 仍然是 "hello"

# 条件赋值（仅当变量未定义时）
CC ?= gcc               # 如果 CC 未定义，设为 gcc

# 追加
CFLAGS = -Wall
CFLAGS += -g            # CFLAGS 现在是 "-Wall -g"
```

### 变量引用

```makefile
CC = gcc
CFLAGS = -Wall -g

app: main.c
	$(CC) $(CFLAGS) main.c -o app
	@echo "Compiler: $(CC)"
```

### 预定义变量

| 变量 | 默认值 | 说明 |
|------|--------|------|
| `CC` | cc | C 编译器 |
| `CXX` | g++ | C++ 编译器 |
| `CFLAGS` | (空) | C 编译选项 |
| `CXXFLAGS` | (空) | C++ 编译选项 |
| `LDFLAGS` | (空) | 链接选项 |
| `RM` | rm -f | 删除命令 |

---

## 自动变量

自动变量在规则的命令中使用，代表特定的文件名：

| 变量 | 含义 |
|------|------|
| `$@` | 目标文件名 |
| `$<` | 第一个依赖文件名 |
| `$^` | 所有依赖文件（去重） |
| `$+` | 所有依赖文件（保留重复） |
| `$?` | 比目标新的依赖文件 |
| `$(@D)` | 目标文件的目录部分 |
| `$(@F)` | 目标文件的文件名部分 |

### 自动变量示例

```makefile
app: main.o utils.o
	gcc $^ -o $@           # $^ = main.o utils.o, $@ = app

main.o: main.c
	gcc -c $< -o $@        # $< = main.c, $@ = main.o

# 安装规则
install: app
	cp $< $(DESTDIR)/bin/  # $< = app
```

---

## 常用函数

### 字符串函数

```makefile
# subst: 字符串替换
$(subst from,to,text)
# 示例: $(subst .c,.o,main.c utils.c) → main.o utils.o

# patsubst: 模式替换
$(patsubst pattern,replacement,text)
# 示例: $(patsubst %.c,%.o,main.c utils.c) → main.o utils.o

# strip: 去除首尾空格
$(strip string)

# findstring: 查找字符串
$(findstring find,in)

# filter: 过滤符合模式的单词
$(filter pattern...,text)
# 示例: $(filter %.c %.h,main.c main.h main.o) → main.c main.h

# filter-out: 过滤掉符合模式的单词
$(filter-out pattern...,text)
```

### 文件名函数

```makefile
# dir: 提取目录部分
$(dir src/main.c)        # → src/

# notdir: 提取文件名
$(notdir src/main.c)     # → main.c

# basename: 去掉后缀
$(basename src/main.c)   # → src/main

# addsuffix: 添加后缀
$(addsuffix .o,main utils)  # → main.o utils.o

# addprefix: 添加前缀
$(addprefix obj/,main.o utils.o)  # → obj/main.o obj/utils.o

# join: 连接单词
$(join a b,.c .o)        # → a.c b.o

# wildcard: 展开通配符
$(wildcard *.c)          # → main.c utils.c

# realpath: 获取绝对路径
$(realpath ../file.txt)
```

### 其他实用函数

```makefile
# foreach: 循环
$(foreach var,list,text)
# 示例:
DIRS = src obj bin
$(foreach dir,$(DIRS),mkdir -p $(dir);)

# if: 条件
$(if condition,then-part,else-part)
# 示例: $(if $(DEBUG),-g -O0,-O2)

# call: 调用自定义函数
define compile
	gcc $(CFLAGS) -c $1 -o $2
endef

main.o: main.c
	$(call compile,$<,$@)

# shell: 执行 shell 命令
$(shell command)
# 示例: $(shell date +%Y%m%d)

# origin: 查询变量来源
$(origin VAR)            # 返回: undefined, default, environment, file, command line 等
```

---

## 条件判断

```makefile
# ifeq: 如果相等
ifeq ($(CC),gcc)
    CFLAGS += -Wall
endif

# ifneq: 如果不相等
ifneq ($(DEBUG),)
    CFLAGS += -g -DDEBUG
endif

# ifdef: 如果定义了
ifdef DEBUG
    CFLAGS += -g
endif

# ifndef: 如果未定义
ifndef INSTALL_DIR
    INSTALL_DIR = /usr/local
endif

# else 和 endif
ifeq ($(OS),Windows_NT)
    EXE = .exe
else
    EXE =
endif
```

---

## 伪目标

伪目标不对应实际文件，用于执行操作：

```makefile
.PHONY: clean install all test

all: app

app: main.o utils.o
	gcc $^ -o $@

clean:
	rm -f *.o app

install: app
	cp app $(INSTALL_DIR)/bin/

test: app
	./app --test
```

**为什么要用 .PHONY？**
- 防止与同名文件冲突
- 确保命令总是执行（即使存在同名文件）

---

## 模式规则

### 静态模式规则

```makefile
objects = main.o utils.o helpers.o

$(objects): %.o: %.c
	gcc -c $< -o $@
```

含义：对于 `objects` 中的每个 `.o` 文件，使用对应的 `.c` 文件编译。

### 隐式模式规则

```makefile
# 通用规则：所有 .o 文件都依赖于对应的 .c 文件
%.o: %.c
	gcc -c $(CFLAGS) $< -o $@

# 多级目录
%.o: %.c
	@mkdir -p $(dir $@)
	gcc -c $< -o $@
```

### 模式规则示例

```makefile
# 编译 C 文件
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 编译汇编文件
%.o: %.asm
	$(NASM) -f elf64 $< -o $@

# 链接可执行文件
%: %.o
	$(CC) $(LDFLAGS) $^ -o $@
```

---

## 实用技巧

### 1. 自动推导依赖

```makefile
# 使用 gcc -MM 自动生成头文件依赖
deps := $(objects:.o=.d)
-include $(deps)

%.d: %.c
	@$(CC) -MM $< > $@
```

### 2. 多目录项目

```makefile
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $< -o $@

$(BIN_DIR)/app: $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CC) $^ -o $@
```

### 3. 调试技巧

```bash
make -n          # 只打印命令，不执行（干运行）
make -d          # 打印调试信息
make -p          # 打印所有规则和变量
make -C dir      # 切换到目录执行
make -j4         # 并行构建（4个任务）
make -k          # 遇到错误继续
```

### 4. 包含其他 Makefile

```makefile
include config.mk
include $(wildcard *.d)
```

### 5. 定义多行变量

```makefile
define compile_rule
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@
	@echo "Done!"
endef

main.o: main.c
	$(compile_rule)
```

### 6. 静默执行

```makefile
# 在命令前加 @ 不显示命令本身
main.o: main.c
	@echo "Compiling..."
	@$(CC) -c $< -o $@

# 或使用 .SILENT
.SILENT: clean
```

---

## 完整示例

### 示例 1：C 项目 Makefile

```makefile
# 项目配置
PROJECT = myapp
VERSION = 1.0

# 目录
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# 编译器设置
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2
CFLAGS += -I./include
LDFLAGS =

# 调试模式
ifdef DEBUG
    CFLAGS += -g -O0 -DDEBUG
endif

# 文件
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))
TARGET = $(BIN_DIR)/$(PROJECT)

# 默认目标
.PHONY: all clean install debug

all: $(TARGET)

# 链接
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@echo "Linking $@..."
	$(CC) $(LDFLAGS) $^ -o $@

# 编译
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# 创建目录
$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

# 调试构建
debug:
	$(MAKE) DEBUG=1

# 清理
clean:
	@echo "Cleaning..."
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# 安装
install: $(TARGET)
	@echo "Installing to /usr/local/bin..."
	cp $(TARGET) /usr/local/bin/

# 打印变量（调试用）
print:
	@echo "SOURCES: $(SOURCES)"
	@echo "OBJECTS: $(OBJECTS)"
	@echo "TARGET: $(TARGET)"
```

### 示例 2：OS 引导项目 Makefile

```makefile
# 汇编器
NASM = nasm
QEMU = qemu-system-x86_64
BOCHS = bochs
BXIMAGE = bximage

# 目标文件
TARGET = boot.bin
SRC = boot.asm
HD_IMG = hd.img

# 默认目标
.PHONY: all clean run debug bochs

all: $(TARGET)

# 编译引导扇区
$(TARGET): $(SRC)
	$(NASM) -f bin $< -o $@

# 运行 (QEMU)
run: $(HD_IMG)
	$(QEMU) -hda $(HD_IMG)

# 调试模式 (QEMU + GDB)
debug: $(HD_IMG)
	$(QEMU) -hda $(HD_IMG) -s -S

# 运行 (Bochs)
bochs: $(HD_IMG) bochsrc
	$(BOCHS) -f bochsrc -q

# 生成 Bochs 配置文件
bochsrc:
	@echo "megs: 32" > $@
	@echo "ata0: enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14" >> $@
	@echo "ata0-master: type=disk, path=$(HD_IMG), mode=flat" >> $@
	@echo "boot: disk" >> $@
	@echo "log: bochs.log" >> $@
	@echo "display_library: x" >> $@

# 生成硬盘镜像
$(HD_IMG): $(TARGET)
	$(BXIMAGE) -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
	dd if=$(TARGET) of=$@ conv=notrunc

# 清理
clean:
	rm -f $(TARGET) $(HD_IMG) bochs.log bochsrc
```

---

## 常用规则速查表

| 目标 | 用途 |
|------|------|
| `all` | 构建所有目标（默认） |
| `clean` | 删除生成的文件 |
| `install` | 安装到系统 |
| `uninstall` | 卸载 |
| `test` | 运行测试 |
| `dist` | 创建发布包 |
| `help` | 显示帮助信息 |

---

## 总结

| 概念 | 关键语法 |
|------|----------|
| **规则** | `目标: 依赖` + Tab + `命令` |
| **变量** | `NAME = value`, `$(NAME)` |
| **自动变量** | `$@`, `$<`, `$^` |
| **伪目标** | `.PHONY: target` |
| **模式规则** | `%.o: %.c` |
| **函数** | `$(function args)` |
| **条件** | `ifeq`, `ifdef` |

---

> **提示**：Makefile 的核心是**依赖关系**。想清楚文件之间的依赖，Makefile 就写对了一半。多实践，从简单项目开始！
