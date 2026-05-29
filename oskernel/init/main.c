//
// Created by ziya on 22-6-23.
//

#include "../kernel/drivers/serial.h"
#include "../kernel/mm/mem_detect.h"
#include "../kernel/mm/paging.h"

void kernel_main(void) {
    // 初始化串口
    serial_init();

    serial_putline("\n========================================");
    serial_putline("  OS Kernel Started!");
    serial_putline("========================================\n");

    // 打印内存映射表
    print_memory_map();
    
    // 初始化物理内存管理器
    pmm_init();
    
    // 测试内存分配
    serial_putline("\n========== Memory Allocation Test ==========");
    
    void* page1 = pmm_alloc_page();
    serial_printf("Allocated page 1 at: 0x%x\n", page1);
    
    void* page2 = pmm_alloc_page();
    serial_printf("Allocated page 2 at: 0x%x\n", page2);
    
    void* page3 = pmm_alloc_page();
    serial_printf("Allocated page 3 at: 0x%x\n", page3);
    
    serial_printf("Free pages: %d\n", pmm_get_free_pages());
    
    // 释放内存
    pmm_free_page(page2);
    serial_printf("Freed page 2\n");
    serial_printf("Free pages after free: %d\n", pmm_get_free_pages());
    
    // 再次分配
    void* page4 = pmm_alloc_page();
    serial_printf("Allocated page 4 at: 0x%x\n", page4);
    serial_printf("Free pages: %d\n", pmm_get_free_pages());
    
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

    // 保留VGA输出作为备用显示
    char* video = (char*)0xb8000;
    char* msg = "Hello OS! Memory OK!";
    int i = 0;

    while (msg[i] != '\0') {
        video[i * 2] = msg[i];      // 字符
        video[i * 2 + 1] = 0x0A;    // 属性（白字黑底）
        i++;
    }
    
    serial_putline("Kernel initialization completed!");
    
    // 无限循环，防止内核退出
    while (1) {
        __asm__ volatile("hlt");
    }
}
