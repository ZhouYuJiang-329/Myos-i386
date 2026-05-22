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
    mov si,read_disking
    call print_string
   ; 验证状态
    ; 3 0表示硬盘未准备好与主机交换数据 1表示准备好了
    ; 7 0表示硬盘不忙 1表示硬盘忙
    ; 0 0表示前一条指令正常执行 1表示执行出错 出错信息通过0x1f1端口获得
.read_check:
    mov dx, 0x1f7
    in al, dx
    and al, 0b10001000  ; 取硬盘状态的第3、7位
    cmp al, 0b00001000  ; 硬盘数据准备好了且不忙了
    jnz .read_check

    ; 读数据
    mov dx, 0x1f0
    mov cx, 256
    mov edi, BOOT_MAIN_ADDR
.read_data:
    in ax, dx
    mov [edi], ax
    add edi, 2
    loop .read_data

    ; 跳过去
    mov     si, jump_setup
    call    print_string

    jmp     BOOT_MAIN_ADDR  

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
    mov ecx, 2  ; 从硬盘哪个扇区开始读
    mov bl, 2   ; 读取的扇区数量

    ; 0x1f2 8bit 指定读取或写入的扇区数
    mov dx, 0x1f2
    mov al, bl
    out dx, al

    ; 0x1f3 8bit iba地址的第八位 0-7
    inc dx
    mov al, cl
    out dx, al

    ; 0x1f4 8bit iba地址的中八位 8-15
    inc dx
    mov al, ch      ; 取中8位
    out dx, al

    ; 0x1f5 8bit iba地址的高八位 16-23
    inc dx
    shr ecx, 16
    mov al, cl
    out dx, al

    ; 0x1f6 8bit
    ; 0-3 位iba地址的24-27
    ; 4 0表示主盘 1表示从盘
    ; 5、7位固定为1
    ; 6 0表示CHS模式，1表示LAB模式
    inc dx
    mov al, ch
    and al, 0b1110_1111
    out dx, al

    ; 0x1f7 8bit  命令或状态端口
    inc dx
    mov al, 0x20
    out dx, al

    ret

msg:
    db "Hello World boot!",10, 13, 0
jump_setup:
    db "jmp setup!", 10, 13,0
read_floppy_error_msg:
    db "read floppy error!", 10, 13, 0
read_disking:
    db "read disk ing...",13,10,0
; 填充到 510 字节
times 510-($-$$) db 0

; 引导扇区签名
dw 0xaa55
