[ORG 0x500]

[Section .text]
[BITS 16]
global _start
_start:
    mov     ax, 0
    mov     ss, ax
    mov     ds, ax
    mov     es, ax
    mov     fs, ax
    mov     gs, ax
    mov     si, ax

    mov     si, msg
    call    print

    ; ============================================
    ; Step 2: 禁用中断
    ; ============================================
    ; 在切换到保护模式前必须禁用中断，原因：
    ; 1. 保护模式使用不同的中断处理机制（IDT 而非 IVT）
    ; 2. 切换过程中如果发生中断，CPU 会在错误的环境中执行中断处理程序
    ; 3. 实模式下的中断向量表（IVT）在保护模式下无效
    cli                 ; Clear Interrupt Flag，禁用可屏蔽中断

    ; ============================================
    ; Step 3: 启用 A20 地址线
    ; ============================================
    ; 历史背景：8086 只有 20 根地址线，可寻址 1MB
    ; 当地址超过 0xFFFFF 时会回绕到 0x00000（模 1MB）
    ; 80286+ 有 24/32 根地址线，但为了兼容，IBM 添加了 A20 门控
    ; 默认情况下 A20 被禁用，导致只能访问奇数兆字节内存
    ; 启用 A20 后才能访问超过 1MB 的内存
    call    enable_a20

    jmp     $

; 如何调用
; mov     si, msg   ; 1 传入字符串
; call    print     ; 2 调用
print:
    mov ah, 0x0e
    mov bh, 0
    mov bl, 0x01
.loop:
    mov al, [si]
    cmp al, 0
    jz .done
    int 0x10

    inc si
    jmp .loop
.done:
    ret

msg:
    db "hello setup!", 10, 13, 0

; ============================================
; Step 1: GDT（全局描述符表）定义
; ============================================

; GDT 起始标记
gdt_start:
    ; 0 号描述符：空描述符（必须保留，不能使用）
    dd 0x00000000       ; 低 4 字节：Limit(15-0) + Base(15-0) 的一部分
    dd 0x00000000       ; 高 4 字节：Base + Flags

; 代码段描述符（1 号描述符）
; 属性：基址=0x00000000，界限=0xFFFFF（4GB），粒度=4KB
;       可执行、可读、特权级=0（内核）、存在位=1
gdt_code:
    dw 0xFFFF           ; Limit (15-0): 段界限低 16 位 = 0xFFFF
    dw 0x0000           ; Base (15-0): 基址低 16 位 = 0x0000
    db 0x00             ; Base (23-16): 基址中间 8 位 = 0x00
    db 10011010b        ; Access Byte:
                        ;   P=1    (段存在)
                        ;   DPL=00 (特权级 0 - 内核)
                        ;   S=1    (代码/数据段)
                        ;   Type=1010 (可执行、一致、可读)
    db 11001111b        ; Flags + Limit(19-16):
                        ;   G=1    (粒度=4KB，界限=0xFFFFF * 4KB = 4GB)
                        ;   D=1    (默认操作数大小=32位)
                        ;   L=0    (非64位代码段)
                        ;   AVL=0  (保留)
                        ;   Limit(19-16)=1111
    db 0x00             ; Base (31-24): 基址高 8 位 = 0x00

; 数据段描述符（2 号描述符）
; 属性：基址=0x00000000，界限=0xFFFFF（4GB），粒度=4KB
;       可读写、特权级=0（内核）、存在位=1
gdt_data:
    dw 0xFFFF           ; Limit (15-0): 段界限低 16 位 = 0xFFFF
    dw 0x0000           ; Base (15-0): 基址低 16 位 = 0x0000
    db 0x00             ; Base (23-16): 基址中间 8 位 = 0x00
    db 10010010b        ; Access Byte:
                        ;   P=1    (段存在)
                        ;   DPL=00 (特权级 0 - 内核)
                        ;   S=1    (代码/数据段)
                        ;   Type=0010 (数据段、向上扩展、可写)
    db 11001111b        ; Flags + Limit(19-16): 同代码段
    db 0x00             ; Base (31-24): 基址高 8 位 = 0x00

; GDT 结束标记
gdt_end:

; ============================================
; GDT 描述符（用于 lgdt 指令）
; ============================================
; lgdt 指令需要一个 6 字节的内存结构：
;   - 前 2 字节：GDT 界限（大小 - 1）
;   - 后 4 字节：GDT 基址
gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT 界限 = 总大小 - 1
    dd gdt_start                ; GDT 基址（32位物理地址）

; ============================================
; 段选择子（Segment Selector）定义
; ============================================
; 段选择子格式：Index(13位) | TI(1位) | RPL(2位)
;   - Index: GDT 中的描述符索引
;   - TI: 0=GDT, 1=LDT
;   - RPL: 请求特权级

CODE_SEG equ gdt_code - gdt_start  ; 代码段选择子 = 0x08 (索引 1, TI=0, RPL=0)
DATA_SEG equ gdt_data - gdt_start  ; 数据段选择子 = 0x10 (索引 2, TI=0, RPL=0)

; ============================================
; Step 3: A20 启用函数
; ============================================
; 通过键盘控制器启用 A20 地址线
; 键盘控制器（8042）的端口：
;   - 0x64: 命令/状态端口（只写/只读）
;   - 0x60: 数据端口（读写）

; 等待键盘控制器输入缓冲区为空
; 输入缓冲区满时，bit 1 (0x02) 被置位
wait_keyboard:
    in      al, 0x64            ; 读取状态寄存器
    test    al, 0x02            ; 检查 bit 1（输入缓冲区状态）
    jnz     wait_keyboard       ; 如果缓冲区满，继续等待
    ret

; 启用 A20 地址线
enable_a20:
    ; 第一步：发送命令 0xD1（写入输出端口）
    call    wait_keyboard       ; 等待键盘控制器就绪
    mov     al, 0xD1            ; 命令：写入输出端口 P2
    out     0x64, al            ; 发送到命令端口

    ; 第二步：发送数据 0xDF（启用 A20）
    call    wait_keyboard       ; 等待键盘控制器就绪
    mov     al, 0xDF            ; 数据：设置 P2 端口值，bit 1 启用 A20
    out     0x60, al            ; 发送到数据端口

    ; 第三步：等待操作完成
    call    wait_keyboard       ; 等待命令执行完成
    ret