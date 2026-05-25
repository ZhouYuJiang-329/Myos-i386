//
// Created by ziya on 22-6-23.
//

#include "../kernel/drivers/serial.h"

void kernel_main(void) {
    // 初始化串口
    serial_init();
    
    // 测试串口输出
    serial_putline("========================================");
    serial_putline("  Hello from Serial Port!");
    serial_putline("========================================");
    
    // 测试格式化输出
    serial_printf("String: %s\n", "Test String");
    serial_printf("Decimal: %d\n", 42);
    serial_printf("Hex: %x\n", 0x1234);
    serial_printf("Char: %c\n", 'A');
    serial_printf("Mixed: %s = %d, addr = %x\n", "Value", 100, 0xB8000);
    
    serial_putline("========================================");
    serial_putline("  Serial output test completed!");
    serial_putline("========================================");
     serial_putline("zyj6666");
    
    // 保留VGA输出作为备用显示
    char* video = (char*)0xb8000;
    char* msg = "Hello OS!";
    int i = 0;

    while (msg[i] != '\0') {
        video[i * 2] = msg[i];      // 字符
        video[i * 2 + 1] = 0x0A;    // 属性（白字黑底）
        i++;
    }
    
    // 无限循环，防止内核退出
    while (1) {
        __asm__ volatile("hlt");
    }
}
