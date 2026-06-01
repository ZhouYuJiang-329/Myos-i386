#include "vga.h"

// VGA 显存指针
static unsigned char* vga_buffer = (unsigned char*)VGA_ADDRESS;

// 当前光标位置
static int cursor_x = 0;
static int cursor_y = 0;

// 当前字符属性（高4位背景色，低4位前景色）
static unsigned char current_attr = 0x0F;  // 白色前景，黑色背景

// 更新硬件光标位置
static void update_cursor(void) {
    unsigned short pos = cursor_y * VGA_WIDTH + cursor_x;

    // 发送光标位置低字节
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));

    // 发送光标位置高字节
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

// 向上滚动屏幕一行
static void scroll(void) {
    // 将第 1-24 行的内容复制到第 0-23 行
    int i;
    for (i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH * 2; i++) {
        vga_buffer[i] = vga_buffer[i + VGA_WIDTH * 2];
    }

    // 清空最后一行（第 24 行）
    for (i = (VGA_HEIGHT - 1) * VGA_WIDTH * 2; i < VGA_HEIGHT * VGA_WIDTH * 2; i += 2) {
        vga_buffer[i] = ' ';       // 字符
        vga_buffer[i + 1] = current_attr;  // 属性
    }
}

// 初始化 VGA 驱动
void vga_init(void) {
    vga_clear();
    update_cursor();
}

// 清屏并将光标移到左上角
void vga_clear(void) {
    int i;
    for (i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        vga_buffer[i] = ' ';       // 字符
        vga_buffer[i + 1] = current_attr;  // 属性
    }

    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

// 设置前景色和背景色
void vga_set_color(enum vga_color fg, enum vga_color bg) {
    current_attr = (unsigned char)((bg << 4) | (fg & 0x0F));
}

// 打印单个字符
void vga_putc(char c) {
    if (c == '\n') {
        // 换行符：移动到下一行开头
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        // 回车符：移动到当前行开头
        cursor_x = 0;
    } else if (c == '\b') {
        // 退格符：后退一列
        if (cursor_x > 0) {
            cursor_x--;
            // 用空格覆盖当前位置
            int offset = (cursor_y * VGA_WIDTH + cursor_x) * 2;
            vga_buffer[offset] = ' ';
            vga_buffer[offset + 1] = current_attr;
        }
    } else if (c >= 32) {
        // 可打印字符
        int offset = (cursor_y * VGA_WIDTH + cursor_x) * 2;
        vga_buffer[offset] = (unsigned char)c;
        vga_buffer[offset + 1] = current_attr;
        cursor_x++;
    }

    // 处理超出列宽的情况（自动换行）
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    // 处理超出屏幕底部的情况（滚动）
    if (cursor_y >= VGA_HEIGHT) {
        scroll();
        cursor_y = VGA_HEIGHT - 1;
    }

    update_cursor();
}

// 打印字符串
void vga_puts(const char* str) {
    while (*str != '\0') {
        vga_putc(*str);
        str++;
    }
}
