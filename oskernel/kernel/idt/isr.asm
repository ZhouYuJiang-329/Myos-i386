; ============================================
; ISR (Interrupt Service Routine) 汇编入口
; ============================================
; 这个文件定义了中断服务程序的汇编入口点
; 每个中断向量都有一个对应的入口函数

[BITS 32]
[SECTION .text]

; ============================================
; 外部函数声明
; ============================================
; interrupt_handler 定义在 interrupt.c 中
extern interrupt_handler

; ============================================
; 通用中断处理宏
; ============================================
; 这个宏用于创建没有错误码的中断处理程序
; CPU 不会自动压入错误码，所以需要手动压入一个伪错误码 (0)

%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    ; 压入伪错误码（CPU 没有自动压入）
    push dword 0
    ; 压入中断向量号
    push dword %1
    ; 跳转到通用中断处理程序
    jmp isr_common_stub
%endmacro

; 这个宏用于创建有错误码的中断处理程序
; CPU 会自动压入错误码，所以不需要手动压入

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    ; 错误码已经被 CPU 压入栈中，不需要再压入
    ; 只需要压入中断向量号
    push dword %1
    ; 跳转到通用中断处理程序
    jmp isr_common_stub
%endmacro

; ============================================
; 通用 IRQ 处理宏
; ============================================
; 这个宏用于创建硬件中断处理程序 (IRQ 0-15)

%macro IRQ 2
global irq%1
irq%1:
    ; 压入伪错误码（硬件中断没有错误码）
    push dword 0
    ; 压入中断向量号（IRQ + 32 = ISR 向量号）
    push dword %2
    ; 跳转到通用中断处理程序
    jmp irq_common_stub
%endmacro

; ============================================
; CPU 异常处理程序 (ISR 0-31)
; ============================================

