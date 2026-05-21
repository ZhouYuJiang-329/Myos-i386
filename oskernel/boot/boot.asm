[ORG 0x7c00]

[SECTION .data]
BOOT_MAIN_ADDR EQU 0x500

[SECTION .text]
[BITS 16]
global _start
_start:
    ; 清除屏幕
    mov  ax,3
    int 0x10
    ; 打印 Hello World
    mov si, msg
    call print_string

    call read_hard_disk
    mov si,jump_setup
    call print_string

    ; 调转
    jmp BOOT_MAIN_ADDR
read_floppy_error:
    mov     si, read_floppy_error_msg
    call    print_string
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

read_hard_disk:
     ; 读硬盘第2扇区到 0x500
    mov     ch, 0       ; 0 柱面
    mov     dh, 0       ; 0 磁头
    mov     cl, 2       ; 2 扇区
    mov     bx, BOOT_MAIN_ADDR ;数据读到内存0x500
    
    mov     ah, 0x02    ; 读盘
    mov     al, 1       ; 读1个扇区
    mov     dl, 0x80    ; ★ 第一个硬盘 ★

    int     0x13
    ret

msg:
    db "Hello World boot!",10, 13, 0
jump_setup:
    db "jmp setup!", 10, 13,0
read_floppy_error_msg:
    db "read floppy error!", 10, 13, 0
; 填充到 510 字节
times 510-($-$$) db 0

; 引导扇区签名
dw 0xaa55
