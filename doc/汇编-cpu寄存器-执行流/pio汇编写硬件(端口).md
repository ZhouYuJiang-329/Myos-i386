1.汇编读写磁盘
直接通过汇编读写硬盘，不是用 BIOS 中断，而是用 CPU 的 I/O 指令直接和硬盘控制器"对话"。这条路是驱动开发者、内核工程师和恶意软件作者才会走的路。
下面从原理到代码，逐步拆解。

1. 两种硬件接口：IDE(AHCI) 和 NVMe
现代硬盘与 CPU 通信，主要通过两种接口标准。它们的编程模型完全不同：
接口	编程方式	寄存器访问	数据读写
SATA (AHCI)	通过 I/O 端口 或 MMIO 操作控制器寄存器	in/out 指令（x86）或 MMIO 读/写	设置 DMA，让控制器自动搬数据
NVMe	完全通过 PCIe MMIO，用队列提交命令	MMIO 读写（mov 直接访问内存地址）	同样走队列，控制器自动 DMA
关键区别：BIOS 中断（int 13h）是在帮你的程序操作这些寄存器。不用 BIOS，你就得自己写。

2. x86 的关键 I/O 指令
在保护模式/长模式的 Ring 0 下，可以用以下指令直接访问 I/O 端口：
; 从端口读一个字节到 al
mov dx, 0x1F0      ; 端口地址
in  al, dx         ; 读端口

; 向端口写一个字节
mov dx, 0x1F0
mov al, 0x42
out dx, al         ; 写端口

; 读/写 16 位 (ax) 或 32 位 (eax)
in  ax, dx
out dx, eax
这些是特权指令，只能在 Ring 0 下执行。在用户态执行会触发 #GP 异常。

3. 实战：通过 PIO 读一个扇区
以最经典的 IDE 控制器（PATA/SATA 兼容模式） 为例，它使用固定的 I/O 端口。这是最底层、最古老、也是最适合教学的方式。
3.1 IDE 控制器的关键寄存器（主通道）
I/O 端口	方向	名称	用途
0x1F0	R/W	Data Register	读写扇区数据（16 位宽）
0x1F1	R	Error Register	错误码
0x1F2	R/W	Sector Count	要读写的扇区数
0x1F3	R/W	LBA Low	LBA 地址 bits 0-7
0x1F4	R/W	LBA Mid	LBA 地址 bits 8-15
0x1F5	R/W	LBA High	LBA 地址 bits 16-23
0x1F6	R/W	Drive/Head	LBA 地址 bits 24-27 + 主/从盘选择 + LBA 模式位
0x1F7	R/W	Status/Command	读状态 / 写命令
0x3F6	R/W	Control	控制寄存器
3.2 读取一个扇区（LBA 模式，PIO 方式）
以下是完整的汇编代码，从主硬盘读取 LBA 0 号扇区到内存 0x1000 处：
; =================================================
; 通过 IDE 控制器 PIO 模式读取一个扇区
; 目标：从主硬盘 LBA 0 读 512 字节到内存 0x1000
; =================================================

read_sector:
    ; --- 第一步：等待驱动器就绪 ---
.wait_ready:
    mov  dx, 0x1F7
    in   al, dx
    test al, 0x80       ; 检查 BSY 位 (bit 7)
    jnz  .wait_ready    ; BSY=1 表示忙，继续等
    test al, 0x40       ; 检查 DRDY 位 (bit 6)
    jz   .wait_ready    ; DRDY=0 表示未就绪，继续等

    ; --- 第二步：设置参数 ---

    ; 扇区计数：读 1 个扇区
    mov  dx, 0x1F2
    mov  al, 1
    out  dx, al

    ; LBA 地址 (这里读 LBA 0)
    mov  dx, 0x1F3
    mov  al, 0          ; LBA bits 0-7
    out  dx, al

    mov  dx, 0x1F4
    mov  al, 0          ; LBA bits 8-15
    out  dx, al

    mov  dx, 0x1F5
    mov  al, 0          ; LBA bits 16-23
    out  dx, al

    ; Drive/Head 寄存器：LBA 模式 + 主盘 + LBA bits 24-27
    mov  dx, 0x1F6
    mov  al, 0xE0       ; 1110_0000 = LBA模式(bit6=1) + 主盘(bit4=0) + LBA高4位=0
    out  dx, al

    ; --- 第三步：发送 READ SECTORS 命令 ---
    mov  dx, 0x1F7
    mov  al, 0x20       ; READ SECTORS (带重试)
    out  dx, al

    ; --- 第四步：等待数据就绪 ---
