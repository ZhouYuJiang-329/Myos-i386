#include "interrupt.h"
#include "pic.h"
#include "../drivers/serial.h"
#include <stddef.h>

#define EFLAGS_IF   0x00000200       // eflags寄存器中的if位为1
#define GET_EFLAGS(EFLAG_VAR) asm volatile("pushfl; popl %0" : "=g" (EFLAG_VAR))
// ============================================
// 中断处理函数指针数组
// ============================================
// 每个中断向量可以注册一个自定义处理函数
// 如果为 NULL，则使用默认处理函数

static interrupt_handler_t interrupt_handlers[256] = {0};

// ============================================
// CPU 异常名称表
// ============================================
// 用于调试输出，显示中断名称

static const char* exception_names[] = {
    "Division Error",           // 0
    "Debug Exception",          // 1
    "Non-Maskable Interrupt",   // 2
    "Breakpoint",               // 3
    "Overflow",                 // 4
    "Bound Range Exceeded",     // 5
    "Invalid Opcode",           // 6
    "Device Not Available",     // 7
    "Double Fault",             // 8
    "Coprocessor Segment Overrun", // 9
    "Invalid TSS",              // 10
    "Segment Not Present",      // 11
    "Stack-Segment Fault",      // 12
    "General Protection Fault", // 13
    "Page Fault",               // 14
    "Reserved",                 // 15
    "x87 Floating-Point Exception", // 16
    "Alignment Check",          // 17
    "Machine Check",            // 18
    "SIMD Floating-Point Exception", // 19
    "Reserved",                 // 20
    "Reserved",                 // 21
    "Reserved",                 // 22
    "Reserved",                 // 23
    "Reserved",                 // 24
    "Reserved",                 // 25
    "Reserved",                 // 26
    "Reserved",                 // 27
    "Reserved",                 // 28
    "Reserved",                 // 29
    "Reserved",                 // 30
    "Reserved",                 // 31
};

// 硬件中断名称表
static const char* irq_names[] = {
    "Timer",                    // IRQ 0
    "Keyboard",                 // IRQ 1
    "Cascade",                  // IRQ 2
    "COM2",                     // IRQ 3
    "COM1",                     // IRQ 4
    "LPT2",                     // IRQ 5
    "Floppy Disk",              // IRQ 6
    "LPT1",                     // IRQ 7
    "RTC Clock",                // IRQ 8
    "ACPI",                     // IRQ 9
    "Open",                     // IRQ 10
    "Open",                     // IRQ 11
    "PS/2 Mouse",               // IRQ 12
    "FPU",                      // IRQ 13
    "Primary ATA Hard Disk",    // IRQ 14
    "Secondary ATA Hard Disk",  // IRQ 15
};

// ============================================
// 初始化中断处理框架
// ============================================
void interrupt_init(void) {
    serial_puts("[INTERRUPT] Initializing interrupt handling framework...\n");

    // 清空所有中断处理函数指针
    for (int i = 0; i < 256; i++) {
        interrupt_handlers[i] = NULL;
    }

    serial_puts("[INTERRUPT] Interrupt handling framework initialized!\n");
}

// ============================================
// 通用中断处理函数
// ============================================
// 这个函数由 ISR 汇编入口调用
// 它会处理所有中断，包括 CPU 异常和硬件中断

