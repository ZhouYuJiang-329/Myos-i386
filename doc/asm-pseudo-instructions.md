# NASM 汇编伪指令参考手册

> 本文档面向汇编语言初学者，介绍 NASM 中常用的伪指令及其使用方法。

---

## 目录

1. [什么是伪指令](#什么是伪指令)
2. [数据定义伪指令](#数据定义伪指令)
3. [预留空间伪指令](#预留空间伪指令)
4. [段定义伪指令](#段定义伪指令)
5. [符号和常量](#符号和常量)
6. [宏定义](#宏定义)
7. [包含和导入](#包含和导入)
8. [程序入口和地址](#程序入口和地址)
9. [常用规则和约定](#常用规则和约定)

---

## 什么是伪指令

**伪指令（Pseudo-instructions）** 是写给汇编器看的指令，不是 CPU 执行的机器指令。它们用于：
- 定义数据
- 分配内存
- 组织代码结构
- 控制汇编过程

---

## 数据定义伪指令

用于在内存中定义初始化的数据。

### DB - Define Byte（定义字节）
分配 1 字节（8 位）的存储空间。

```nasm
msg:    db 'Hello, World!'    ; 定义字符串
count:  db 100                ; 定义数值 100
flag:   db 0x01               ; 定义十六进制值
```

### DW - Define Word（定义字）
分配 2 字节（16 位）的存储空间。

```nasm
value:  dw 0x1234             ; 定义 16 位值
addr:   dw 0x07C0             ; 定义段地址
```

### DD - Define Doubleword（定义双字）
分配 4 字节（32 位）的存储空间。

```nasm
counter: dd 0x12345678        ; 定义 32 位值
```

### DQ - Define Quadword（定义四字）
分配 8 字节（64 位）的存储空间。

```nasm
bigval: dq 0x123456789ABCDEF0 ; 定义 64 位值
```

### 数据定义速查表

| 伪指令 | 大小 | 用途 |
|--------|------|------|
| `DB` | 1 字节 | 字符、小整数 |
| `DW` | 2 字节 | 16 位地址、整数 |
| `DD` | 4 字节 | 32 位地址、整数 |
| `DQ` | 8 字节 | 64 位整数 |

---

## 预留空间伪指令

用于预留未初始化的内存空间。

### RESB - Reserve Byte（预留字节）
```nasm
buffer: resb 512              ; 预留 512 字节的缓冲区
```

### RESW - Reserve Word（预留字）
```nasm
array:  resw 100              ; 预留 100 个字（200 字节）
```

### RESD - Reserve Doubleword（预留双字）
```nasm
table:  resd 50               ; 预留 50 个双字（200 字节）
```

### RESQ - Reserve Quadword（预留四字）
```nasm
large:  resq 10               ; 预留 10 个四字（80 字节）
```

**重要区别**：
- `DB/DW/DD/DQ` - 定义**已初始化**的数据，占用文件空间
- `RESB/RESW/RESD/RESQ` - 预留**未初始化**的空间，不占用文件空间（仅在运行时占用内存）

---

## 段定义伪指令

用于组织代码和数据到不同的段。

### SECTION / SEGMENT
```nasm
section .data                 ; 数据段
    msg: db 'Hello'

section .bss                  ; 未初始化数据段
    buffer: resb 1024

section .text                 ; 代码段
    global _start
_start:
    ; 代码在这里
```

### 标准段名称

| 段名 | 用途 | 伪指令 |
|------|------|--------|
| `.text` | 代码段 | `DB`, `DW` 等 |
| `.data` | 已初始化数据 | `DB`, `DW` 等 |
| `.bss` | 未初始化数据 | `RESB`, `RESW` 等 |
| `.rodata` | 只读数据 | `DB`, `DW` 等 |

---

## 符号和常量

### EQU - 定义常量
```nasm
SECTOR_SIZE equ 512           ; 定义常量
MAX_FILES   equ 100

; 使用
mov ax, SECTOR_SIZE           ; 编译时替换为 mov ax, 512
```

**特点**：
- `EQU` 定义的常量不占用内存
- 编译时直接替换为值
- 不能被修改

### TIMES - 重复数据
```nasm
times 512 db 0                ; 重复 512 个 0
times 10 dw 0x1234            ; 重复 10 次 0x1234

; 常用于填充引导扇区
times 510-($-$$) db 0         ; 填充到 510 字节
```

### 特殊符号

| 符号 | 含义 |
|------|------|
| `$` | 当前地址（当前行的位置） |
| `$$` | 当前段的起始地址 |
| `$-$$` | 当前段已使用的字节数 |

**示例**：
```nasm
section .text
    jmp short start
    nop
start:
    ; $ 表示当前地址
    ; $$ 表示 .text 段的起始地址
    
; 计算当前段大小
current_size equ $ - $$
```

---

## 宏定义

### %macro / %endmacro
用于定义可重复使用的代码块。

```nasm
; 定义宏：打印字符
%macro print_char 1           ; 1 表示参数个数
    mov ah, 0x0E
    mov al, %1                ; %1 表示第一个参数
    int 0x10
%endmacro

; 使用宏
print_char 'A'                ; 输出 A
print_char 'B'                ; 输出 B
```

### 多参数宏
```nasm
; 定义宏：设置光标位置
%macro set_cursor 2           ; 2 个参数
    mov ah, 0x02
    mov bh, 0                 ; 页号
    mov dh, %1                ; 行
    mov dl, %2                ; 列
    int 0x10
%endmacro

; 使用
set_cursor 10, 20             ; 设置光标到第 10 行 20 列
```

---

## 包含和导入

### %include - 包含文件
```nasm
%include "utils.asm"          ; 包含其他汇编文件
%include "macros.inc"         ; 包含宏定义文件
```

**作用**：将指定文件的内容插入到当前位置。

### GLOBAL - 导出符号
```nasm
global _start                 ; 声明 _start 为全局符号
global print_string           ; 导出函数
```

**作用**：让其他文件可以访问这些符号。

### EXTERN - 导入外部符号
```nasm
extern print_string           ; 声明 print_string 在其他文件中定义
extern kernel_main            ; 导入 C 语言编写的函数
```

**作用**：使用在其他文件中定义的符号。

---

## 程序入口和地址

### ORG - 设置起始地址
```nasm
org 0x7C00                    ; 代码从地址 0x7C00 开始

start:
    jmp 0x0000:main           ; 跳转到 main

main:
    ; 代码在这里
```

**重要**：
- `ORG` 告诉汇编器代码将被加载到哪个内存地址
- BIOS 加载引导扇区到 `0x7C00`
- 没有 `ORG`，地址计算会出错

### BITS - 设置操作模式
```nasm
bits 16                       ; 16 位实模式（默认）
    mov ax, bx

bits 32                       ; 32 位保护模式
    mov eax, ebx

bits 64                       ; 64 位长模式
    mov rax, rbx
```

---

## 常用规则和约定

### 命名规范

| 类型 | 命名方式 | 示例 |
|------|----------|------|
| 常量 | 全大写 | `SECTOR_SIZE`, `MAX_FILES` |
| 标签 | 小写/下划线 | `start`, `print_string` |
| 局部标签 | 以点开头 | `.loop`, `.done` |
| 宏 | 小写/下划线 | `print_char`, `set_cursor` |

### 代码格式建议

```nasm
; 1. 注释说明段用途
section .data
    ; 2. 数据定义对齐
    msg:        db 'Hello', 0
    count:      dw 100
    
section .text
    ; 3. 全局声明放在前面
    global _start
    
_start:
    ; 4. 代码缩进
    mov ax, 0x07C0
    mov ds, ax
    
    ; 5. 局部标签用于循环和分支
.loop:
    dec cx
    jnz .loop
    
.done:
    ret
```

### 注释风格

```nasm
; 单行注释

;--------------------------------------------------
; 函数：print_string
; 参数：DS:SI = 字符串地址
; 返回：无
; 说明：使用 BIOS int 0x10 输出字符串
;--------------------------------------------------
print_string:
    pusha
.next_char:
    lodsb                       ; 加载下一个字符到 AL
    test al, al                 ; 检查是否为结束符
    jz .done                    ; 是则结束
    
    mov ah, 0x0E                ; BIOS  teletype 功能
    int 0x10                    ; 调用 BIOS 中断
    jmp .next_char
.done:
    popa
    ret
```

### 引导扇区签名

```nasm
; 引导扇区最后两个字节必须是 0xAA55
times 510-($-$$) db 0         ; 填充到 510 字节
dw 0xAA55                     ; 引导签名
```

---

## 完整示例

```nasm
; boot.asm - 引导扇区示例
;--------------------------------------------------

bits 16
org 0x7C00

; 常量定义
SECTOR_SIZE     equ 512
BOOT_MSG        equ 'Booting...'

;--------------------------------------------------
section .data
    msg: db 'Hello, OS World!', 0x0D, 0x0A, 0

;--------------------------------------------------
section .text
    global _start

_start:
    ; 设置段寄存器
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00              ; 设置栈顶
    
    ; 打印消息
    mov si, msg
    call print_string
    
    ; 无限循环
    jmp $

;--------------------------------------------------
; 函数：print_string
; 参数：DS:SI = 字符串地址
;--------------------------------------------------
print_string:
    pusha
.next_char:
    lodsb
    test al, al
    jz .done
    
    mov ah, 0x0E
    int 0x10
    jmp .next_char
.done:
    popa
    ret

;--------------------------------------------------
; 填充和签名
;--------------------------------------------------
times 510-($-$$) db 0
dw 0xAA55
```

---

## 总结

| 类别 | 常用伪指令 |
|------|-----------|
| 数据定义 | `DB`, `DW`, `DD`, `DQ` |
| 预留空间 | `RESB`, `RESW`, `RESD`, `RESQ` |
| 段定义 | `SECTION`, `SEGMENT` |
| 常量 | `EQU`, `TIMES` |
| 宏 | `%macro`, `%endmacro` |
| 包含 | `%include`, `GLOBAL`, `EXTERN` |
| 地址 | `ORG`, `BITS` |

---

> **提示**：汇编编程需要精确控制每一个细节。建议多实践，从简单的程序开始，逐步理解每个伪指令的作用。