; ISR 0: 除法错误 (#DE) - 没有错误码
ISR_NOERRCODE 0
; ISR 1: 调试异常 (#DB) - 没有错误码
ISR_NOERRCODE 1
; ISR 2: NMI 中断 - 没有错误码
ISR_NOERRCODE 2
; ISR 3: 断点 (#BP) - 没有错误码
ISR_NOERRCODE 3
; ISR 4: 溢出 (#OF) - 没有错误码
ISR_NOERRCODE 4
; ISR 5: 越界 (#BR) - 没有错误码
ISR_NOERRCODE 5
; ISR 6: 无效操作码 (#UD) - 没有错误码
ISR_NOERRCODE 6
; ISR 7: 设备不可用 (#NM) - 没有错误码
ISR_NOERRCODE 7
; ISR 8: 双重故障 (#DF) - 有错误码
ISR_ERRCODE 8
; ISR 9: 协处理器段溢出 - 没有错误码（保留）
ISR_NOERRCODE 9
; ISR 10: 无效 TSS (#TS) - 有错误码
ISR_ERRCODE 10
; ISR 11: 段不存在 (#NP) - 有错误码
ISR_ERRCODE 11
; ISR 12: 栈段错误 (#SS) - 有错误码
ISR_ERRCODE 12
; ISR 13: 一般保护错误 (#GP) - 有错误码
ISR_ERRCODE 13
; ISR 14: 页错误 (#PF) - 有错误码
ISR_ERRCODE 14
; ISR 15: 保留 - 没有错误码
ISR_NOERRCODE 15
; ISR 16: x87 浮点错误 (#MF) - 没有错误码
ISR_NOERRCODE 16
; ISR 17: 对齐检查 (#AC) - 有错误码
ISR_ERRCODE 17
; ISR 18: 机器检查 (#MC) - 没有错误码
ISR_NOERRCODE 18
; ISR 19: SIMD 流程异常 (#XM) - 没有错误码
ISR_NOERRCODE 19
; ISR 20-31: 保留 - 没有错误码
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; ============================================
; 硬件中断处理程序 (IRQ 0-15)
; ============================================
; IRQ 0-7: 主 PIC (映射到 ISR 32-39)
; IRQ 8-15: 从 PIC (映射到 ISR 40-47)

; IRQ 0: 定时器 -> ISR 32
IRQ 0, 32
; IRQ 1: 键盘 -> ISR 33
IRQ 1, 33
; IRQ 2: 级联 -> ISR 34
IRQ 2, 34
; IRQ 3: COM2 -> ISR 35
IRQ 3, 35
; IRQ 4: COM1 -> ISR 36
IRQ 4, 36
; IRQ 5: LPT2 -> ISR 37
IRQ 5, 37
; IRQ 6: 软盘 -> ISR 38
IRQ 6, 38
; IRQ 7: LPT1/并口 -> ISR 39
IRQ 7, 39
; IRQ 8: RTC 时钟 -> ISR 40
IRQ 8, 40
; IRQ 9: ACPI -> ISR 41
IRQ 9, 41
; IRQ 10: 开放 -> ISR 42
IRQ 10, 42
; IRQ 11: 开放 -> ISR 43
IRQ 11, 43
; IRQ 12: PS/2 鼠标 -> ISR 44
IRQ 12, 44
; IRQ 13: FPU -> ISR 45
IRQ 13, 45
; IRQ 14: 主 ATA 硬盘 -> ISR 46
IRQ 14, 46
; IRQ 15: 从 ATA 硬盘 -> ISR 47
IRQ 15, 47

; ============================================
; 通用中断处理程序（汇编部分）
; ============================================
; 这个函数保存所有寄存器，调用 C 语言中断处理函数，然后恢复寄存器

isr_common_stub:
    ; 保存所有通用寄存器
    ; 注意：中断发生时，CPU 已经压入了 EIP, CS, EFLAGS, (ESP, SS)
    ; 我们需要保存其他通用寄存器以便恢复

    push eax        ; 保存 EAX
    push ecx        ; 保存 ECX
    push edx        ; 保存 EDX
    push ebx        ; 保存 EBX
    push ebp        ; 保存 EBP
    push esi        ; 保存 ESI
    push edi        ; 保存 EDI

    ; 保存数据段寄存器
    push ds
    push es
    push fs
    push gs

    ; 设置内核数据段
    ; 中断可能发生在用户模式，我们需要切换到内核数据段
    mov ax, 0x10    ; 内核数据段选择子
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; 调用 C 语言中断处理函数
    ; cdecl 调用约定：参数从右到左压栈
    ; void interrupt_handler(struct interrupt_frame* frame, uint32_t interrupt_number)
    ;   1. 先压 interrupt_number (右参数)
    ;   2. 再压 frame (左参数)
    ;
    ; 栈布局（push gs 后，push esp 前）：
    ;   [esp+0]  gs        [esp+36] ecx      [esp+44] int_number
    ;   [esp+4]  fs        [esp+40] eax      [esp+48] error_code
    ;   [esp+8]  es        ...               [esp+52] eip
    ;   [esp+12] ds                         [esp+56] cs
    ;   [esp+16] edi                        [esp+60] eflags
    ;   [esp+20] esi
    ;   [esp+24] ebp
    ;   [esp+28] ebx
    ;   [esp+32] edx
    push dword [esp + 44]  ; interrupt_number (右参数，先压栈)
    push esp               ; frame 指针 (左参数，后压栈)
    call interrupt_handler
    add esp, 8             ; 清理参数（两个 4 字节）

    ; 恢复数据段寄存器
    pop gs
    pop fs
    pop es
    pop ds

    ; 恢复所有通用寄存器
    pop edi
    pop esi
    pop ebp
    pop ebx
    pop edx
    pop ecx
    pop eax

    ; 清理栈上的伪错误码和中断向量号
    add esp, 8      ; 跳过中断向量号和错误码

    ; 返回（中断返回）
    iret

; ============================================
; IRQ 通用处理程序
; ============================================
; 硬件中断需要额外的处理：
; 1. 发送 EOI (End of Interrupt) 给 PIC
; 2. 清除中断控制器的中断标志

irq_common_stub:
    ; 保存所有通用寄存器
    push eax
    push ecx
    push edx
    push ebx
    push ebp
    push esi
    push edi

    ; 保存数据段寄存器
    push ds
    push es
    push fs
    push gs

    ; 设置内核数据段
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; 调用 C 语言中断处理函数
    ; cdecl 调用约定：参数从右到左压栈
    push dword [esp + 44]  ; interrupt_number (右参数，先压栈)
    push esp               ; frame 指针 (左参数，后压栈)
    call interrupt_handler
    add esp, 8             ; 清理参数

    ; 恢复数据段寄存器
    pop gs
    pop fs
    pop es
    pop ds

    ; 恢复所有通用寄存器
    pop edi
    pop esi
    pop ebp
    pop ebx
    pop edx
    pop ecx
    pop eax

    ; 清理栈上的伪错误码和中断向量号
    add esp, 8

    ; 返回
    iret
