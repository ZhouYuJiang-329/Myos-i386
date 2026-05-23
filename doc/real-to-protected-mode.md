# 从实模式跳转到保护模式详解

## 概述

本文档详细说明 x86 架构下从实模式（Real Mode）跳转到保护模式（Protected Mode）的完整流程。这是操作系统启动过程中的关键步骤。

---

## 一、前置知识

### 1.1 实模式 vs 保护模式

| 特性 | 实模式 (Real Mode) | 保护模式 (Protected Mode) |
|------|-------------------|--------------------------|
| 地址空间 | 1MB (20位地址线) | 4GB (32位地址线) |
| 内存保护 | 无 | 有（段限、特权级） |
| 多任务 | 不支持 | 支持 |
| 段寄存器用法 | 段地址 × 16 + 偏移 | 段选择子指向描述符 |
| 默认操作数大小 | 16位 | 32位 |

### 1.2 关键数据结构

#### 段描述符（Segment Descriptor）
段描述符是 GDT/LDT 中的条目，共 8 字节：

```
 31                   16 15                    0
+----------------------+----------------------+
|  Base Address (31-24) |  Flags + Limit (19-16) |
+----------------------+----------------------+
|  Base Address (23-16) |  Base Address (15-0)   |
+----------------------+----------------------+
|  Base Address (15-0)  |  Limit (15-0)          |
+----------------------+----------------------+
```

**字段说明：**
- **Base Address (32位)**: 段的起始物理地址
- **Limit (20位)**: 段的大小限制
- **Flags (12位)**: 包含 G、D/B、L、AVL、P、DPL、S、Type 等标志

#### 段选择子（Segment Selector）
段选择子是加载到段寄存器（CS、DS、SS 等）中的 16 位值：

```
 15                        3  2   0
+---------------------------+---+---+
|      Index (13位)          |TI |RPL|
+---------------------------+---+---+
```

- **Index**: GDT/LDT 中的描述符索引
- **TI (Table Indicator)**: 0=GDT, 1=LDT
- **RPL (Requested Privilege Level)**: 请求特权级

---

## 二、切换步骤详解

### Step 1: 准备 GDT（全局描述符表）

GDT 是保护模式下段描述符的数组，必须至少包含：
1. **空描述符**（索引 0）：必须保留，不能被使用
2. **代码段描述符**：用于执行代码
3. **数据段描述符**：用于数据访问

#### 1.1 GDT 结构定义

```nasm
; GDT 定义
; 每个描述符 8 字节

gdt_start:
    ; 空描述符（必须）
    dd 0x00000000       ; 低 4 字节
    dd 0x00000000       ; 高 4 字节

gdt_code:               ; 代码段描述符
    ; Base=0x00000000, Limit=0xFFFFF
    ; Type=Execute/Read, DPL=0, Present=1
    ; G=1 (4KB粒度), D=1 (32位操作数)
    dw 0xFFFF           ; Limit (15-0)
    dw 0x0000           ; Base (15-0)
    db 0x00             ; Base (23-16)
    db 10011010b        ; P=1, DPL=00, S=1, Type=1010 (可执行/可读)
    db 11001111b        ; G=1, D=1, L=0, AVL=0, Limit(19-16)=1111
    db 0x00             ; Base (31-24)

gdt_data:               ; 数据段描述符
    ; Base=0x00000000, Limit=0xFFFFF
    ; Type=Read/Write, DPL=0, Present=1
    dw 0xFFFF           ; Limit (15-0)
    dw 0x0000           ; Base (15-0)
    db 0x00             ; Base (23-16)
    db 10010010b        ; P=1, DPL=00, S=1, Type=0010 (可读/可写)
    db 11001111b        ; G=1, D=1, L=0, AVL=0, Limit(19-16)=1111
    db 0x00             ; Base (31-24)

gdt_end:

; GDT 描述符（用于 lgdt 指令）
gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT 大小（界限）
    dd gdt_start                ; GDT 基址
```

