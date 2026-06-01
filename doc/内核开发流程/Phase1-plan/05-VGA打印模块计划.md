# Phase 1 - VGA 打印模块开发计划

## 1. 任务概述

**目标**: 实现操作系统 VGA 文本模式显示驱动，提供字符输出、光标控制、颜色设置和屏幕滚动功能。

**重要性**: VGA 显示是操作系统的核心输出设备，为内核调试、系统信息展示和用户交互提供可视化界面。

---

## 2. 前置依赖

- [x] 串口输出模块已完成（用于调试输出）
- [x] 保护模式切换已完成
- [x] 端口 I/O 操作已实现（outb/inb）
- [x] Makefile 构建系统已配置

---

## 3. 开发步骤

### Step 1: 设计 VGA 数据结构 (30分钟)

**任务内容**:
- 定义 VGA 显存地址常量 (0xB8000)
- 定义屏幕尺寸常量 (80x25)
- 定义颜色枚举 (16色)
- 声明公共 API 函数

**输出文件**:
- `oskernel/kernel/drivers/vga.h`

**验收标准**:
- [ ] 常量定义正确，符合 VGA 文本模式规范
- [ ] 颜色枚举完整，包含 16 种标准颜色
- [ ] 函数声明清晰，符合 C 语言规范

---

### Step 2: 实现 VGA 基础功能 (1小时)

**任务内容**:
- 实现 `vga_init()` - 初始化 VGA 驱动
- 实现 `vga_clear()` - 清屏并重置光标
- 实现 `update_cursor()` - 更新硬件光标位置
- 实现 `scroll()` - 屏幕向上滚动一行

**关键技术点**:
- VGA 显存映射：每个字符占 2 字节（字符 + 属性）
- 硬件光标控制：通过端口 0x3D4/0x3D5 设置
- 滚屏实现：内存复制 + 最后一行清空

**输出文件**:
- 修改 `oskernel/kernel/drivers/vga.c`

**验收标准**:
- [ ] 初始化后屏幕清空，光标位于左上角
- [ ] 硬件光标位置正确更新
- [ ] 滚屏功能正常工作

---

### Step 3: 实现字符输出功能 (1小时)

**任务内容**:
- 实现 `vga_set_color()` - 设置前景色和背景色
- 实现 `vga_putc()` - 打印单个字符（核心函数）
- 实现 `vga_puts()` - 打印字符串

**关键技术点**:
- 特殊字符处理：\n（换行）、\r（回车）、\b（退格）
- 自动换行：cursor_x >= 80 时换行
- 自动滚屏：cursor_y >= 25 时滚动
- 属性字节计算：(bg << 4) | (fg & 0x0F)

**输出文件**:
- 修改 `oskernel/kernel/drivers/vga.c`

**验收标准**:
- [ ] 可打印字符正确显示
- [ ] 特殊字符处理正确
- [ ] 颜色设置功能正常
- [ ] 边界情况处理完善

---

### Step 4: 集成到内核主程序 (30分钟)

**任务内容**:
- 修改 main.c 包含 vga.h 头文件
- 调用 vga_init() 初始化 VGA
- 添加 VGA 输出测试代码
- 验证与串口输出的协同工作

**输出文件**:
- 修改 `oskernel/init/main.c`

**验收标准**:
- [ ] 内核启动时 VGA 正确初始化
- [ ] 测试信息正确显示在屏幕上
- [ ] 颜色切换功能正常

---

### Step 5: 修改 Makefile (15分钟)

**任务内容**:
- 添加 DRIVERS_DIR 变量
- 添加 vga.o 编译规则
- 更新 KERNEL_BIN 依赖
- 更新 clean 目标

**输出文件**:
- 修改 `Makefile`

**验收标准**:
- [ ] make 能正确编译 vga.c
- [ ] 链接时包含 vga.o
- [ ] make clean 能清理所有生成文件

---

## 4. 测试验证

### 测试用例 1: 基本字符输出
```
代码:
    vga_puts("Hello, OS Kernel!");

预期输出:
    屏幕左上角显示: Hello, OS Kernel!
```

### 测试用例 2: 颜色设置
```
代码:
    vga_set_color(VGA_GREEN, VGA_BLACK);
    vga_puts("Green Text");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_puts("White Text");

预期输出:
    "Green Text" 显示为绿色前景
    "White Text" 显示为白色前景
```

