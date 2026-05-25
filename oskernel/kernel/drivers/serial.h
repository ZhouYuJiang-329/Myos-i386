#ifndef SERIAL_H
#define SERIAL_H

// 端口 I/O 函数 (内联汇编实现)
static inline void outb(unsigned short port, unsigned char value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
    unsigned char value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// 可变参数支持
typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, t) __builtin_va_arg(v, t)

// 串口初始化
void serial_init();

// 字符输出
void serial_putc(char c);
void serial_puts(const char* str);
void serial_putline(const char* str);

// 格式化输出
void serial_printf(const char* fmt, ...);

#endif // SERIAL_H