#### 1.2 段选择子定义

```nasm
; 段选择子（Index << 3 | TI | RPL）
CODE_SEG equ gdt_code - gdt_start  ; 0x08 (索引 1)
DATA_SEG equ gdt_data - gdt_start  ; 0x10 (索引 2)
```

---

### Step 2: 禁用中断

在切换模式前必须禁用中断，防止切换过程中被中断：

```nasm
    cli                 ; 清除中断标志，禁用可屏蔽中断
```

**注意：** 如果之前启用了 NMI（不可屏蔽中断），也需要处理。但通常在 bootloader 阶段 NMI 是禁用的。

---

### Step 3: 启用 A20 地址线

#### 3.1 为什么需要 A20

8086 有 20 位地址线，可寻址 1MB。当地址超过 0xFFFFF 时会回绕到 0x00000。
80286+ 有 24/32 位地址线，但为了兼容旧程序，IBM 在 PC/AT 中添加了 A20 门控，默认禁用第 21 根地址线（A20）。

**启用 A20 后才能访问超过 1MB 的内存。**

#### 3.2 通过键盘控制器启用 A20

```nasm
; 等待键盘控制器就绪
wait_keyboard:
    in al, 0x64         ; 读取状态寄存器
    test al, 0x02       ; 检查输入缓冲区是否已满（bit 1）
    jnz wait_keyboard   ; 如果满，继续等待
    ret

; 启用 A20
enable_a20:
    call wait_keyboard
    mov al, 0xD1        ; 命令：写入输出端口
    out 0x64, al

    call wait_keyboard
    mov al, 0xDF        ; 数据：启用 A20
    out 0x60, al

    call wait_keyboard
    ret
```

#### 3.3 快速 A20 方法（如果支持）

```nasm
; 快速 A20 启用（通过系统控制端口 A）
    in al, 0x92         ; 读取系统控制端口 A
    or al, 0x02         ; 设置 A20 启用位
    out 0x92, al
```

---

### Step 4: 加载 GDT 并进入保护模式

#### 4.1 加载 GDT

```nasm
    lgdt [gdt_descriptor]   ; 加载 GDT 到 GDTR 寄存器
```

#### 4.2 设置 CR0 的 PE 位

```nasm
    mov eax, cr0
    or eax, 0x01        ; 设置 PE (Protection Enable) 位
    mov cr0, eax
```

#### 4.3 远跳转刷新流水线

```nasm
    ; 远跳转格式: jmp 段选择子:偏移地址
    jmp CODE_SEG:protected_mode_start
```

**为什么需要远跳转？**
1. 刷新 CPU 流水线，确保后续指令以 32 位模式解码
2. 加载 CS 寄存器为代码段选择子
3. 跳转到保护模式代码入口

---

### Step 5: 初始化保护模式环境

#### 5.1 设置段寄存器

```nasm
[bits 32]               ; 告诉汇编器后续是 32 位代码

protected_mode_start:
    ; 设置数据段寄存器
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
```

#### 5.2 设置栈指针

```nasm
    ; 设置栈顶（根据实际内存布局调整）
    mov esp, 0x90000    ; 设置 ESP 指向可用内存区域
```

#### 5.3 重新启用中断（可选）

```nasm
    ; 如果需要，可以重新启用中断
    ; 但必须先设置好 IDT（中断描述符表）
    ; sti
```

---

## 三、完整代码示例

