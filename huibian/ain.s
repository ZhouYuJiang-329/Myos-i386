	.file	"ain.c"
	.text

# add 函数：接收两个 int 参数，返回它们的和
# 对应 C 代码: int add(int a, int b) { int c = 10; return a + b; }
	.globl	add
	.type	add, @function
add:
.LFB0:
	.cfi_startproc
	endbr64				# CET: 启用控制流完整性保护
	pushq	%rbp			# 保存调用者的帧指针
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp		# 建立当前函数的栈帧
	.cfi_def_cfa_register 6
	movl	%edi, -20(%rbp)		# 第1个参数 a 从 edi 寄存器存入栈 [rbp-20]
	movl	%esi, -24(%rbp)		# 第2个参数 b 从 esi 寄存器存入栈 [rbp-24]
	movl	$10, -4(%rbp)		# 局部变量 c = 10（存入 [rbp-4]）
	movl	-20(%rbp), %edx		# 将参数 a 从栈加载到 edx 寄存器
	movl	-24(%rbp), %eax		# 将参数 b 从栈加载到 eax 寄存器
	addl	%edx, %eax		# eax = edx + eax，即 a + b，结果存入 eax（返回值）
	popq	%rbp			# 恢复调用者的帧指针
	.cfi_def_cfa 7, 8
	ret				# 返回调用者，结果在 eax 中
	.cfi_endproc
.LFE0:
	.size	add, .-add

# main 函数：调用 add(1, 2) 并返回 0
	.globl	main
	.type	main, @function
main:
.LFB1:
	.cfi_startproc
	endbr64				# CET: 启用控制流完整性保护
	pushq	%rbp			# 保存调用者的帧指针
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp		# 建立当前函数的栈帧
	.cfi_def_cfa_register 6
	movl	$2, %esi		# 第2个参数 b = 2，存入 esi 寄存器
	movl	$1, %edi		# 第1个参数 a = 1，存入 edi 寄存器
	call	add			# 调用 add(1, 2)，返回值存入 eax
	movl	$0, %eax		# 设置返回值 eax = 0（return 0）
	popq	%rbp			# 恢复调用者的帧指针
	.cfi_def_cfa 7, 8
	ret				# 返回，程序结束
	.cfi_endproc
.LFE1:
	.size	main, .-main

	.ident	"GCC: (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	1f - 0f
	.long	4f - 1f
	.long	5
0:
	.string	"GNU"
1:
	.align 8
	.long	0xc0000002
	.long	3f - 2f
2:
	.long	0x3
3:
	.align 8
4:
