#ifndef OSKERNEL_KERNEL_IDT_INTERRUPT_H
#define OSKERNEL_KERNEL_IDT_INTERRUPT_H

#include <stdint.h>
#include "idt.h"

// ============================================
// 中断处理函数指针类型
// ============================================
// 用于注册自定义中断处理函数
typedef void (*interrupt_handler_t)(struct interrupt_frame* frame);

// ============================================
// 函数声明
// ============================================

// 初始化中断处理框架
void interrupt_init(void);

// 通用中断处理函数
// 由 ISR 汇编入口调用
// frame: 中断栈帧指针
// interrupt_number: 中断向量号
void interrupt_handler(struct interrupt_frame* frame, uint32_t interrupt_number);

// 注册自定义中断处理函数
// vector: 中断向量号 (0-255)
// handler: 处理函数指针
// 返回: 0 成功，-1 失败
int interrupt_register_handler(uint8_t vector, interrupt_handler_t handler);

// 注销中断处理函数
// vector: 中断向量号 (0-255)
void interrupt_unregister_handler(uint8_t vector);

// 获取中断名称
// interrupt_number: 中断向量号
// 返回: 中断名称字符串
const char* interrupt_get_name(uint32_t interrupt_number);

// 检查中断是否发生
// irq: IRQ 号 (0-15)
// 返回: 1 = 发生，0 = 未发生
int interrupt_pending(uint8_t irq);

#endif // OSKERNEL_KERNEL_IDT_INTERRUPT_H
