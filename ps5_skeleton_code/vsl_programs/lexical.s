.section .rodata
intout: .string "%ld "
strout: .string "%s "
errout: .string "Wrong number of arguments"
STR0:	.string "Hello, world!"
STR1:	.string ""
STR2:	.string "Hello, \"world\"!"
STR3:	.string "+"
STR4:	.string ":="
STR5:	.string "-"
STR6:	.string ":="
STR7:	.string "+ (-"
STR8:	.string ") :="
STR9:	.string "*"
STR10:	.string ":="
STR11:	.string "/"
STR12:	.string ":="
STR13:	.string "Skip..."
.section .data
.globl main
.section .text
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $1, %rdi
	cmpq	$0,%rdi
	jne ABORT
	cmpq $0, %rdi
	jz SKIP_ARGS
	movq %rdi, %rcx
	addq $0, %rsi
PARSE_ARGV:
	pushq %rcx
	pushq %rsi
	movq (%rsi), %rdi
	movq $0, %rsi
	movq $10, %rdx
	call strtol
	popq %rsi
	popq %rcx
	pushq %rax
	subq $8, %rsi
	loop PARSE_ARGV
SKIP_ARGS:
	call	_main
	jmp END
ABORT:
	movq $errout, %rdi
	call puts
END:
	movq %rax, %rdi
	call exit
_main:
	pushq %rbp
	movq %rsp, %rbp
	subq $24, %rsp
	pushq	$0
	movq $STR0, %rsi
	movq $strout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	movq $STR1, %rsi
	movq $strout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	movq $STR2, %rsi
	movq $strout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	movq $10, %rax
	movq %rax, -16(%rbp)
	movq $2, %rax
	movq %rax, -24(%rbp)
	movq -16(%rbp), %rax
	pushq %rax
	movq -24(%rbp), %rax
	addq %rax, (%rsp)
	popq %rax
	movq %rax, -8(%rbp)
	movq -16(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $STR3, %rsi
	movq $strout, %rdi
	call printf
	movq -24(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $STR4, %rsi
	movq $strout, %rdi
	call printf
	movq -8(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	movq -16(%rbp), %rax
	pushq %rax
	movq -24(%rbp), %rax
	subq %rax, (%rsp)
	popq %rax
	movq %rax, -8(%rbp)
	movq -16(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $STR5, %rsi
	movq $strout, %rdi
	call printf
	movq -24(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $STR6, %rsi
	movq $strout, %rdi
	call printf
	movq -8(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	movq -16(%rbp), %rax
	pushq %rax
	movq -24(%rbp), %rax
	negq	%%rax
	addq %rax, (%rsp)
	popq %rax
	movq %rax, -8(%rbp)
	movq -16(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $STR7, %rsi
	movq $strout, %rdi
	call printf
	movq -24(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $STR8, %rsi
	movq $strout, %rdi
	call printf
	movq -8(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	pushq %rdx
	movq -24(%rbp), %rax
	pushq %rax
	movq -16(%rbp), %rax
	mulq (%rsp)
	popq %rdx
	popq %rdx
	movq %rax, -8(%rbp)
	movq -16(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $STR9, %rsi
	movq $strout, %rdi
	call printf
	movq -24(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $STR10, %rsi
	movq $strout, %rdi
	call printf
	movq -8(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	pushq %rdx
	movq -24(%rbp), %rax
	pushq %rax
	movq -16(%rbp), %rax
	cqo
	idivq (%rsp)
	popq %rdx
	popq rdx
	movq %rax, -8(%rbp)
	movq -16(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $STR11, %rsi
	movq $strout, %rdi
	call printf
	movq -24(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $STR12, %rsi
	movq $strout, %rdi
	call printf
	movq -8(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	movq -8(%rbp), %rax
	movq $0, %rbx
	cmpq %rax, %rbx
jge	 END_IF0
	movq -8(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
END_IF0:
WHILE0:
	movq -8(%rbp), %rax
	movq $0, %rbx
	cmpq %rax, %rbx
jge	 DONE0
	movq -8(%rbp), %rax
	pushq %rax
	movq $3, %rax
	subq %rax, (%rsp)
	popq %rax
	movq $0, %rbx
	cmpq %rax, %rbx
jge	 ELSE1
	movq -8(%rbp), %rax
	pushq %rax
	movq $1, %rax
	subq %rax, (%rsp)
	popq %rax
	movq %rax, -8(%rbp)
	jmp END_IF1
ELSE1:
	movq -8(%rbp), %rax
	pushq %rax
	movq $1, %rax
	subq %rax, (%rsp)
	popq %rax
	movq %rax, -8(%rbp)
	movq $STR13, %rsi
	movq $strout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	jmp WHILE0
END_IF1:
	movq -8(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	jmp WHILE0
DONE0:
	movq $0, %rax
	leave
	ret