### 测试用例 3: 特殊字符处理
```
代码:
    vga_puts("Line1\nLine2\rOverwrite\bX");

预期输出:
    Line1
    OverwriteX
```

### 测试用例 4: 自动换行和滚屏
```
代码:
    for (int i = 0; i < 30; i++) {
        vga_puts("This is a long line that should wrap and scroll.\n");
    }

预期输出:
    - 每行超过 80 字符自动换行
    - 超过 25 行后自动向上滚动
    - 光标始终可见
```

---

## 5. 关键技术要点

### 5.1 VGA 文本模式内存布局

```
显存起始地址: 0xB8000
屏幕尺寸: 80 列 x 25 行 = 2000 字符
每个字符占用 2 字节:
  +0: ASCII 字符码
  +1: 属性字节

属性字节格式:
  Bit 7: 闪烁（BLINK）
  Bit 6-4: 背景色（BG）
  Bit 3: 高亮（INTENSITY）
  Bit 2-0: 前景色（FG）

地址计算: offset = (y * 80 + x) * 2
```

### 5.2 硬件光标控制

```
光标位置寄存器:
  索引端口 0x3D4:
    0x0E - 光标位置高字节
    0x0F - 光标位置低字节
  数据端口 0x3D5: 写入位置值

位置计算: pos = y * 80 + x
```

### 5.3 颜色定义

| 值 | 名称 | 颜色 |
|----|------|------|
| 0 | VGA_BLACK | 黑色 |
| 1 | VGA_BLUE | 蓝色 |
| 2 | VGA_GREEN | 绿色 |
| 3 | VGA_CYAN | 青色 |
| 4 | VGA_RED | 红色 |
| 5 | VGA_MAGENTA | 洋红 |
| 6 | VGA_BROWN | 棕色 |
| 7 | VGA_LIGHT_GREY | 亮灰 |
| 8 | VGA_DARK_GREY | 深灰 |
| 9 | VGA_LIGHT_BLUE | 亮蓝 |
| 10 | VGA_LIGHT_GREEN | 亮绿 |
| 11 | VGA_LIGHT_CYAN | 亮青 |
| 12 | VGA_LIGHT_RED | 亮红 |
| 13 | VGA_LIGHT_MAGENTA | 亮洋红 |
| 14 | VGA_YELLOW | 黄色 |
| 15 | VGA_WHITE | 白色 |

### 5.4 特殊字符处理

| 字符 | ASCII | 处理逻辑 |
|------|-------|----------|
| \n | 10 | cursor_x = 0; cursor_y++; |
| \r | 13 | cursor_x = 0; |
| \b | 8 | if (cursor_x > 0) cursor_x--; 覆盖空格 |
| < 32 | - | 忽略（控制字符） |
| >= 32 | - | 显示字符，cursor_x++ |

---

## 6. 风险与应对

| 风险 | 可能性 | 影响 | 应对措施 |
|------|--------|------|----------|
| 显存地址错误 | 中 | 高 | 确认使用 0xB8000（彩色文本模式） |
| 光标不显示 | 中 | 中 | 检查端口 I/O 操作是否正确 |
| 滚屏闪烁 | 低 | 低 | 优化滚屏实现，使用内存块复制 |
| 颜色显示异常 | 低 | 低 | 检查属性字节计算是否正确 |

---

## 7. 时间安排

| 步骤 | 预计时间 | 实际时间 | 状态 |
|------|----------|----------|------|
| Step 1: 数据结构 | 30分钟 | | |
| Step 2: 基础功能 | 1小时 | | |
| Step 3: 字符输出 | 1小时 | | |
| Step 4: 集成测试 | 30分钟 | | |
| Step 5: Makefile | 15分钟 | | |
| **总计** | **~3.5小时** | | |

---

## 8. 参考资源

- [OSDev Wiki - VGA Hardware](https://wiki.osdev.org/VGA_Hardware)
- [OSDev Wiki - Text UI](https://wiki.osdev.org/Text_UI)
- [VGA Text Mode](https://wiki.osdev.org/VGA_Hardware#Text_mode)
- [CRT Controller Registers](https://wiki.osdev.org/VGA_Hardware#CRT_Controller_Registers)
