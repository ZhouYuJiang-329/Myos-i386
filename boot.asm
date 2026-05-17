[ORG 0x7c00]

[SECTION .text]
[BITS 16]
global _start
_start:
    ; 设置段寄存器
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00

    ; 打印 Hello World
    mov si, msg
    call print_string

    ; 无限循环
    jmp $

; 打印字符串函数
; 输入: DS:SI 指向字符串
print_string:
    pusha
.loop:
    lodsb           ; 从 DS:SI 加载一个字节到 AL, SI 自增
    test al, al     ; 检查是否为字符串结束符 (0)
    jz .done
    mov ah, 0x0e    ; BIOS 中断 0x10 功能号: 电传打字机输出
    mov bh, 0x00    ; 页号
    mov bl, 0x07    ; 颜色属性 (浅灰色)
    int 0x10        ; 调用 BIOS 中断
    jmp .loop
.done:
    popa
    ret

msg:
    db "Hello World", 0

; 填充到 510 字节
times 510-($-$$) db 0

; 引导扇区签名
dw 0xaa55