.wait_data:
    mov  dx, 0x1F7
    in   al, dx
    test al, 0x80       ; 等 BSY 清零
    jnz  .wait_data
    test al, 0x08       ; 检查 DRQ 位 (bit 3)，=1 表示数据就绪
    jz   .wait_data

    ; --- 第五步：读取扇区数据（256 个 16 位字 = 512 字节）---
    mov  di, 0x1000     ; 目标缓冲区地址
    mov  cx, 256         ; 512 字节 = 256 个字 (x86 是 16 位 I/O)
    mov  dx, 0x1F0      ; 数据端口  16位的
    rep  insw           ; 重复 256 次：从端口读 16 位写入 [di]，di+=2

    ret
执行 rep insw 时，CPU 会连续 256 次从 0x1F0 端口读取 2 字节，写入 0x1000 开始的内存。读完就是完整的 MBR（含分区表和 0x55AA 签名）。
3.3 写入一个扇区
写入的过程几乎对称，只是：
● 用 rep outsw 替代 rep insw
● 命令改为 0x30（WRITE SECTORS）
● 等 BSY=0 且 DRQ=1 后，立即往外写数据
; ... 设置参数（同上，改 Sector Count、LBA、Drive/Head）

    ; 发送 WRITE SECTORS 命令
    mov  dx, 0x1F7
    mov  al, 0x30
    out  dx, al

    ; 等待 DRDY + DRQ
    ; ...

    ; 写入 256 个 16 位字
    mov  si, 0x1000     ; 源数据地址
    mov  cx, 256
    mov  dx, 0x1F0
    rep  outsw          ; 重复 256 次：从 [si] 读 16 位写入端口，si+=2

4. 现代方式：AHCI 与 NVMe
上面的 PIO 方式非常简单，但非常慢——CPU 要死等每一个 16 位数据的传输。现代系统全部使用 DMA。
4.1 AHCI (SATA)
AHCI 是 SATA 控制器的标准编程接口，通过 MMIO 来操控：
● 端口寄存器：映射到 MMIO 空间
● 命令列表：在内存中构建一个 FIS（Frame Information Structure），写入命令表的物理地址
● DMA 引擎：设置 PRDT（Physical Region Descriptor Table），告诉控制器数据要放到内存的哪个物理地址
完整流程：
1. 在内存中构建 FIS（读写命令）
2. 构建 PRDT（DMA 缓冲区列表，含物理地址）
3. 把 FIS 和 PRDT 写入命令表的第 N 个槽位
4. 向 MMIO 的 PxCI 寄存器写入对应的位，硬件开始执行
5. 等中断或轮询 PxSIG 寄存器，命令完成后数据已在内存中
4.2 NVMe
NVMe 更为简洁。它完全走 PCIe，通过 SQ/CQ 队列对 来提交命令：
1. 在内存中构建一个 64 字节的 Submission Queue Entry（描述读/写、LBA、物理缓冲区地址）
2. 写入 SQ 的 Tail Doorbell 寄存器
3. 控制器从内存中读取命令，执行 DMA，完成后把 Completion Queue Entry 写入 CQ
4. 检查 CQ 的 Phase Tag 位，确认完成
NVMe 对内核最重要的意义：它使用多队列设计（每个 CPU 核心可以有自己的一对 SQ/CQ），彻底消除了单一大锁。Linux 内核的 blk-mq 多层队列架构就是在充分利用这个硬件特性。

5. 为什么内核需要这样做？
1. 没有 BIOS：内核启动后，BIOS 的 int 13h 已经不存在。内核必须自己驱动硬盘。
2. 性能极致：内核的 I/O 调度器、中断处理、DMA 引擎，都是围绕直接控制硬盘控制器设计的。
3. 这是 drivers/ata/ 和 drivers/nvme/ 的本质：内核中成千上万行的 ATA/AHCI/NVMe 驱动代码，就是把这些寄存器和命令队列封装成 struct request 的接口。

6. 一句话总结
不用 BIOS 中断，程序通过 in/out 指令直接操作 I/O 端口（AHCI）或通过 MMIO 提交命令队列（NVMe）来读写硬盘。最经典的 PIO 方式：写 LBA 地址到 0x1F3-0x1F6 端口 → 写命令到 0x1F7 → 等 DRQ → 用 rep insw/outsw 从 0x1F0 端口搬运数据。现代系统几乎全用 DMA，但 PIO 是一切硬盘驱动的起点。