void interrupt_handler(struct interrupt_frame* frame, uint32_t interrupt_number) {
    // 检查是否有自定义处理函数
    if (interrupt_number < 256 && interrupt_handlers[interrupt_number] != NULL) {
        // 调用自定义处理函数
        // 注意：EOI 已经在 isr.asm 的 irq_common_stub 中发送
        interrupt_handlers[interrupt_number](frame);
        return;
    }

    // 使用默认处理函数
    if (interrupt_number < 32) {
        // ============================================
        // CPU 异常处理 (ISR 0-31)
        // ============================================
        // 这些是 CPU 自动触发的中断，通常是严重的错误

        serial_puts("\n========================================\n");
        serial_printf("[CPU EXCEPTION] %s (ISR %d)\n",
                      exception_names[interrupt_number], interrupt_number);
        serial_puts("========================================\n");

        // 打印中断栈帧信息
        serial_printf("  EIP:    0x%x\n", frame->eip);
        serial_printf("  CS:     0x%x\n", frame->cs);
        serial_printf("  EFLAGS: 0x%x\n", frame->eflags);

        // 对于有错误码的异常，尝试获取错误码
        // 错误码在栈帧中位于返回地址之前
        // 注意：这个实现假设错误码已经在栈上，但实际可能需要更复杂的处理

        // 打印寄存器状态
        // 注意：这里我们只能获取到中断发生时的栈帧信息
        // 完整的寄存器保存需要在 ISR 汇编入口中完成
        // 由于内联汇编的限制，我们暂时只打印栈帧信息

        serial_puts("========================================\n");

        // 对于严重异常，系统应该停止
        // 这里我们进入无限循环，防止系统继续运行
        serial_puts("[FATAL] System halted due to CPU exception!\n");
        while (1) {
            __asm__ __volatile__("cli; hlt");
        }

    } else if (interrupt_number >= 32 && interrupt_number < 48) {
        // ============================================
        // 硬件中断处理 (IRQ 0-15, ISR 32-47)
        // ============================================
        // 这些是 8259A PIC 产生的硬件中断

        uint8_t irq = interrupt_number - 32;

        // 对于键盘中断，需要从数据端口读取扫描码
        // 否则键盘控制器会一直保持 IRQ 线高电平，无法触发下一次中断
        if (irq == 1) {
            // 读取键盘扫描码（端口 0x60）
            uint8_t scancode = inb(0x60);
            serial_printf("[IRQ] Keyboard scancode: 0x%x\n", scancode);
            // 打印键盘中断信息
            serial_printf("[IRQ] %s (IRQ %d, ISR %d)\n",
                          irq_names[irq], irq, interrupt_number);
        } else if (irq == 0) {
            // 对于定时器中断，只打印简短信息
            // 这样可以更容易看到其他中断
            static int timer_count = 0;
            timer_count++;
            if (timer_count % 100 == 0) {
                serial_printf("[IRQ] Timer tick: %d\n", timer_count);
            }
        } else {
            // 其他中断打印完整信息
            serial_printf("[IRQ] %s (IRQ %d, ISR %d)\n",
                          irq_names[irq], irq, interrupt_number);
        }

        // 注意：EOI 已经在 isr.asm 的 irq_common_stub 中发送

    } else {
        // ============================================
        // 其他中断
        // ============================================
        serial_printf("[INTERRUPT] Unknown interrupt %d\n", interrupt_number);
    }
}

// ============================================
// 注册自定义中断处理函数
// ============================================
int interrupt_register_handler(uint8_t vector, interrupt_handler_t handler) {
    if (handler == NULL) {
        serial_printf("[INTERRUPT] Error: NULL handler for vector %d\n", vector);
        return -1;
    }

    if (interrupt_handlers[vector] != NULL) {
        serial_printf("[INTERRUPT] Warning: Overwriting existing handler for vector %d\n", vector);
    }

    interrupt_handlers[vector] = handler;
    serial_printf("[INTERRUPT] Handler registered for vector %d\n", vector);
    return 0;
}

// ============================================
// 注销中断处理函数
// ============================================
void interrupt_unregister_handler(uint8_t vector) {
    if (interrupt_handlers[vector] != NULL) {
        interrupt_handlers[vector] = NULL;
        serial_printf("[INTERRUPT] Handler unregistered for vector %d\n", vector);
    }
}

// ============================================
// 获取中断名称
// ============================================
const char* interrupt_get_name(uint32_t interrupt_number) {
    if (interrupt_number < 32) {
        return exception_names[interrupt_number];
    } else if (interrupt_number >= 32 && interrupt_number < 48) {
        return irq_names[interrupt_number - 32];
    } else {
        return "Unknown Interrupt";
    }
}

// ============================================
// 检查中断是否发生
// ============================================
int interrupt_pending(uint8_t irq) {
    if (irq >= 16) {
        return 0;
    }

    // 读取 IRR (Interrupt Request Register)
    uint16_t irr = pic_get_irr();

    // 检查对应的位
    return (irr >> irq) & 1;
}


/* 获取当前中断状态 */
enum intr_status intr_get_status() {
   uint32_t eflags = 0; 
   GET_EFLAGS(eflags);
   return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}


/* 开中断并返回开中断前的状态*/
enum intr_status intr_enable() {
   enum intr_status old_status;
   if (INTR_ON == intr_get_status()) {
      old_status = INTR_ON;
      return old_status;
   } else {
      old_status = INTR_OFF;
      asm volatile("sti");	 // 开中断,sti指令将IF位置1
      return old_status;
   }
}

/* 关中断,并且返回关中断前的状态 */
enum intr_status intr_disable() {     
   enum intr_status old_status;
   if (INTR_ON == intr_get_status()) {
      old_status = INTR_ON;
      asm volatile("cli" : : : "memory"); // 关中断,cli指令将IF位置0
                                          //cli指令不会直接影响内存。然而，从一个更大的上下文来看，禁用中断可能会影响系统状态，
                                          //这个状态可能会被存储在内存中。所以改变位填 "memory" 是为了安全起见，确保编译器在生成代码时考虑到这一点。
      return old_status;
   } else {
      old_status = INTR_OFF;
      return old_status;
   }
}

/* 将中断状态设置为status */
enum intr_status intr_set_status(enum intr_status status) {
   return status & INTR_ON ? intr_enable() : intr_disable();   //enable与disable函数会返回旧中断状态
}