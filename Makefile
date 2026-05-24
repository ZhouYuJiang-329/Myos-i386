# 编译器
NASM = nasm
GCC = gcc
LD = ld
QEMU = qemu-system-x86_64
BOCHS = bochs
BXIMAGE = bximage

# 目录
BOOT_DIR = oskernel/boot
INIT_DIR = init
KERNEL_DIR = oskernel

# 源文件
ASM_SRCS = $(wildcard $(BOOT_DIR)/*.asm)
C_SRCS = $(wildcard $(INIT_DIR)/*.c)

# 目标文件
BOOT_BIN = $(BOOT_DIR)/boot.bin
SETUP_BIN = $(BOOT_DIR)/setup.bin
HEAD_O = $(BOOT_DIR)/head.o
MAIN_O = $(INIT_DIR)/main.o
KERNEL_BIN = $(KERNEL_DIR)/kernel.bin

# 硬盘镜像
HD_IMG = hd.img

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
	$(GCC) -m32 -ffreestanding -fno-pic -fno-stack-protector -c $< -o $@

# ============================================
# 内核链接
# ============================================
$(KERNEL_BIN): $(HEAD_O) $(MAIN_O)
	$(LD) -m elf_i386 -Ttext 0x1200 --oformat binary -o $@ $^

# ============================================
# 运行
# ============================================
run: $(HD_IMG)
	$(QEMU) -hda $(HD_IMG)  -m 512

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
	rm -f $(BOOT_BIN) $(SETUP_BIN) $(HEAD_O) $(MAIN_O) $(KERNEL_BIN) $(HD_IMG) bochs.log

.PHONY: all run debug bochs bochs-debug hd clean
