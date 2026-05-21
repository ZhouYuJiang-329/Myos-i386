# 编译器
NASM = nasm
QEMU = qemu-system-x86_64
BOCHS = bochs

# 目录
BOOT_DIR = oskernel/boot

# 源文件（自动查找所有 .asm 文件）
ASM_SRCS = $(wildcard $(BOOT_DIR)/*.asm)

# 目标文件（.asm → .bin）
BIN_TARGETS = $(ASM_SRCS:.asm=.bin)

# 硬盘镜像
HD_IMG = hd.img

# 默认目标：编译所有 .bin 文件
all: $(BIN_TARGETS)

# 通用规则：任意 .asm 编译为 .bin
$(BOOT_DIR)/%.bin: $(BOOT_DIR)/%.asm
	$(NASM) -f bin $< -o $@

# 运行 (使用 QEMU + 硬盘镜像)
run: $(HD_IMG)
	$(QEMU) -hda $(HD_IMG)

# 调试模式运行 (QEMU + 硬盘镜像)
debug: $(HD_IMG)
	$(QEMU) -hda $(HD_IMG) -s -S

# 运行 (使用 Bochs + 硬盘镜像)
bochs: $(HD_IMG)
	$(BOCHS) -f bochsrc -q

# 调试模式运行 (Bochs + 硬盘镜像)
bochs-debug: $(HD_IMG)
	$(BOCHS) -f bochsrc

# 生成硬盘镜像
# boot.bin → 第1扇区 (偏移 0)
# setup.bin → 第2扇区 (偏移 512)
$(HD_IMG): $(BIN_TARGETS)
	@echo "Creating hard disk image..."
	dd if=/dev/zero of=$@ bs=512 count=2880
	dd if=$(BOOT_DIR)/boot.bin of=$@ bs=512 count=1 conv=notrunc
	dd if=$(BOOT_DIR)/setup.bin of=$@ bs=512 seek=1 conv=notrunc

# 生成硬盘镜像 (别名)
hd: $(HD_IMG)

# 清理生成的文件
clean:
	rm -f $(BIN_TARGETS) $(HD_IMG) bochs.log

.PHONY: all run debug bochs bochs-debug hd clean
