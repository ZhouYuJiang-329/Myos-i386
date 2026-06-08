//
// Created by ziya on 22-6-23.
//

#include "../kernel/drivers/serial.h"
#include "../kernel/drivers/vga.h"
#include "../kernel/mm/mem_detect.h"
#include "../kernel/mm/paging.h"
#include "../kernel/idt/idt.h"
#include "../kernel/idt/pic.h"
#include "../kernel/idt/interrupt.h"
#include "../kernel/mm/memory.h"
#include "../kernel/thread/thread.h"
#include "../kernel/device/timer.h"

#

void k_thread_a(void *arg);
void k_thread_b(void *arg);
void kernel_main(void) {
    // 初始化串口
    serial_init();

    // 初始化 VGA 显卡
    vga_init();

    serial_putline("\n========================================");
    serial_putline("  OS Kernel Started!");
    serial_putline("========================================\n");

    // 打印内存映射表
    print_memory_map();
    
    // 初始化物理内存管理器
    mem_init();

    // 初始化中断处理框架
    serial_putline("\n========== Interrupt Initialization ==========");
    interrupt_init();

    // 初始化 8259A PIC
    // 主 PIC 中断向量偏移: 0x20 (ISR 32-39)
    // 从 PIC 中断向量偏移: 0x28 (ISR 40-47)
    pic_init(0x20, 0x28);

    // 初始化 IDT
    idt_init();

    // 启用键盘中断 (IRQ 1)
    // 键盘中断会映射到 ISR 33
    pic_set_mask(1, 0);  // 0 = 启用

    // 启用定时器中断 (IRQ 0)
    // 定时器中断会映射到 ISR 32
    pic_set_mask(0, 0);  // 0 = 启用
   

  
    

    serial_puts("Interrupts enabled!\n");
    serial_putline("============================================\n");

    // 验证分页（读取CR0）
    uint32_t cr0;
    __asm__ __volatile__("mov %%cr0, %0" : "=r"(cr0));
    serial_printf("CR0: 0x%x\n", cr0);

    if (cr0 & 0x80000000) {
        serial_puts("Paging enabled successfully!\n");
    } else {
        serial_puts("ERROR: Paging not enabled!\n");
    }

    // 使用 VGA 驱动输出测试信息
    vga_set_color(VGA_GREEN, VGA_BLACK);
    vga_puts("Hello OS! VGA OK!\n");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_puts("Hello OS! VGA OK!\n");
   
    serial_putline("Kernel initialization completed!");
    vga_puts("Kernel initialization completed!\n");

    // 初始化线程环境（必须在创建线程之前）
    thread_init();

    // 创建两个测试线程
    thread_start("k_thread_a", 8, k_thread_a, "theada ");
    thread_start("k_thread_b", 31, k_thread_b, "threadb ");

    // 初始化定时器（会注册定时器中断处理函数）
    timer_init();

    // 启用所有中断
    __asm__ __volatile__("sti");
    // 无限循环，防止内核退出
     
    while (1) {
        serial_puts("main ");
    }
}

void k_thread_a(void *arg) {
    char *message = (char*)arg;
    while(1){
        serial_puts(message);
        // 主动让出 CPU，等待下一次调度
        // schedule();
    }

}

void k_thread_b(void *arg) {
    char *message = (char*)arg;
    while(1){
        serial_puts(message);
        // 主动让出 CPU，等待下一次调度
        // schedule();
    }
}