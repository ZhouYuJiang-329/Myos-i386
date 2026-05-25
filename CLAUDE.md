# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 项目概述

这是一个 x86 操作系统内核项目，实现了从实模式引导到 32 位保护模式的完整启动流程。项目使用汇编语言和 C 语言编写，针对 QEMU 和 Bochs 模拟器进行测试。

## 构建和运行

```bash
make          # 编译生成 hd.img 硬盘镜像
make run      # 使用 QEMU 运行（-m 512 -serial stdio，串口输出到终端）
make debug    # QEMU 调试模式（-s -S，配合 GDB 使用）
make bochs    # 使用 Bochs 运行
make bochs-debug  # Bochs 调试模式
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

代码按执行顺序分为四个阶段，boot/setup 以裸机二进制格式输出，内核编译为 ELF 后链接为 raw binary：

1. **boot.asm**（`oskernel/boot/boot.asm`）— 引导扇区，实模式 16 位
   - 加载到 0x7C00，BIOS 从硬盘第一扇区加载
   - 从硬盘读取 setup.bin 到 0x500，跳转到 0x500 执行 setup

2. **setup.asm**（`oskernel/boot/setup.asm`）— Setup 阶段，实模式 16 位
   - 加载到 0x500，执行 E820 物理内存检测（结果存入 0x8000）
   - 执行保护模式切换：关中断 → 启用 A20 → 加载 GDT → 切换 CR0 PE 位
   - 远跳转到保护模式，读取内核到 0x1200（从扇区 3 开始，60 个扇区），跳转到 0x1200

3. **head.asm**（`oskernel/boot/head.asm`）— 内核入口，32 位保护模式
   - `_start` 调用 `kernel_main`

4. **main.c**（`oskernel/init/main.c`）— 内核主体
   - 初始化串口（COM1），通过串口输出所有诊断信息
   - 调用 `print_memory_map()` 打印 E820 内存映射
   - 初始化物理内存管理器（PMM），测试页分配/释放

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
0x1100:            E820 内存映射表（setup.asm 填充，C 代码通过 g_memory_map 访问）
0x90000:           保护模式栈顶
0xB8000:           VGA 显存（备用显示方式）
```

## GDT 段选择子

- `0x08`（CODE_SEG）：代码段，基址=0，界限=4GB，特权级 0
- `0x10`（DATA_SEG）：数据段，基址=0，界限=4GB，特权级 0

## 内核子系统

### 串口驱动（`oskernel/kernel/drivers/serial.c`）
- COM1 端口 0x3F8，波特率 115200，8N1
- API：`serial_init()`, `serial_putc()`, `serial_puts()`, `serial_putline()`, `serial_printf()`（支持 %s, %d, %x, %c）
- 端口 I/O 通过 `outb()`/`inb()` 内联汇编实现（定义在 serial.h）
- QEMU 使用 `-serial stdio` 将串口输出重定向到终端

### 物理内存管理器（`oskernel/kernel/mm/mem_detect.c`）
- 基于位图的页帧分配器，页大小 4KB
- 内存映射表在 setup.asm 中通过 E820 中断检测，存放在物理地址 0x8000
- PMM 位图固定在 0x20000，管理从 1MB 开始的物理内存
- API：`pmm_init()`, `pmm_alloc_page()`, `pmm_free_page()`, `pmm_get_free_pages()`

## 编译参数说明

- `boot.bin` / `setup.bin`：`nasm -f bin` 纯二进制格式
- `head.o`：`nasm -f elf32` ELF32 对象格式
- `main.o` / `serial.o` / `mem_detect.o`：`gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -I.`
- `kernel.bin`：`ld -m elf_i386 -Ttext 0x1200 --oformat binary` 链接到 0x1200 地址

## 关键端口

| 端口 | 用途 |
|------|------|
| 0x1F0-0x1F7 | ATA/IDE 硬盘控制器 |
| 0x3F8-0x3FF | COM1 串口（串口驱动使用） |
| 0x60, 0x64 | 键盘控制器（用于 A20 启用） |

## 添加新源文件

1. 在对应目录（`oskernel/kernel/drivers/` 或 `oskernel/kernel/mm/`）创建 `.c` 和 `.h` 文件
2. 在 `Makefile` 中添加对应的 `XXX_O` 变量和编译规则（参考 `SERIAL_O` 的写法）
3. 将新的 `.o` 文件加入 `KERNEL_BIN` 的依赖列表

## 工具依赖

- NASM：汇编器
- GCC / LD：C 编译器和链接器（需支持 32 位）
- QEMU：快速测试
- Bochs：精确硬件模拟，调试功能更强
