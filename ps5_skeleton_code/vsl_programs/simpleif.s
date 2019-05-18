.section .rodata
intout: .string "%ld "
strout: .string "%s "
errout: .string "Wrong number of arguments"
STR0:	.string "Equal!"
.section .data
.globl main
.section .text
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $1, %rdi
	cmpq	$7,%rdi
	jne ABORT
	cmpq $0, %rdi
	jz SKIP_ARGS
	movq %rdi, %rcx
	addq $56, %rsi
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
	popq	%rdi
	popq	%rsi
	popq	%rdx
	popq	%rcx
	popq	%r8
	popq	%r9
SKIP_ARGS:
	call	_dingdong
	jmp END
ABORT:
	movq $errout, %rdi
	call puts
END:
	movq %rax, %rdi
	call exit
_dingdong:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rdi
	pushq %rsi
	pushq %rdx
	pushq %rcx
	pushq %r8
	pushq %r9
	subq $8, %rsp
	movq $42, %rax
	movq %rax, -56(%rbp)
	movq -8(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq -16(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq -24(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq -32(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq -40(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq -48(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq 16(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	movq -56(%rbp), %rax
	movq $42, %rbx
	cmpq %rax, %rbx
jne	 ELSE0
	movq $STR0, %rsi
	movq $strout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	movq $43, %rax
	movq %rax, -56(%rbp)
	jmp END_IF0
ELSE0:
	movq $44, %rax
	movq %rax, -56(%rbp)
END_IF0:
WHILE0:
	movq -56(%rbp), %rax
	movq $0, %rbx
	cmpq %rax, %rbx
jge	 DONE0
	movq -56(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	movq -56(%rbp), %rax
	pushq %rax
	movq $1, %rax
	subq %rax, (%rsp)
	popq %rax
	movq %rax, -56(%rbp)
	jmp WHILE0
DONE0:
	movq -56(%rbp), %rax
	leave
	ret
