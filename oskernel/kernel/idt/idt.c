#include "idt.h"
#include "../drivers/serial.h"

// ============================================
// IDT 相关变量
// ============================================

// 256 个 IDT 表项
static struct idt_entry idt[256];

// IDT 指针结构
static struct idt_ptr idtp;

// ============================================
// ISR 外部函数声明（定义在 isr.asm 中）
// ============================================
// 这些函数是中断服务程序的入口点
// 每个 ISR 都会保存寄存器，调用 interrupt_handler，然后返回

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

// IRQ 0-15 (硬件中断)
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

// ============================================
// 设置 IDT 门描述符
// ============================================
void idt_set_gate(uint8_t vector, uint32_t handler, uint16_t selector, uint8_t type_attr) {
    // 从处理函数地址提取低16位和高16位
    idt[vector].base_lo = handler & 0xFFFF;
    idt[vector].base_hi = (handler >> 16) & 0xFFFF;

    // 设置段选择子和属性
    idt[vector].selector = selector;
    idt[vector].always0 = 0;  // 保留位必须为0
    idt[vector].flags = type_attr;
}

// ============================================
// 初始化 IDT
// ============================================
void idt_init(void) {
    serial_puts("[IDT] Initializing IDT...\n");

    // 设置 IDT 指针
    idtp.limit = sizeof(idt) - 1;  // 界限 = 总大小 - 1
    idtp.base = (uint32_t)&idt;    // 基地址指向 IDT 数组

    // 清零 IDT 表
    uint8_t* dst = (uint8_t*)&idt;
    for (uint16_t i = 0; i < sizeof(idt); i++) {
        dst[i] = 0;
    }

    // ============================================
    // 设置 CPU 异常处理程序 (ISR 0-31)
    // 这些是 CPU 自动触发的中断（除法错误、调试、页错误等）
    // ============================================

    // ISR 0: 除法错误 (#DE)
    idt_set_gate(0, (uint32_t)isr0, 0x08, IDT_GATE_INTERRUPT);
    // ISR 1: 调试异常 (#DB)
    idt_set_gate(1, (uint32_t)isr1, 0x08, IDT_GATE_INTERRUPT);
    // ISR 2: NMI 中断
    idt_set_gate(2, (uint32_t)isr2, 0x08, IDT_GATE_INTERRUPT);
    // ISR 3: 断点 (#BP)
    idt_set_gate(3, (uint32_t)isr3, 0x08, IDT_GATE_TRAP);  // 陷阱门，不自动禁用中断
    // ISR 4: 溢出 (#OF)
    idt_set_gate(4, (uint32_t)isr4, 0x08, IDT_GATE_TRAP);
    // ISR 5: 越界 (#BR)
    idt_set_gate(5, (uint32_t)isr5, 0x08, IDT_GATE_INTERRUPT);
    // ISR 6: 无效操作码 (#UD)
    idt_set_gate(6, (uint32_t)isr6, 0x08, IDT_GATE_INTERRUPT);
    // ISR 7: 设备不可用 (#NM)
    idt_set_gate(7, (uint32_t)isr7, 0x08, IDT_GATE_INTERRUPT);
    // ISR 8: 双重故障 (#DF) - 错误码由 CPU 压入
    idt_set_gate(8, (uint32_t)isr8, 0x08, IDT_GATE_INTERRUPT);
    // ISR 9: 协处理器段溢出（保留）
    idt_set_gate(9, (uint32_t)isr9, 0x08, IDT_GATE_INTERRUPT);
    // ISR 10: 无效 TSS (#TS) - 错误码由 CPU 压入
    idt_set_gate(10, (uint32_t)isr10, 0x08, IDT_GATE_INTERRUPT);
    // ISR 11: 段不存在 (#NP) - 错误码由 CPU 压入
    idt_set_gate(11, (uint32_t)isr11, 0x08, IDT_GATE_INTERRUPT);
    // ISR 12: 栈段错误 (#SS) - 错误码由 CPU 压入
    idt_set_gate(12, (uint32_t)isr12, 0x08, IDT_GATE_INTERRUPT);
    // ISR 13: 一般保护错误 (#GP) - 错误码由 CPU 压入
    idt_set_gate(13, (uint32_t)isr13, 0x08, IDT_GATE_INTERRUPT);
    // ISR 14: 页错误 (#PF) - 错误码由 CPU 压入
    idt_set_gate(14, (uint32_t)isr14, 0x08, IDT_GATE_INTERRUPT);
    // ISR 15: 保留
    idt_set_gate(15, (uint32_t)isr15, 0x08, IDT_GATE_INTERRUPT);
    // ISR 16: x87 浮点错误 (#MF)
    idt_set_gate(16, (uint32_t)isr16, 0x08, IDT_GATE_INTERRUPT);
    // ISR 17: 对齐检查 (#AC) - 错误码由 CPU 压入
    idt_set_gate(17, (uint32_t)isr17, 0x08, IDT_GATE_INTERRUPT);
    // ISR 18: 机器检查 (#MC)
    idt_set_gate(18, (uint32_t)isr18, 0x08, IDT_GATE_INTERRUPT);
    // ISR 19: SIMD 流程异常 (#XM)
    idt_set_gate(19, (uint32_t)isr19, 0x08, IDT_GATE_INTERRUPT);
    // ISR 20-31: 保留
    idt_set_gate(20, (uint32_t)isr20, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(21, (uint32_t)isr21, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(22, (uint32_t)isr22, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(23, (uint32_t)isr23, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(24, (uint32_t)isr24, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(25, (uint32_t)isr25, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(26, (uint32_t)isr26, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(27, (uint32_t)isr27, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(28, (uint32_t)isr28, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(29, (uint32_t)isr29, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(30, (uint32_t)isr30, 0x08, IDT_GATE_INTERRUPT);
    idt_set_gate(31, (uint32_t)isr31, 0x08, IDT_GATE_INTERRUPT);

    // ============================================
    // 设置硬件中断处理程序 (IRQ 0-15)
    // 这些是 8259A PIC 产生的硬件中断
    // IRQ 0-7 映射到 ISR 32-39 (主 PIC)
    // IRQ 8-15 映射到 ISR 40-47 (从 PIC)
    // ============================================

    // IRQ 0: 定时器 (主 PIC)
    idt_set_gate(32, (uint32_t)irq0, 0x08, IDT_GATE_INTERRUPT);
    // IRQ 1: 键盘 (主 PIC)
    idt_set_gate(33, (uint32_t)irq1, 0x08, IDT_GATE_INTERRUPT);
    // IRQ 2: 级联 (主 PIC)
    idt_set_gate(34, (uint32_t)irq2, 0x08, IDT_GATE_INTERRUPT);
    // IRQ 3: COM2 (主 PIC)
    idt_set_gate(35, (uint32_t)irq3, 0x08, IDT_GATE_INTERRUPT);
    // IRQ 4: COM1 (主 PIC)
    idt_set_gate(36, (uint32_t)irq4, 0x08, IDT_GATE_INTERRUPT);
    // IRQ 5: LPT2 (主 PIC)
    idt_set_gate(37, (uint32_t)irq5, 0x08, IDT_GATE_INTERRUPT);
    // IRQ 6: 软盘 (主 PIC)
    idt_set_gate(38, (uint32_t)irq6, 0x08, IDT_GATE_INTERRUPT);
    // IRQ 7: LPT1 / 并口 (主 PIC)
    idt_set_gate(39, (uint32_t)irq7, 0x08, IDT_GATE_INTERRUPT);

    // IRQ 8: RTC 时钟 (从 PIC)
    idt_set_gate(40, (uint32_t)irq8, 0x08, IDT_GATE_INTERRUPT);
    // IRQ 9: ACPI (从 PIC)
    idt_set_gate(41, (uint32_t)irq9, 0x08, IDT_GATE_INTERRUPT);
    // IRQ 10: 开放 (从 PIC)
    idt_set_gate(42, (uint32_t)irq10, 0x08, IDT_GATE_INTERRUPT);
    // IRQ 11: 开放 (从 PIC)
    idt_set_gate(43, (uint32_t)irq11, 0x08, IDT_GATE_INTERRUPT);
    // IRQ 12: PS/2 鼠标 (从 PIC)
    idt_set_gate(44, (uint32_t)irq12, 0x08, IDT_GATE_INTERRUPT);
    // IRQ 13: FPU / 协处理器 (从 PIC)
    idt_set_gate(45, (uint32_t)irq13, 0x08, IDT_GATE_INTERRUPT);
    // IRQ 14: 主 ATA 硬盘 (从 PIC)
    idt_set_gate(46, (uint32_t)irq14, 0x08, IDT_GATE_INTERRUPT);
    // IRQ 15: 从 ATA 硬盘 (从 PIC)
    idt_set_gate(47, (uint32_t)irq15, 0x08, IDT_GATE_INTERRUPT);

    // ============================================
    // 加载 IDT 到 IDTR 寄存器
    // ============================================
    // lidt 指令从内存加载 6 字节到 IDTR 寄存器：
    //   - 前 2 字节：IDT 界限
    //   - 后 4 字节：IDT 基地址
    __asm__ __volatile__("lidt %0" : : "m"(idtp));

    serial_puts("[IDT] IDT initialized successfully!\n");
    serial_printf("[IDT] IDT base: 0x%x, limit: %d bytes\n", idtp.base, idtp.limit + 1);
}
