[SECTION .text]
[BITS 32]
extern kernel_main
extern paging_init

global _start

_start:
    ; ============================================
    ; 在启用分页之前，设置临时页目录和页表
    ; ============================================

    ; 清零页目录 (4KB)
    xor eax, eax
    mov edi, 0x50000  ; PAGE_DIR_PHYS
    mov ecx, 1024    ; 1024个条目
.clear_page_dir:
    mov [edi], eax
    add edi, 4
    dec ecx
    jnz .clear_page_dir

    ; 清零页表0 (4KB)
    xor eax, eax
    mov edi, 0x51000  ; PAGE_TABLE0_PHYS
    mov ecx, 1024    ; 1024个条目
.clear_page_table:
    mov [edi], eax
    add edi, 4
    dec ecx
    jnz .clear_page_table

    ; 映射低1MB (256个4KB页)
    ; 页表0覆盖: 0x00000000 - 0x000FFFFF
    mov edi, 0x51000  ; 页表0起始地址
    xor ebx, ebx      ; 物理地址 = 0

.map_low_1mb:
    ; 设置页表项: present=1, writable=1, 物理地址>>12
    mov eax, ebx
    shr eax, 12       ; 物理地址 >> 12
    or eax, 0x03      ; present=1, writable=1
    mov [edi], eax ; 映射低1MB (256个4KB页)

    add ebx, 0x1000   ; 下一个4KB页
    add edi, 4        ; 下一个页表项
    cmp ebx, 0x100000 ; 是否到达1MB
    jl .map_low_1mb

    ; 设置页目录项[0]指向页表0
    mov eax, 0x51000  ; PAGE_TABLE0_PHYS
    shr eax, 12       ; 物理地址 >> 12
    or eax, 0x03      ; present=1, writable=1
    mov [0x50000], eax  ; 页目录项[0]

    ; ============================================
    ; 启用分页
    ; ============================================

    ; 设置CR3指向页目录
    mov eax, 0x50000  ; PAGE_DIR_PHYS
    mov cr3, eax

    ; 启用分页（设置CR0.PG位）
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; 跳转刷新流水线
    jmp .flush
.flush:
    ; 调用paging_init完善页表
    call paging_init

    ; 调用内核主函数
    call kernel_main

    jmp $