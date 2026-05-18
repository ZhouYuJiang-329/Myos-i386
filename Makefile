# 编译器
NASM = nasm
QEMU = qemu-system-x86_64
BOCHS = bochs
BXIMAGE = bximage

# 目标文件
TARGET = boot.bin
SRC = boot.asm
HD_IMG = hd.img

# 默认目标
all: $(TARGET)

# 编译引导扇区
$(TARGET): $(SRC)
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

# 使用 bximage 生成硬盘镜像 (16 扇区 = 8KB)
$(HD_IMG): $(TARGET)
	@echo "Creating hard disk image with bximage..."
	$(BXIMAGE) -q -hd=16 -func=create -sectsize=512 -imgmode=flat $@
	dd if=$(TARGET) of=$@ conv=notrunc

# 生成硬盘镜像 (别名)
hd: $(HD_IMG)

# 清理生成的文件
clean:
	rm -f $(TARGET) $(HD_IMG) bochs.log

.PHONY: all run debug bochs bochs-debug hd clean
