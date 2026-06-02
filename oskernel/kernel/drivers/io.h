#ifndef OSKERNEL_KERNEL_DRIVERS_IO_H
#define OSKERNEL_KERNEL_DRIVERS_IO_H

#include <stdint.h>

// ============================================
// 端口 I/O 函数（内联汇编实现）
// ============================================
// 这些函数用于直接访问 x86 端口，是所有硬件驱动的基础

// 向指定端口写入一个字节
// port: 端口号 (0x0000 - 0xFFFF)
// value: 要写入的值 (0x00 - 0xFF)
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

// 从指定端口读取一个字节
// port: 端口号 (0x0000 - 0xFFFF)
// 返回: 读取到的值
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ __volatile__("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// 向指定端口写入一个字（16位）
// port: 端口号
// value: 要写入的值
static inline void outw(uint16_t port, uint16_t value) {
    __asm__ __volatile__("outw %0, %1" : : "a"(value), "Nd"(port));
}

// 从指定端口读取一个字（16位）
// port: 端口号
// 返回: 读取到的值
static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ __volatile__("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// 短暂延迟，等待硬件操作完成
// 通过向未使用的端口 0x80 写入来产生短暂延迟
static inline void io_wait(void) {
    outb(0x80, 0);
}

#endif // OSKERNEL_KERNEL_DRIVERS_IO_H
