#include "serial.h"

// 串口端口定义 (COM1)
#define SERIAL_PORT 0x3F8

// 初始化串口 (COM1)
// 波特率: 115200, 8数据位, 1停止位, 无校验
void serial_init() {
    // 禁用中断
    outb(SERIAL_PORT + 1, 0x00);
    
    // 设置波特率除数 (115200 = 1)
    // 先设置 DLAB=1 以访问除数寄存器
    outb(SERIAL_PORT + 3, 0x80);
    outb(SERIAL_PORT + 0, 0x01);  // 除数低字节 = 1
    outb(SERIAL_PORT + 1, 0x00);  // 除数高字节 = 0
    
    // 设置线路控制寄存器: 8位数据, 1位停止, 无校验
    outb(SERIAL_PORT + 3, 0x03);
    
    // 启用 FIFO, 清空缓冲区, 14字节阈值
    outb(SERIAL_PORT + 2, 0xC7);
    
    // 设置 MODEM 控制寄存器: 启用 DTR, RTS, OUT2
    outb(SERIAL_PORT + 4, 0x0B);
}

// 检查发送缓冲区是否为空
static int serial_is_transmit_empty() {
    return inb(SERIAL_PORT + 5) & 0x20;
}

// 发送单个字符
void serial_putc(char c) {
    // 等待发送缓冲区为空
    while (serial_is_transmit_empty() == 0);
    
    outb(SERIAL_PORT, c);
}

// 发送字符串
void serial_puts(const char* str) {
    while (*str) {
        serial_putc(*str++);
    }
}

// 发送字符串并换行
void serial_putline(const char* str) {
    serial_puts(str);
    serial_putc('\r');
    serial_putc('\n');
}

// 将整数转换为字符串 (辅助函数)
static void itoa(int value, char* str, int base) {
    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    int tmp_value;
    
    // 处理负数 (仅十进制)
    if (value < 0 && base == 10) {
        *ptr++ = '-';
    }
    
    ptr1 = ptr;
    
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
    } while (value);
    
    *ptr-- = '\0';
    
    // 反转字符串
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

// 格式化输出 (简化版 printf)
// 支持: %s(字符串), %d(十进制整数), %x(十六进制), %c(字符)
void serial_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    char buffer[32];
    
    while (*fmt) {
        if (*fmt == '%' && *(fmt + 1)) {
            fmt++;
            switch (*fmt) {
                case 's':
                    serial_puts(va_arg(args, char*));
                    break;
                case 'd':
                    itoa(va_arg(args, int), buffer, 10);
                    serial_puts(buffer);
                    break;
                case 'x':
                    serial_puts("0x");
                    itoa(va_arg(args, int), buffer, 16);
                    serial_puts(buffer);
                    break;
                case 'c':
                    serial_putc((char)va_arg(args, int));
                    break;
                case '%':
                    serial_putc('%');
                    break;
                default:
                    serial_putc('%');
                    serial_putc(*fmt);
                    break;
            }
        } else {
            serial_putc(*fmt);
        }
        fmt++;
    }
    
    va_end(args);
}
