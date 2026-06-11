#include "pic.h"
#include "serial.h"

// ============================================
// 初始化 8259A PIC
// ============================================
void pic_init(uint8_t master_offset, uint8_t slave_offset) {
    serial_puts("[PIC] Initializing 8259A PIC...\n");

    // ============================================
    // ICW1: 开始初始化序列
    // ============================================
    // 发送初始化命令给主 PIC 和从 PIC
    // ICW1 格式：
    //   Bit 0: 需要 ICW4
    //   Bit 1: 单片模式（不级联）
    //   Bit 2: 调用地址间隔（8086 模式下忽略）
    //   Bit 3: 边缘触发（Level triggered）
    //   Bit 4: 初始化命令（必须为1）
    //   Bit 5-7: 8086 模式下忽略

    // 发送 ICW1 给主 PIC
    outb(PIC_MASTER_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    io_wait();

    // 发送 ICW1 给从 PIC
    outb(PIC_SLAVE_COMMAND, PIC_ICW1_INIT | PIC_ICW1_ICW4);
    io_wait();

    // ============================================
    // ICW2: 设置中断向量偏移
    // ============================================
    // ICW2 指定 IRQ 0-7 映射到哪个中断向量号
    // 主 PIC: IRQ 0-7 -> ISR master_offset 到 master_offset+7
    // 从 PIC: IRQ 0-7 -> ISR slave_offset 到 slave_offset+7

    // 发送 ICW2 给主 PIC（IRQ 0-7 映射到 master_offset）
    outb(PIC_MASTER_DATA, master_offset);
    io_wait();

    // 发送 ICW2 给从 PIC（IRQ 0-7 映射到 slave_offset）
    outb(PIC_SLAVE_DATA, slave_offset);
    io_wait();

    // ============================================
    // ICW3: 设置级联关系
    // ============================================
    // ICW3 指定主 PIC 和从 PIC 的连接方式
    // 主 PIC: Bit N 表示 IRQ N 连接了从 PIC
    // 从 PIC: 指定连接到主 PIC 的哪个 IRQ

    // 发送 ICW3 给主 PIC（IRQ 2 连接从 PIC）
    // 主 PIC 的 bit 2 置1，表示从 PIC 连接到 IRQ 2
    outb(PIC_MASTER_DATA, 1 << 2);
    io_wait();

    // 发送 ICW3 给从 PIC（连接到主 PIC 的 IRQ 2）
    // 从 PIC 的值为 2，表示连接到主 PIC 的 IRQ 2
    outb(PIC_SLAVE_DATA, 2);
    io_wait();

    // ============================================
    // ICW4: 设置工作模式
    // ============================================
    // ICW4 指定 PIC 的工作模式
    // Bit 0: 8086/8088 模式（必须为1）
    // Bit 1: 自动 EOI 模式（0 = 正常 EOI，1 = 自动 EOI）
    // Bit 2-3: 缓冲模式
    // Bit 4: 特殊完全嵌套模式

    // 发送 ICW4 给主 PIC（8086 模式，正常 EOI）
    outb(PIC_MASTER_DATA, PIC_ICW4_8086);
    io_wait();

    // 发送 ICW4 给从 PIC（8086 模式，正常 EOI）
    outb(PIC_SLAVE_DATA, PIC_ICW4_8086);
    io_wait();

    // ============================================
    // 初始化完成，设置中断掩码
    // ============================================
    // 初始化后，所有中断都被禁用（掩码全为1）
    // 我们需要根据需要启用特定的中断

    // 默认禁用所有中断，稍后根据需要启用
    outb(PIC_MASTER_DATA, 0xFF);  // 禁用所有主 PIC 中断
    outb(PIC_SLAVE_DATA, 0xFF);   // 禁用所有从 PIC 中断

    serial_puts("[PIC] 8259A PIC initialized successfully!\n");
    serial_printf("[PIC] Master PIC offset: 0x%x, Slave PIC offset: 0x%x\n",
                  master_offset, slave_offset);
}

// ============================================
// 发送 EOI (End of Interrupt) 给指定的 PIC
// ============================================
void pic_send_eoi(uint8_t irq) {
    // 如果 IRQ 来自从 PIC (IRQ 8-15)，需要同时发送 EOI 给主 PIC
    // 因为从 PIC 通过主 PIC 的 IRQ 2 级联连接

    if (irq >= 8) {
        // IRQ 来自从 PIC，发送 EOI 给从 PIC
        outb(PIC_SLAVE_COMMAND, PIC_EOI);
    }

    // 无论 IRQ 来自主 PIC 还是从 PIC，都需要发送 EOI 给主 PIC
    outb(PIC_MASTER_COMMAND, PIC_EOI);
}

// ============================================
// 设置中断掩码（禁用/启用指定的 IRQ）
// ============================================
void pic_set_mask(uint8_t irq, uint8_t masked) {
    uint16_t port;

    // 根据 IRQ 号选择对应的 PIC 端口
    if (irq < 8) {
        // IRQ 0-7: 主 PIC
        port = PIC_MASTER_DATA;
    } else {
        // IRQ 8-15: 从 PIC
        port = PIC_SLAVE_DATA;
        irq -= 8;  // 转换为从 PIC 的 IRQ 号 (0-7)
    }

    // 读取当前掩码
    uint8_t mask = inb(port);

    if (masked) {
        // 禁用中断：设置对应的位为1
        mask |= (1 << irq);
    } else {
        // 启用中断：清除对应的位为0
        mask &= ~(1 << irq);
    }

    // 写入新的掩码
    outb(port, mask);
}

// ============================================
// 读取中断掩码
// ============================================
uint16_t pic_get_mask(void) {
    // 读取主 PIC 掩码（低8位）和从 PIC 掩码（高8位）
    uint16_t master_mask = inb(PIC_MASTER_DATA);
    uint16_t slave_mask = inb(PIC_SLAVE_DATA);

    return (slave_mask << 8) | master_mask;
}

// ============================================
// 读取中断请求寄存器 (IRR)
// ============================================
uint16_t pic_get_irr(void) {
    // 发送读取 IRR 命令
    outb(PIC_MASTER_COMMAND, 0x0A);  // 读取主 PIC IRR
    outb(PIC_SLAVE_COMMAND, 0x0A);   // 读取从 PIC IRR

    // 读取 IRR 值
    uint16_t master_irr = inb(PIC_MASTER_COMMAND);
    uint16_t slave_irr = inb(PIC_SLAVE_COMMAND);

    return (slave_irr << 8) | master_irr;
}

// ============================================
// 读取中断服务寄存器 (ISR)
// ============================================
uint16_t pic_get_isr(void) {
    // 发送读取 ISR 命令
    outb(PIC_MASTER_COMMAND, 0x0B);  // 读取主 PIC ISR
    outb(PIC_SLAVE_COMMAND, 0x0B);   // 读取从 PIC ISR

    // 读取 ISR 值
    uint16_t master_isr = inb(PIC_MASTER_COMMAND);
    uint16_t slave_isr = inb(PIC_SLAVE_COMMAND);

    return (slave_isr << 8) | master_isr;
}

// ============================================
// 禁用所有中断
// ============================================
void pic_disable_all(void) {
    outb(PIC_MASTER_DATA, 0xFF);  // 禁用所有主 PIC 中断
    outb(PIC_SLAVE_DATA, 0xFF);   // 禁用所有从 PIC 中断
}

// ============================================
// 启用所有中断
// ============================================
void pic_enable_all(void) {
    outb(PIC_MASTER_DATA, 0x00);  // 启用所有主 PIC 中断
    outb(PIC_SLAVE_DATA, 0x00);   // 启用所有从 PIC 中断
}
