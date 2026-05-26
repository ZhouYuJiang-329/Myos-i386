# QEMU + GDB 调试完全指南

## 目录
1. [环境准备](#1-环境准备)
2. [启动调试](#2-启动调试)
3. [GDB 基础命令](#3-gdb-基础命令)
4. [内存查看命令](#4-内存查看命令)
5. [断点技巧](#5-断点技巧)
6. [寄存器查看](#6-寄存器查看)
7. [实模式 vs 保护模式](#7-实模式-vs-保护模式)
8. [具体调试场景](#8-具体调试场景)
9. [常见问题](#9-常见问题)

---

## 1. 环境准备

### 1.1 所需工具

```bash
# 检查是否已安装
qemu-system-x86_64 --version
gdb --version

# 如果没有安装
sudo apt-get install qemu-system-x86 gdb
```

### 1.2 项目 Makefile 配置

确保 Makefile 中有 debug 目标：

```makefile
debug: $(HD_IMG)
    $(QEMU) -hda $(HD_IMG) -s -S -m 512 -serial stdio
```

参数说明：
- `-s`：启用 GDB 服务器，监听端口 1234
- `-S`：启动时暂停 CPU，等待 GDB 连接
- `-m 512`：分配 512MB 内存
- `-serial stdio`：串口输出到终端

---

## 2. 启动调试

### 2.1 双终端调试流程

**终端 1：启动 QEMU**

```bash
cd /home/zyj/Code/OS
make debug
```

输出：
```
qemu-system-x86_64 -hda hd.img -s -S -m 512 -serial stdio
```

此时 QEMU 窗口黑屏，CPU 已暂停，等待 GDB 连接。

**终端 2：启动 GDB**

```bash
gdb
```

### 2.2 GDB 初始化命令

```gdb
# 连接到 QEMU
target remote localhost:1234

# 设置架构（实模式用 i8086，保护模式用 i386）
set architecture i8086

# 可选：设置显示格式
set disassembly-flavor intel    # Intel 语法（推荐）
# 或
set disassembly-flavor att      # AT&T 语法

# 开始执行
continue
```

---

## 3. GDB 基础命令

### 3.1 执行控制

| 命令 | 缩写 | 功能 | 使用场景 |
|------|------|------|----------|
| `continue` | `c` | 继续执行 | 运行到下一个断点 |
| `stepi` | `si` | 单步执行（进入） | 跟踪函数内部 |
| `nexti` | `ni` | 单步执行（跳过） | 不进入函数内部 |
| `finish` | | 执行到函数返回 | 跳出当前函数 |
| `until` | | 执行到指定行 | 快速跳过循环 |

**示例：**

```gdb
# 从当前位置单步执行
(gdb) si
0x000007c1 in ?? ()

# 继续执行 5 条指令
(gdb) si 5

# 查看接下来要执行的 10 条指令
(gdb) x/10i $pc
```

### 3.2 程序状态

```gdb
# 查看当前指令位置
(gdb) where
#0  0x000007c0 in ?? ()

# 查看调用栈
(gdb) bt
#0  0x000007c0 in ?? ()

# 查看所有寄存器
(gdb) info registers

# 查看特定寄存器
(gdb) print $eax
$1 = 0

# 以十六进制查看
(gdb) print /x $eax
$2 = 0x0
```

---

## 4. 内存查看命令

### 4.1 x 命令格式

```
x/[数量][格式][单位] [地址]
```

| 选项 | 含义 |
|------|------|
| **数量** | 显示多少个单位 |
| **格式** | x=十六进制, d=十进制, u=无符号, o=八进制, t=二进制, i=指令, c=字符, s=字符串 |
| **单位** | b=字节, h=半字(2字节), w=字(4字节), g= giant(8字节) |

### 4.2 常用内存查看示例

```gdb
# 查看 10 个 32 位十六进制值（从 0x1100 开始）
(gdb) x/10wx 0x1100
0x1100:	0x00000002	0x00000000	0x00000000	0x000A0000
0x1110:	0x00000000	0x00000001	0x00100000	0x00000000

# 查看 5 个 64 位值（内存映射条目）
(gdb) x/5gx 0x1104
0x1104:	0x0000000000000000	0x00000000000A0000
0x1114:	0x0000000100000000	0x0000000000100000
0x1124:	0x0000000000000001

# 查看 20 条指令
(gdb) x/20i 0x7c00
   0x7c00:	cli
   0x7c01:	xor    ax,ax
   0x7c03:	mov    ds,ax
   ...

# 查看字符串
(gdb) x/s 0x7c20
0x7c20:	"hello boot!"

# 查看字节（VGA 显存）
(gdb) x/10bx 0xb8000
0xb8000:	0x48	0x0f	0x65	0x0f	0x6c	0x0f	0x6c	0x0f
0xb8008:	0x6f	0x0f
```

### 4.3 基于寄存器的查看

```gdb
# 查看 CS:IP 处的指令
(gdb) x/5i $cs*16+$eip

# 查看栈顶
(gdb) x/10wx $esp

# 查看 DS:SI 处的数据
(gdb) x/10bx $ds*16+$esi
```

---

## 5. 断点技巧

### 5.1 基础断点

```gdb
# 在绝对地址设置断点
(gdb) break *0x7c00
Breakpoint 1 at 0x7c00

# 在标签地址（如果有符号表）
(gdb) break *kernel_main

# 查看所有断点
(gdb) info breakpoints
Num     Type           Disp Enb Address    What
1       breakpoint     keep y   0x000007c0

# 删除断点
(gdb) delete 1

# 禁用/启用断点
(gdb) disable 1
(gdb) enable 1
```

### 5.2 条件断点

```gdb
# 当 eax 等于特定值时停止
(gdb) break *0x500 if $eax == 0x534D4150

# 当某个内存地址被写入时
(gdb) watch *0x1100

# 当寄存器被修改时
(gdb) watch $ebx
```

### 5.3 临时断点

```gdb
# 只触发一次的断点
(gdb) tbreak *0x1200

# 执行到指定地址后删除断点
(gdb) advance *0x1200
```

---

## 6. 寄存器查看

### 6.1 实模式下的寄存器

```gdb
# 查看段寄存器和偏移
(gdb) info registers ax bx cx dx si di sp bp
(gdb) info registers cs ds es ss fs gs

# 计算物理地址
(gdb) print /x $cs*16+$eip    # CS:IP 物理地址
$1 = 0x7c00

(gdb) print /x $ds*16+$esi    # DS:SI 物理地址
$2 = 0x500
```

### 6.2 保护模式下的寄存器

```gdb
# 切换到保护模式后，使用 32 位寄存器
(gdb) set architecture i386

# 查看 32 位寄存器
(gdb) info registers eax ebx ecx edx esi edi esp ebp eip eflags

# 查看段选择子
(gdb) info registers cs ss ds es fs gs

# 查看 CR0 控制寄存器（判断是否进入保护模式）
(gdb) print /x $cr0
$1 = 0x60000011    # PE=1 表示保护模式
```

### 6.3 标志位解析

```gdb
# 查看 EFLAGS
(gdb) print /x $eflags
$1 = 0x202

# 解析标志位
(gdb) print $eflags & 0x1    # CF (进位标志)
(gdb) print ($eflags >> 6) & 0x1    # ZF (零标志)
(gdb) print ($eflags >> 7) & 0x1    # SF (符号标志)
(gdb) print ($eflags >> 11) & 0x1   # OF (溢出标志)
```

---

## 7. 实模式 vs 保护模式

### 7.1 切换架构

```gdb
# 实模式（16位）
(gdb) set architecture i8086

# 保护模式（32位）
(gdb) set architecture i386

# 自动检测
(gdb) set architecture auto
```

### 7.2 判断当前模式

```gdb
# 查看 CR0 寄存器的 PE 位（第0位）
(gdb) print $cr0 & 0x1
$1 = 0    # 实模式
$1 = 1    # 保护模式
```

### 7.3 地址计算差异

```gdb
# 实模式：物理地址 = 段寄存器 * 16 + 偏移
(gdb) print /x $cs*16+$eip

# 保护模式：物理地址 = 线性地址（平坦模型下）
(gdb) print /x $eip
```

---

## 8. 具体调试场景

### 场景 1：跟踪引导扇区执行

```gdb
# 1. 连接到 QEMU
target remote localhost:1234
set architecture i8086

# 2. 在引导扇区入口设置断点
break *0x7c00

# 3. 继续执行
continue

# 4. 单步跟踪引导过程
si
si
...

# 5. 查看当前指令
x/5i $pc

# 6. 查看即将加载 setup 的代码
# 通常在 0x7c00+ 的某处
x/20i $pc
```

### 场景 2：调试 E820 内存检测

```gdb
# 1. 在 setup.asm 的 detect_memory_e820 处设置断点
# 假设地址是 0x506
break *0x506

# 2. 运行到断点
continue

# 3. 单步跟踪 E820 调用
si                    # 进入函数
si                    # 设置寄存器
si                    # mov edx, 0x534D4150
si                    # mov eax, 0xE820
si                    # mov ecx, 20
si                    # int 0x15    <-- E820 调用

# 4. 查看 E820 返回结果
info registers eax ebx ecx edx
# EAX 应该 = 0x534D4150
# EBX = 下次调用的值（0 表示结束）
# CF = 0 表示成功

# 5. 查看写入的内存映射条目
x/5gx 0x1104          # 第一个条目

# 6. 继续跟踪循环
si
si
...

# 7. 查看最终的 count
x/1wx 0x1100          # 应该显示条目数量
```

### 场景 3：调试保护模式切换

```gdb
# 1. 在 A20 启用后设置断点
break *0x550    # 根据实际地址调整

# 2. 运行到断点
continue

# 3. 查看 GDT 加载
si                    # lgdt 指令

# 4. 在设置 CR0 处设置断点
break *0x560
continue

# 5. 单步执行 mov cr0, eax
si

# 6. 检查是否进入保护模式
print $cr0 & 0x1      # 应该输出 1

# 7. 切换到 32 位架构
set architecture i386

# 8. 查看 32 位寄存器
info registers eip cs

# 9. 继续跟踪远跳转
si                    # jmp CODE_SEG:protected_mode_start

# 10. 查看新的 CS 值
print /x $cs          # 应该是 0x8（代码段选择子）
```

### 场景 4：调试内核初始化

```gdb
# 1. 在内核入口设置断点
break *0x1200

# 2. 运行到断点
continue

# 3. 确保是 32 位模式
set architecture i386

# 4. 查看内核代码
x/20i $pc

# 5. 在 kernel_main 处设置断点（如果有符号）
break *kernel_main

# 6. 跟踪到内存检测代码
break *0x1300    # pmm_init 大致位置
continue

# 7. 查看内存映射表
x/10wx 0x1100
x/5gx 0x1104

# 8. 跟踪位图初始化
si
si
...

# 9. 查看位图区域
x/10wx 0x20000
```

### 场景 5：调试内存分配

```gdb
# 1. 在 pmm_alloc_page 设置断点
break *0x1400    # 根据实际地址调整

# 2. 运行到断点
continue

# 3. 查看位图状态
x/4wx 0x20000

# 4. 单步跟踪分配算法
si
si
...

# 5. 查看返回值（EAX）
print /x $eax     # 分配的物理地址

# 6. 查看更新后的位图
x/4wx 0x20000     # 应该看到某一位被置1
```

---

## 9. 常见问题

### Q1: GDB 连接失败

```
(gdb) target remote localhost:1234
Connection refused
```

**解决：**
- 确保 QEMU 已启动（`make debug`）
- 检查端口是否正确（默认 1234）
- 检查防火墙设置

### Q2: 断点设置失败

```
(gdb) break *0x7c00
Cannot access memory at address 0x7c00
```

**解决：**
- 确保在正确的架构模式下（i8086/i386）
- 确保地址是物理地址
- 检查地址是否在有效范围内

### Q3: 单步执行时跳过代码

**原因：** GDB 可能优化了某些指令

**解决：**
```gdb
# 使用 stepi 而不是 step
(gdb) stepi

# 或者设置调度锁
(gdb) set scheduler-locking on
```

### Q4: 实模式下地址显示不正确

**解决：**
```gdb
# 显式计算物理地址
(gdb) print /x ($cs << 4) + $eip

# 或者查看原始内存
(gdb) x/i ($cs << 4) + $eip
```

### Q5: 如何退出 GDB 但不终止 QEMU

```gdb
# 分离连接，QEMU 继续运行
(gdb) detach

# 然后退出 GDB
(gdb) quit
```

---

## 10. 实用脚本

### 10.1 GDB 初始化脚本 (.gdbinit)

创建 `~/.gdbinit` 或项目目录下的 `.gdbinit`：

```gdb
# 自动连接到 QEMU
target remote localhost:1234

# 设置架构
set architecture i8086

# 设置 Intel 语法
set disassembly-flavor intel

# 设置分页（避免长输出暂停）
set pagination off

# 设置确认（避免重复确认）
set confirm off

# 常用断点
break *0x7c00
break *0x500
break *0x1200

# 继续执行
continue
```

### 10.2 快速调试别名

添加到 `~/.bashrc`：

```bash
# QEMU 调试
alias os-debug='cd /home/zyj/Code/OS && make debug'

# GDB 连接
alias os-gdb='gdb -ex "target remote localhost:1234" -ex "set architecture i8086"'
```

---

## 总结

| 任务 | 命令 |
|------|------|
| 启动调试 | `make debug` + `gdb` |
| 连接 QEMU | `target remote localhost:1234` |
| 设置架构 | `set architecture i8086/i386` |
| 设置断点 | `break *0x地址` |
| 单步执行 | `si` / `ni` |
| 查看内存 | `x/10wx 0x地址` |
| 查看寄存器 | `info registers` / `print $eax` |
| 继续执行 | `continue` |

掌握这些命令和技巧，你就可以高效地调试操作系统的各个阶段了！
