#ifndef OSKERNEL_KERNEL_IDT_PIC_H
#define OSKERNEL_KERNEL_IDT_PIC_H

#include <stdint.h>

// ============================================
// 8259A 可编程中断控制器 (PIC) 定义
// ============================================
// x86 系统通常有两个 8259A PIC：
// - 主 PIC (Master PIC): 端口 0x20-0x21
// - 从 PIC (Slave PIC): 端口 0xA0-0xA1

// ============================================
// PIC I/O 端口地址
// ============================================
#define PIC_MASTER_COMMAND     0x20    // 主 PIC 命令端口
#define PIC_MASTER_DATA        0x21    // 主 PIC 数据端口
#define PIC_SLAVE_COMMAND      0xA0    // 从 PIC 命令端口
#define PIC_SLAVE_DATA         0xA1    // 从 PIC 数据端口

// ============================================
// ICW1 (Initialization Command Word 1)
// ============================================
// 用于初始化 PIC，设置初始化序列
#define PIC_ICW1_ICW4          0x01    // 需要 ICW4
#define PIC_ICW1_SINGLE        0x02    // 单片模式（不级联）
#define PIC_ICW1_INTERVAL4     0x04    // 调用地址间隔 4 字节
#define PIC_ICW1_LEVEL         0x08    // 边缘触发（Level triggered）
#define PIC_ICW1_INIT          0x10    // 初始化命令

// ============================================
// ICW4 (Initialization Command Word 4)
// ============================================
// 用于设置 PIC 的工作模式
#define PIC_ICW4_8086          0x01    // 8086/8088 模式
#define PIC_ICW4_AUTO_EOI      0x02    // 自动 EOI 模式
#define PIC_ICW4_BUF_SLAVE     0x08    // 缓冲模式（从 PIC）
#define PIC_ICW4_BUF_MASTER    0x0C    // 缓冲模式（主 PIC）
#define PIC_ICW4_SFNM          0x10    // 特殊完全嵌套模式

// ============================================
// EOI (End of Interrupt) 命令
// ============================================
#define PIC_EOI                0x20    // 非特殊 EOI 命令

// ============================================
// 函数声明
// ============================================

// 初始化主从 PIC
// master_offset: 主 PIC 中断向量偏移（通常 0x20）
// slave_offset: 从 PIC 中断向量偏移（通常 0x28）
void pic_init(uint8_t master_offset, uint8_t slave_offset);

// 发送 EOI (End of Interrupt) 给指定的 PIC
// irq: IRQ 号 (0-15)
void pic_send_eoi(uint8_t irq);

// 设置中断掩码（禁用/启用指定的 IRQ）
// irq: IRQ 号 (0-15)
// masked: 1 = 禁用, 0 = 启用
void pic_set_mask(uint8_t irq, uint8_t masked);

// 读取中断掩码
// 返回主 PIC 和从 PIC 的掩码（主 PIC 在低8位，从 PIC 在高8位）
uint16_t pic_get_mask(void);

// 读取中断请求寄存器 (IRR)
// 返回主 PIC 和从 PIC 的 IRR
uint16_t pic_get_irr(void);

// 读取中断服务寄存器 (ISR)
// 返回主 PIC 和从 PIC 的 ISR
uint16_t pic_get_isr(void);

// 禁用所有中断
void pic_disable_all(void);

// 启用所有中断
void pic_enable_all(void);

#endif // OSKERNEL_KERNEL_IDT_PIC_H
