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