```nasm
; setup.asm - 从实模式切换到保护模式

[bits 16]               ; 16 位实模式代码
[org 0x7c00]            ; BIOS 加载 bootloader 到 0x7c00

start:
    ; 设置段寄存器
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00      ; 设置栈

    ; Step 1: 禁用中断
    cli

    ; Step 2: 启用 A20
    call enable_a20

    ; Step 3: 加载 GDT
    lgdt [gdt_descriptor]

    ; Step 4: 进入保护模式
    mov eax, cr0
    or eax, 0x01
    mov cr0, eax

    ; Step 5: 远跳转到 32 位代码
    jmp CODE_SEG:protected_mode

; ============================================
; A20 启用函数
; ============================================
enable_a20:
    call wait_keyboard
    mov al, 0xD1
    out 0x64, al
    call wait_keyboard
    mov al, 0xDF
    out 0x60, al
    call wait_keyboard
    ret

wait_keyboard:
    in al, 0x64
    test al, 0x02
    jnz wait_keyboard
    ret

; ============================================
; GDT 定义
; ============================================
gdt_start:
    dd 0x00000000       ; 空描述符
    dd 0x00000000

gdt_code:
    dw 0xFFFF           ; Limit 0-15
    dw 0x0000           ; Base 0-15
    db 0x00             ; Base 16-23
    db 10011010b        ; Access: P=1, DPL=0, S=1, Type=1010
    db 11001111b        ; Flags: G=1, D=1, Limit 16-19
    db 0x00             ; Base 24-31

gdt_data:
    dw 0xFFFF           ; Limit 0-15
    dw 0x0000           ; Base 0-15
    db 0x00             ; Base 16-23
    db 10010010b        ; Access: P=1, DPL=0, S=1, Type=0010
    db 11001111b        ; Flags: G=1, D=1, Limit 16-19
    db 0x00             ; Base 24-31
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; ============================================
; 32 位保护模式代码
; ============================================
[bits 32]

protected_mode:
    ; 初始化段寄存器
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; 设置栈
    mov esp, 0x90000

    ; 在屏幕左上角显示 "PM" 表示成功进入保护模式
    mov byte [0xB8000], 'P'
    mov byte [0xB8001], 0x0F
    mov byte [0xB8002], 'M'
    mov byte [0xB8003], 0x0F

    ; 无限循环
    jmp $

; 填充到 512 字节并添加引导签名
times 510-($-$$) db 0
dw 0xAA55
```

---

## 四、常见错误与调试

### 4.1 三重故障（Triple Fault）

**现象：** CPU 立即重启

**原因：**
- GDT 配置错误（如空描述符被使用）
- 段选择子指向无效的描述符
- 代码段没有执行权限
- 远跳转后代码位置错误

**调试方法：**
1. 使用 Bochs 调试器单步执行
2. 检查 GDT 内容是否正确加载
3. 验证段选择子计算是否正确

### 4.2 A20 未启用

**现象：** 只能访问奇数兆字节内存（1MB-2MB 映射到 0-1MB）

**检测方法：**
```nasm
; 检查 A20 是否启用
    mov ax, 0xFFFF
    mov es, ax
    mov word [es:0x7E0E], 0x1234    ; 写入 0xFFFF:0x7E0E (物理地址 0x107E0E)
    cmp word [0x7E0E], 0x1234       ; 检查 0x0000:0x7E0E (物理地址 0x07E0E)
    je a20_disabled                 ; 如果相等，说明 A20 未启用
```

### 4.3 段寄存器未更新

**现象：** 进入保护模式后访问内存错误

**解决：** 确保所有段寄存器（DS, ES, FS, GS, SS）都被重新加载

---

## 五、参考资源

- Intel 64 and IA-32 Architectures Software Developer's Manual
- AMD64 Architecture Programmer's Manual
- OSDev Wiki: https://wiki.osdev.org/Protected_Mode

---

## 六、总结检查清单

进入保护模式前确认：

- [ ] GDT 已正确定义（包含空描述符、代码段、数据段）
- [ ] 段描述符的 Base/Limit/Flags 设置正确
- [ ] 中断已禁用（cli）
- [ ] A20 地址线已启用
- [ ] GDT 已加载（lgdt）
- [ ] CR0.PE 位已设置
- [ ] 远跳转使用正确的段选择子
- [ ] 32 位代码中重新加载了所有段寄存器
- [ ] 栈指针已设置
