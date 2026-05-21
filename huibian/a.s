	.file	"a.c"
	.text

# add 函数：设置局部变量并返回
	.globl	add
	.type	add, @function
add:
.LFB0:
	.cfi_startproc
	endbr64				# CET: 启用控制流完整性，防止跳转攻击
	pushq	%rbp			# 保存调用者的帧指针
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp		# 建立当前函数的栈帧
	.cfi_def_cfa_register 6
	movl	$10, -4(%rbp)		# 局部变量 a = 10
	nop				# 空操作（编译器填充对齐）
	popq	%rbp			# 恢复调用者的帧指针
	.cfi_def_cfa 7, 8
	ret				# 返回调用者
	.cfi_endproc
.LFE0:
	.size	add, .-add

# main 函数：调用 add 并退出
	.globl	main
	.type	main, @function
main:
.LFB1:
	.cfi_startproc
	endbr64				# CET: 启用控制流完整性
	pushq	%rbp			# 保存调用者的帧指针
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp		# 建立当前函数的栈帧
	.cfi_def_cfa_register 6
	movl	$0, %eax		# 设置返回值寄存器 eax = 0（准备调用）
	call	add			# 调用 add 函数
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
