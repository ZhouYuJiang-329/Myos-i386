# VGA 打印模块代码详解

## 1. 模块概述

VGA 打印模块负责：
1. **字符显示**：在 VGA 文本模式下输出字符到屏幕
2. **光标控制**：维护光标位置并更新硬件光标
3. **颜色管理**：支持 16 色前景/背景色设置
4. **屏幕滚动**：自动处理超出屏幕内容的滚动

---

## 2. 核心数据结构

### 2.1 VGA 显存地址与尺寸

```c
// oskernel/kernel/drivers/vga.h
#define VGA_ADDRESS 0xB8000    // VGA 彩色文本模式显存起始地址
#define VGA_WIDTH 80           // 屏幕宽度（字符数）
#define VGA_HEIGHT 25          // 屏幕高度（字符数）
```

**设计说明**：
- `0xB8000` 是 x86 架构 VGA 彩色文本模式的固定显存地址
- 屏幕尺寸 80x25 是标准的 VGA 文本模式分辨率
- 每个字符占用 2 字节，总显存需求：80 × 25 × 2 = 4000 字节

### 2.2 颜色枚举定义

```c
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
```

**颜色编码规则**：
- 4 位颜色值，支持 16 种标准颜色
- 值 0-7 为标准色，8-15 为高亮色
- 实际显示效果取决于 VGA 调色板配置

### 2.3 模块内部状态

```c
// oskernel/kernel/drivers/vga.c
static unsigned char* vga_buffer = (unsigned char*)VGA_ADDRESS;
static int cursor_x = 0;
static int cursor_y = 0;
static unsigned char current_attr = 0x0F;
```

**状态变量说明**：

| 变量 | 类型 | 说明 |
|------|------|------|
| `vga_buffer` | `unsigned char*` | 显存指针，指向 0xB8000 |
| `cursor_x` | `int` | 当前光标列位置 (0-79) |
| `cursor_y` | `int` | 当前光标行位置 (0-24) |
| `current_attr` | `unsigned char` | 当前字符属性（前景+背景色） |

---

## 3. 显存布局详解

### 3.1 内存映射结构

```
物理地址 0xB8000 开始的显存布局：

+--------+--------+--------+--------+--------+--------+--------+--------+
| 字符0  | 属性0  | 字符1  | 属性1  | 字符2  | 属性2  | ...... | 字符79 |
| (1B)   | (1B)   | (1B)   | (1B)   | (1B)   | (1B)   |        | (1B)   |
+--------+--------+--------+--------+--------+--------+--------+--------+
0xB8000  0xB8001  0xB8002  0xB8003  0xB8004  0xB8005          0xB809F

行 0: 0xB8000 - 0xB809F  (160 字节)
行 1: 0xB80A0 - 0xB813F  (160 字节)
...
行 24: 0xB8F00 - 0xB8F9F (160 字节)
```

### 3.2 属性字节格式

```
Bit 7    Bit 6-4    Bit 3    Bit 2-0
+-----+-----------+-----+-----------+
| 闪烁 |  背景色   | 高亮  |  前景色   |
| (1b) |   (3b)    | (1b) |   (3b)    |
+-----+-----------+-----+-----------+

默认值 0x0F:
  背景色 = 0 (黑色)
  前景色 = 15 (白色)
  高亮 = 1
  闪烁 = 0
```

### 3.3 地址计算公式

```c
// 计算字符在显存中的偏移量
int offset = (cursor_y * VGA_WIDTH + cursor_x) * 2;

// 示例：第 10 行第 20 列
// offset = (10 * 80 + 20) * 2 = 1640 = 0x668
// 物理地址 = 0xB8000 + 0x668 = 0xB8668
```

---

## 4. 硬件光标控制

### 4.1 光标位置寄存器

```c
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
```

**CRT 控制器寄存器**：

| 端口 | 索引 | 功能 |
|------|------|------|
| 0x3D4 | - | CRT 索引寄存器 |
| 0x3D5 | - | CRT 数据寄存器 |
| - | 0x0E | 光标位置高字节 |
| - | 0x0F | 光标位置低字节 |

