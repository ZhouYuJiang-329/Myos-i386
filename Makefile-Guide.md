# Makefile 使用指南

本文档详细说明 `Makefile` 中所有命令的用途和使用方法。

---

## 目录

1. [变量定义](#变量定义)
2. [编译命令](#编译命令)
3. [QEMU 运行命令](#qemu-运行命令)
4. [Bochs 运行命令](#bochs-运行命令)
5. [镜像生成命令](#镜像生成命令)
6. [清理命令](#清理命令)
7. [完整命令速查表](#完整命令速查表)

---

## 变量定义

| 变量 | 值 | 说明 |
|------|-----|------|
| `NASM` | `nasm` | NASM 汇编器 |
| `QEMU` | `qemu-system-x86_64` | QEMU 模拟器 |
| `BOCHS` | `bochs` | Bochs 模拟器 |
| `BXIMAGE` | `bximage` | Bochs 镜像工具 |
| `TARGET` | `boot.bin` | 编译目标文件 |
| `SRC` | `boot.asm` | 源代码文件 |
| `HD_IMG` | `hd.img` | 硬盘镜像文件名 |

---

## 编译命令

### `make` 或 `make all`

**功能**: 编译 boot.asm 生成 boot.bin

**执行过程**:
```bash
nasm -f bin boot.asm -o boot.bin
```

**参数说明**:
- `-f bin`: 输出纯二进制格式（无头部信息）
- `boot.asm`: 输入源文件
- `-o boot.bin`: 指定输出文件名

**用途**: 这是默认目标，只编译不运行。

---

## QEMU 运行命令

### `make run`

**功能**: 编译并启动 QEMU 运行硬盘镜像

**执行过程**:
```bash
qemu-system-x86_64 -hda hd.img
```

**参数说明**:
- `-hda hd.img`: 将 hd.img 作为硬盘 A 加载

**依赖**: 自动编译 boot.bin 并生成 hd.img 镜像

**用途**: 快速测试引导扇区程序，看是否能正常显示 "Hello World"。

---

### `make debug`

**功能**: 以调试模式启动 QEMU（使用硬盘镜像）

**执行过程**:
```bash
qemu-system-x86_64 -hda hd.img -s -S
```

**参数说明**:
- `-hda hd.img`: 将 hd.img 作为硬盘 A 加载
- `-s`: 在 1234 端口启动 GDB 服务器
- `-S`: 启动时暂停 CPU，等待 GDB 连接

**依赖**: 自动编译 boot.bin 并生成 hd.img 镜像

**用途**: 用于配合 GDB 进行源码级调试。

**GDB 连接示例**:
```bash
gdb
target remote localhost:1234
set architecture i8086
continue
```

---

## Bochs 运行命令

### `make bochs`

**功能**: 使用 Bochs 运行硬盘镜像

**执行过程**:
1. 生成 `hd.img` 硬盘镜像（如果不存在）
2. 生成 `bochsrc` 配置文件
3. 启动 Bochs: `bochs -f bochsrc -q`

**生成的 bochsrc 配置**:
```
megs: 32
ata0: enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14
ata0-master: type=disk, path=hd.img, mode=flat
boot: disk
log: bochs.log
mouse: enabled=0
display_library: x, options=gui_debug
```

**参数说明**:
- `megs: 32`: 分配 32MB 内存
- `ata0`: 启用 ATA/IDE 控制器
- `ata0-master`: 主硬盘使用 hd.img
- `boot: disk`: 从硬盘启动
- `-q`: 跳过启动菜单直接运行

**依赖**: 自动编译 boot.bin 并生成 hd.img 镜像

**用途**: 使用 Bochs 模拟器测试硬盘启动，更接近真实硬件行为。

---

### `make bochs-debug`

**功能**: 以交互式调试模式启动 Bochs（使用硬盘镜像）

**执行过程**:
```bash
bochs -f bochsrc
```

**特点**:
- 不添加 `-q` 参数，显示启动菜单
- 可以选择启动调试器
- 可以单步执行、查看寄存器等

**依赖**: 自动编译 boot.bin 并生成 hd.img 镜像

**用途**: 需要详细调试时使用，Bochs 内置调试功能非常强大。

---

## 镜像生成命令

### `make hd`

**功能**: 生成硬盘镜像 hd.img

**执行过程**:
```bash
bximage -q -hd=16 -func=create -sectsize=512 -imgmode=flat hd.img
dd if=boot.bin of=hd.img conv=notrunc
```

**bximage 参数说明**:
- `-q`: 安静模式
- `-hd=16`: 创建 16 个扇区的镜像
- `-func=create`: 创建模式
- `-sectsize=512`: 每个扇区 512 字节
- `-imgmode=flat`: 扁平模式（简单文件）

**dd 命令说明**:
- `if=boot.bin`: 输入文件
- `of=hd.img`: 输出文件
- `conv=notrunc`: 不截断输出文件

**镜像大小**: 16 扇区 × 512 字节 = 8KB

**依赖**: 自动编译 boot.bin

**用途**: 创建一个小型硬盘镜像，用于测试硬盘启动。

---

## 清理命令

### `make clean`

**功能**: 删除所有生成的文件

**执行过程**:
```bash
rm -f boot.bin hd.img bochsrc bochs.log
```

**删除的文件**:
- `boot.bin`: 编译生成的二进制文件
- `hd.img`: 硬盘镜像
- `bochsrc`: Bochs 硬盘配置文件
- `bochs.log`: Bochs 日志文件

**用途**: 清理工作目录，重新开始。

---

## 完整命令速查表

| 命令 | 功能 | 使用场景 |
|------|------|----------|
| `make` | 编译 boot.bin | 只编译不运行 |
| `make run` | QEMU 运行硬盘镜像 | 快速测试 |
| `make debug` | QEMU 调试模式 | 配合 GDB 调试 |
| `make bochs` | Bochs 运行硬盘镜像 | 使用 Bochs 测试 |
| `make bochs-debug` | Bochs 交互调试 | 详细调试 |
| `make hd` | 生成硬盘镜像 | 创建 hd.img |
| `make clean` | 清理文件 | 重新开始 |

---

## 典型工作流程

### 1. 快速测试（推荐）

```bash
make run
```

### 2. 使用 Bochs 测试

```bash
make bochs
```

### 3. 调试流程

```bash
# 终端 1
make debug

# 终端 2
gdb
target remote localhost:1234
set architecture i8086
break *0x7c00
continue
```

### 4. 完整清理后重新构建

```bash
make clean
make run
```

---

## 注意事项

1. **依赖关系**: 所有运行命令都依赖 `hd.img`，会自动触发编译和镜像生成
2. **统一使用硬盘**: 当前 Makefile 统一使用硬盘镜像（hd.img），不再使用软盘
3. **QEMU vs Bochs**: 
   - QEMU 更快，适合快速测试
   - Bochs 更精确，适合学习硬件细节
4. **镜像大小**: hd.img 只有 8KB，足够存放引导扇区，后续可以扩展
5. **权限问题**: 某些系统可能需要 `sudo` 运行模拟器

---

## 相关文件说明

| 文件 | 说明 |
|------|------|
| `boot.asm` | 引导扇区源代码 |
| `boot.bin` | 编译后的引导扇区（512字节） |
| `hd.img` | 硬盘镜像文件 |
| `bochsrc` | Bochs 硬盘配置文件 |
| `bochs.log` | Bochs 运行日志 |
| `Makefile` | 构建脚本 |
| `Makefile-Guide.md` | 本文档 |
