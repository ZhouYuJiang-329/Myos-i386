# 编译器
NASM = nasm
GCC = gcc
LD = ld
QEMU = qemu-system-x86_64
BOCHS = bochs
BXIMAGE = bximage

# C 编译器标志（32位保护模式，无标准库，无栈保护）
CFLAGS = -m32 -ffreestanding -fno-pic -fno-stack-protector \
         -I. \
         -Ioskernel/init \
         -Ioskernel/kernel \
         -Ioskernel/kernel/drivers \
         -Ioskernel/kernel/mm \
         -Ioskernel/kernel/idt \
         -Ioskernel/kernel/thread \
         -Ioskernel/kernel/device \
         -Ioskernel/kernel/lib \
         -Ioskernel/kernel/lib/kernel

# 目录
BOOT_DIR = oskernel/boot
INIT_DIR = oskernel/init
KERNEL_DIR = oskernel
DRIVERS_DIR = oskernel/kernel/drivers
DEVICE_DIR = oskernel/kernel/device
MM_DIR = oskernel/kernel/mm
IDT_DIR = oskernel/kernel/idt
BUILD_DIR = build
LIB_DIR = oskernel/kernel/lib
THREAD_DIR = oskernel/kernel/thread

# 创建 build 目录（如果不存在）
$(shell mkdir -p $(BUILD_DIR))