**操作流程**：
1. 向 0x3D4 写入索引（0x0E 或 0x0F）
2. 向 0x3D5 写入数据（光标位置的高/低字节）

### 4.2 光标位置计算

```
光标位置值 = y * 80 + x

示例：光标在第 5 行第 10 列
  pos = 5 * 80 + 10 = 410 = 0x19A
  
  高字节 = 0x01
  低字节 = 0x9A
```

---

## 5. 屏幕滚动实现

### 5.1 滚屏算法

```c
static void scroll(void) {
    // 将第 1-24 行的内容复制到第 0-23 行
    int i;
    for (i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH * 2; i++) {
        vga_buffer[i] = vga_buffer[i + VGA_WIDTH * 2];
    }

    // 清空最后一行（第 24 行）
    for (i = (VGA_HEIGHT - 1) * VGA_WIDTH * 2; 
         i < VGA_HEIGHT * VGA_WIDTH * 2; i += 2) {
        vga_buffer[i] = ' ';       // 字符
        vga_buffer[i + 1] = current_attr;  // 属性
    }
}
```

**算法步骤**：
1. 将第 1 行到第 24 行的内容向上复制一行（覆盖第 0 行到第 23 行）
2. 清空第 24 行（最后一行），填充空格和当前属性

**内存操作示意图**：
```
滚动前：
  行 0: [内容 A]
  行 1: [内容 B]
  ...
  行 23: [内容 X]
  行 24: [内容 Y]

滚动后：
  行 0: [内容 B]  ← 原行 1
  行 1: [内容 C]  ← 原行 2
  ...
  行 23: [内容 Y] ← 原行 24
  行 24: [空格]   ← 新清空
```

---

## 6. 字符输出核心逻辑

### 6.1 字符处理状态机

```c
void vga_putc(char c) {
    if (c == '\n') {
        // 换行符
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        // 回车符
        cursor_x = 0;
    } else if (c == '\b') {
        // 退格符
        if (cursor_x > 0) {
            cursor_x--;
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

    // 边界处理...
}
```

**特殊字符处理表**：

| 字符 | ASCII | 处理逻辑 |
|------|-------|----------|
| `\n` | 10 | `cursor_x = 0; cursor_y++;` |
| `\r` | 13 | `cursor_x = 0;` |
| `\b` | 8 | `cursor_x--;` 用空格覆盖 |
| < 32 | - | 忽略（控制字符） |
| >= 32 | - | 显示字符，`cursor_x++` |

### 6.2 边界处理逻辑

```c
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
```

**边界条件处理**：
- **列溢出** (cursor_x >= 80)：自动换行到下一行开头
- **行溢出** (cursor_y >= 25)：触发滚屏，光标保持在第 24 行

---

## 7. 公共 API 实现

### 7.1 初始化与清屏

```c
void vga_init(void) {
    vga_clear();
    update_cursor();
}

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
```

### 7.2 颜色设置

```c
void vga_set_color(enum vga_color fg, enum vga_color bg) {
    current_attr = (unsigned char)((bg << 4) | (fg & 0x0F));
}
```

**属性字节计算示例**：
```
前景色 = VGA_GREEN (2)
背景色 = VGA_BLACK (0)

current_attr = (0 << 4) | (2 & 0x0F) = 0x02

前景色 = VGA_WHITE (15)
背景色 = VGA_BLUE (1)

current_attr = (1 << 4) | (15 & 0x0F) = 0x1F
```

### 7.3 字符串输出

```c
void vga_puts(const char* str) {
    while (*str != '\0') {
        vga_putc(*str);
        str++;
    }
}
```

---

## 8. 内存布局示意图

