# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

这是一个 x86 操作系统内核项目，实现了从实模式引导到 32 位保护模式的完整启动流程。项目使用汇编语言和 C 语言编写，针对 QEMU 和 Bochs 模拟器进行测试。

## 构建和运行

```bash
make          # 编译生成 hd.img 硬盘镜像
make run      # 使用 QEMU 运行
make bochs    # 使用 Bochs 运行
make debug    # QEMU 调试模式（配合 GDB 使用）
make clean    # 清理生成文件
```

GDB 调试连接：
```bash
# 终端 1: 启动 QEMU 调试模式
make debug

# 终端 2: 连接 GDB
gdb
target remote localhost:1234
set architecture i8086
```

## 启动流程架构

代码按执行顺序分为三个阶段，全部以裸机二进制格式输出：

1. **boot.asm**（`oskernel/boot/boot.asm`）— 引导扇区，实模式 16 位
   - 加载到 0x7C00，BIOS 从硬盘第一扇区加载
   - 显示 "Hello World boot!"，从硬盘读取 setup.bin 到 0x500
   - 跳转到 0x500 执行 setup

2. **setup.asm**（`oskernel/boot/setup.asm`）— Setup 阶段，实模式 16 位
   - 加载到 0x500，读取内核到 0x1200（60 个扇区）
   - 执行保护模式切换：关中断 → 启用 A20 地址线 → 加载 GDT → 切换 CR0 PE 位
   - 远跳转到保护模式入口，跳转到 0x1200 执行内核

3. **head.asm + main.c**（`oskernel/boot/head.asm` + `init/main.c`）— 内核，32 位保护模式
   - head.asm 是 ELF 入口（`_start`），调用 `kernel_main`
   - main.c 中 `kernel_main` 通过直接写 VGA 显存（0xB8000）输出字符

## 硬盘镜像布局

```
扇区 0 (偏移 0x000):    boot.bin    (1 扇区)
扇区 1-2 (偏移 0x200):  setup.bin   (2 扇区)
扇区 3-62 (偏移 0x600): kernel.bin  (60 扇区)
```

## 内存布局

```
0x0000 - 0x04FF:  BIOS 数据区
0x0500 - 0x05FF:  setup 加载地址（实模式）
0x1200:            内核加载地址（实模式和保护模式）
0x7C00:            引导扇区加载地址
0x90000:           保护模式栈顶
0xB8000:           VGA 显存
```

## GDT 段选择子

- `0x08`（CODE_SEG）：代码段，基址=0，界限=4GB，特权级 0
- `0x10`（DATA_SEG）：数据段，基址=0，界限=4GB，特权级 0

## 编译参数说明

- `boot.bin` / `setup.bin`：`nasm -f bin` 纯二进制格式
- `head.o`：`nasm -f elf32` ELF32 对象格式
- `main.o`：`gcc -m32 -ffreestanding -fno-pic -fno-stack-protector`（无标准库、无 PIC、无栈保护）
- `kernel.bin`：`ld -m elf_i386 -Ttext 0x1200 --oformat binary` 链接到 0x1200 地址

## 关键端口

| 端口 | 用途 |
|------|------|
| 0x1F0-0x1F7 | ATA/IDE 硬盘控制器 |
| 0x60, 0x64 | 键盘控制器（用于 A20 启用） |

## 工具依赖

- NASM：汇编器
- GCC / LD：C 编译器和链接器（需支持 32 位）
- QEMU：快速测试
- Bochs：精确硬件模拟，调试功能更强
