#ifndef OSKERNEL_KERNEL_IDT_IDT_H
#define OSKERNEL_KERNEL_IDT_IDT_H

#include <stdint.h>

// ============================================
// IDT 门描述符类型标志
// ============================================
// P (Present): 段存在位
// DPL (Descriptor Privilege Level): 特权级
// S (System): 系统段标志
// Type: 门描述符类型

// 中断门：通过中断调用，自动禁用中断（清 IF 位）
#define IDT_GATE_INTERRUPT  0x8E  // P=1, DPL=00, S=0, Type=1110 (32位中断门)

// 陷阱门：通过陷阱调用，不会自动禁用中断
#define IDT_GATE_TRAP       0x8F  // P=1, DPL=00, S=0, Type=1111 (32位陷阱门)

// ============================================
// 中断描述符表项结构（8 字节）
// ============================================
// x86 IDT 表项格式：
// Bits 0-15:   ISR 偏移低16位 (base_lo)
// Bits 16-31:  段选择子 (selector)
// Bits 32-39:  保留位，必须为0 (always0)
// Bits 40-47:  类型和属性 (flags)
// Bits 48-63:  ISR 偏移高16位 (base_hi)

struct idt_entry {
    uint16_t base_lo;      // ISR 低16位地址
    uint16_t selector;     // 内核代码段选择子 (0x08)
    uint8_t  always0;      // 保留位，始终为0
    uint8_t  flags;        // 门描述符类型和属性
    uint16_t base_hi;      // ISR 高16位地址
} __attribute__((packed));

// ============================================
// IDT 指针结构（6 字节）
// ============================================
// lgdt/lidt 指令从内存加载此结构到 GDTR/IDTR 寄存器
// - 前 2 字节：IDT 界限（大小 - 1）
// - 后 4 字节：IDT 基地址

struct idt_ptr {
    uint16_t limit;        // IDT 界限（字节数 - 1）
    uint32_t base;         // IDT 基地址
} __attribute__((packed));

// ============================================
// 中断栈帧结构
// ============================================
// 当 CPU 响应中断时，会自动压入以下信息到栈中
// 这个结构对应栈中的布局（从高地址到低地址）

struct interrupt_frame {
    // 以下字段由 CPU 在中断响应时自动压入
    uint32_t eip;          // 返回地址（中断发生时的指令地址）
    uint32_t cs;           // 代码段选择子
    uint32_t eflags;       // 标志寄存器

    // 以下字段仅在特权级切换时由 CPU 压入
    // 如果中断从用户模式（特权级3）切换到内核模式（特权级0）
    uint32_t esp;          // 栈指针（可能）
    uint32_t ss;           // 栈段选择子（可能）
} __attribute__((packed));

// ============================================
// 函数声明
// ============================================

// 初始化 IDT
void idt_init(void);

// 设置中断门
// vector: 中断向量号 (0-255)
// handler: 中断处理函数地址
// selector: 段选择子（通常是 0x08）
// type_attr: 门描述符类型和属性
void idt_set_gate(uint8_t vector, uint32_t handler, uint16_t selector, uint8_t type_attr);

// 通用中断处理函数
// 由 ISR 汇编入口调用
void interrupt_handler(struct interrupt_frame* frame, uint32_t interrupt_number);

#endif // OSKERNEL_KERNEL_IDT_IDT_H