```
VGA 显存区域 (0xB8000 - 0xB8FFF):

+--------------------------------------------------+
| 0xB8000                                          |
| +----------------------------------------------+ |
| | 行 0 (80字符 x 2字节 = 160字节)               | |
| | [字符][属性][字符][属性]...                   | |
| +----------------------------------------------+ |
| | 行 1                                          | |
| | ...                                           | |
| +----------------------------------------------+ |
| | 行 24 (最后一行)                              | |
| +----------------------------------------------+ |
| 0xB8F9F                                          |
+--------------------------------------------------+

字符属性布局：
+--------+--------+
| 字符码 | 属性   |
+--------+--------+
  1字节   1字节

属性字节：
+-----+-----------+-----+-----------+
| 闪烁 | 背景色    | 高亮 | 前景色    |
| (1) |  (3)      | (1) |  (3)      |
+-----+-----------+-----+-----------+
```

---

## 9. 关键设计决策

### 9.1 为什么使用静态变量？

- **封装性**：光标位置和当前颜色是模块内部状态
- **线程安全**：单核内核无需考虑并发，静态变量足够
- **简洁性**：避免全局命名空间污染

### 9.2 为什么每次输出都更新硬件光标？

- **实时反馈**：用户能立即看到光标位置变化
- **简单可靠**：避免批量更新导致的复杂度
- **性能可接受**：端口 I/O 在现代 CPU 上开销很小

### 9.3 为什么退格要覆盖空格？

```c
// 退格处理
if (cursor_x > 0) {
    cursor_x--;
    int offset = (cursor_y * VGA_WIDTH + cursor_x) * 2;
    vga_buffer[offset] = ' ';       // 用空格覆盖
    vga_buffer[offset + 1] = current_attr;
}
```

**原因**：
- 仅移动光标会留下原字符可见
- 覆盖空格确保视觉上字符被"删除"
- 符合终端退格的预期行为

---

## 10. 调试技巧

### 10.1 验证显存写入

```c
// 直接写入显存测试
void test_vga_memory(void) {
    unsigned char* vga = (unsigned char*)0xB8000;
    
    // 在左上角显示 'A' (绿色)
    vga[0] = 'A';
    vga[1] = 0x0A;  // 绿色前景
    
    // 在右上角显示 'B' (红色)
    vga[158] = 'B';
    vga[159] = 0x0C;  // 红色前景
}
```

### 10.2 光标位置调试

```c
// 打印当前光标位置到串口
void debug_cursor(void) {
    serial_printf("Cursor: x=%d, y=%d, pos=%d\n", 
                  cursor_x, cursor_y, 
                  cursor_y * VGA_WIDTH + cursor_x);
}
```

### 10.3 颜色测试

```c
// 显示所有颜色组合
void test_colors(void) {
    for (int bg = 0; bg < 8; bg++) {
        for (int fg = 0; fg < 16; fg++) {
            vga_set_color(fg, bg);
            vga_putc('X');
        }
        vga_putc('\n');
    }
    vga_set_color(VGA_WHITE, VGA_BLACK);  // 恢复默认
}
```

---

## 11. 扩展方向

1. **光标样式控制**：支持块状/下划线光标，通过 CRT 寄存器 0x0A/0x0B 设置
2. **闪烁控制**：开启/关闭字符闪烁（属性字节 bit 7）
3. **屏幕保存/恢复**：实现显存内容的保存和恢复功能
4. **滚屏区域**：支持设置滚屏的顶部/底部边界
5. **VGA 图形模式**：扩展到 320x200 256 色图形模式

---

## 12. 参考代码索引

| 文件 | 功能 |
|------|------|
| `oskernel/kernel/drivers/vga.h` | 常量定义、颜色枚举、函数声明 |
| `oskernel/kernel/drivers/vga.c` | VGA 驱动实现 |
| `oskernel/kernel/drivers/serial.h` | 端口 I/O 函数 (outb/inb) |
| `oskernel/init/main.c` | 使用示例和测试代码 |

---

## 13. 相关硬件文档

- **VGA 文本模式**：80x25 彩色文本，显存 0xB8000
- **CRT 控制器**：端口 0x3D4/0x3D5，控制光标和显示参数
- **颜色调色板**：16 色标准 VGA 调色板
