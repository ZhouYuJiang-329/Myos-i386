#ifndef __VGA_H__
#define __VGA_H__

#include "serial.h"

// VGA 显存地址
#define VGA_ADDRESS 0xB8000

// 屏幕尺寸
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// 颜色枚举
enum vga_color {
    VGA_BLACK = 0,
    VGA_BLUE = 1,
    VGA_GREEN = 2,
    VGA_CYAN = 3,
    VGA_RED = 4,
    VGA_MAGENTA = 5,
    VGA_BROWN = 6,
    VGA_LIGHT_GREY = 7,
    VGA_DARK_GREY = 8,
    VGA_LIGHT_BLUE = 9,
    VGA_LIGHT_GREEN = 10,
    VGA_LIGHT_CYAN = 11,
    VGA_LIGHT_RED = 12,
    VGA_LIGHT_MAGENTA = 13,
    VGA_YELLOW = 14,
    VGA_WHITE = 15,
};

// 公共 API
void vga_init(void);
void vga_putc(char c);
void vga_puts(const char* str);
void vga_set_color(enum vga_color fg, enum vga_color bg);
void vga_clear(void);

#endif