# 源文件
ASM_SRCS = $(wildcard $(BOOT_DIR)/*.asm)
C_SRCS = $(wildcard $(INIT_DIR)/*.c)

# 目标文件（全部放在 build 目录下）
BOOT_BIN = $(BUILD_DIR)/boot.bin
SETUP_BIN = $(BUILD_DIR)/setup.bin
HEAD_O = $(BUILD_DIR)/head.o
MAIN_O = $(BUILD_DIR)/main.o
SERIAL_O = $(BUILD_DIR)/serial.o
VGA_O = $(BUILD_DIR)/vga.o
MEM_DETECT_O = $(BUILD_DIR)/mem_detect.o
PAGING_O = $(BUILD_DIR)/paging.o
IDT_O = $(BUILD_DIR)/idt.o
ISR_O = $(BUILD_DIR)/isr.o
PIC_O = $(BUILD_DIR)/pic.o
STRING_O = $(BUILD_DIR)/string.o
BITMAP_O = $(BUILD_DIR)/bitmap.o
MEMORY_O = $(BUILD_DIR)/memory.o
INTERRUPT_O = $(BUILD_DIR)/interrupt.o
THREAD_O = $(BUILD_DIR)/thread.o
SWITCH_O = $(BUILD_DIR)/switch.o
LIST_O = $(BUILD_DIR)/list.o
TIMER_O = $(BUILD_DIR)/timer.o
SYNC_O = $(BUILD_DIR)/sync.o
KERNEL_BIN = $(BUILD_DIR)/kernel.bin


# 内核对象文件集合
OBJS = $(HEAD_O) $(MAIN_O) $(SERIAL_O) $(VGA_O) $(MEM_DETECT_O) \
		$(PAGING_O) $(IDT_O) $(ISR_O) $(PIC_O) $(STRING_O) \
		$(BITMAP_O) $(MEMORY_O) $(INTERRUPT_O) $(THREAD_O) $(SWITCH_O) \
		$(LIST_O) $(TIMER_O) $(SYNC_O)

# 硬盘镜像（也放在 build 目录下）
HD_IMG = $(BUILD_DIR)/hd.img

# 默认目标
all: $(HD_IMG)

# ============================================
# Boot 扇区编译（实模式，纯二进制）
# ============================================
$(BOOT_BIN): $(BOOT_DIR)/boot.asm
	$(NASM) -f bin $< -o $@

# Setup 编译（实模式，占 2 个扇区）
$(SETUP_BIN): $(BOOT_DIR)/setup.asm
	$(NASM) -f bin $< -o $@

# ============================================
# 内核对象文件编译（32 位保护模式）
# ============================================
# head.asm 编译为 ELF 对象文件
$(HEAD_O): $(BOOT_DIR)/head.asm
	$(NASM) -f elf32 $< -o $@

# main.c 编译为对象文件（32 位，无标准库，无栈保护）
$(MAIN_O): $(INIT_DIR)/main.c
	$(GCC) $(CFLAGS) -c $< -o $@

# serial.c 编译为对象文件
$(SERIAL_O): $(DRIVERS_DIR)/serial.c
	$(GCC) $(CFLAGS) -c $< -o $@

# vga.c 编译为对象文件
$(VGA_O): $(DRIVERS_DIR)/vga.c
	$(GCC) $(CFLAGS) -c $< -o $@

# mem_detect.c 编译为对象文件
$(MEM_DETECT_O): $(MM_DIR)/mem_detect.c
	$(GCC) $(CFLAGS) -c $< -o $@

# paging.c 编译为对象文件
$(PAGING_O): $(MM_DIR)/paging.c
	$(GCC) $(CFLAGS) -c $< -o $@

# idt.c 编译为对象文件
$(IDT_O): $(IDT_DIR)/idt.c
	$(GCC) $(CFLAGS) -c $< -o $@

# isr.asm 编译为 ELF 对象文件
$(ISR_O): $(IDT_DIR)/isr.asm
	$(NASM) -f elf32 $< -o $@

# pic.c 编译为对象文件
$(PIC_O): $(IDT_DIR)/pic.c
	$(GCC) $(CFLAGS) -c $< -o $@

# interrupt.c 编译为对象文件
$(INTERRUPT_O): $(IDT_DIR)/interrupt.c
	$(GCC) $(CFLAGS) -c $< -o $@

# timer.c 编译为对象文件
$(TIMER_O): $(DEVICE_DIR)/timer.c
	$(GCC) $(CFLAGS) -c $< -o $@

# string.c 编译为对象文件
$(STRING_O): $(LIB_DIR)/string.c
	$(GCC) $(CFLAGS) -c $< -o $@

# bitmap.c 编译为对象文件
$(BITMAP_O): $(LIB_DIR)/kernel/bitmap.c
	$(GCC) $(CFLAGS) -c $< -o $@

# memory.c 编译为对象文件
$(MEMORY_O): $(MM_DIR)/memory.c
	$(GCC) $(CFLAGS) -c $< -o $@

# thread.c 编译为对象文件
$(THREAD_O): $(THREAD_DIR)/thread.c
	$(GCC) $(CFLAGS) -c $< -o $@

# list.c 编译为对象文件
$(LIST_O): $(LIB_DIR)/kernel/list.c
	$(GCC) $(CFLAGS) -c $< -o $@

# switch.S 编译为对象文件
$(SWITCH_O): $(THREAD_DIR)/switch.S
	$(NASM) -f elf32 $< -o $@

# sync.c 编译为对象文件
$(SYNC_O): $(THREAD_DIR)/sync.c
	$(GCC) $(CFLAGS) -c $< -o $@


# ============================================
# 内核链接
# ============================================
$(KERNEL_BIN): $(OBJS)
	$(LD) -m elf_i386 -Ttext 0x1200  --oformat binary -o $@ $^ -Map $(BUILD_DIR)/kernel.map

# ============================================
# 运行
# ============================================
run: $(HD_IMG)
	$(QEMU) -hda $(HD_IMG) -m 512 -serial stdio

debug: $(HD_IMG)
	$(QEMU) -hda $(HD_IMG) -s -S

bochs: $(HD_IMG)
	$(BOCHS) -f bochsrc -q

bochs-debug: $(HD_IMG)
	$(BOCHS) -f bochsrc

# ============================================
# 生成硬盘镜像
# 布局：
#   - 第 1 扇区 (偏移 0):       boot.bin
#   - 第 2-3 扇区 (偏移 512):   setup.bin (2 个扇区)
#   - 第 4 扇区起 (偏移 1536):  kernel.bin
# ============================================
$(HD_IMG): $(BOOT_BIN) $(SETUP_BIN) $(KERNEL_BIN)
	@echo "Creating hard disk image..."
	$(BXIMAGE) -q -hd=32 -func=create -sectsize=512 -imgmode=flat $@
	dd if=$(BOOT_BIN) of=$@ bs=512 seek=0 count=1 conv=notrunc
	dd if=$(SETUP_BIN) of=$@ bs=512 seek=1 count=2 conv=notrunc
	dd if=$(KERNEL_BIN) of=$@ bs=512 seek=3 count=60 conv=notrunc

# 生成硬盘镜像 (别名)
hd: $(HD_IMG)

# 清理生成的文件
clean:
	rm -rf $(BUILD_DIR) bochs.log

.PHONY: all run debug bochs bochs-debug hd clean
