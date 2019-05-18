.section .rodata
intout: .string "%ld "
strout: .string "%s "
errout: .string "Wrong number of arguments"
STR0:	.string "The square root of"
STR1:	.string "is"
.section .data
.globl main
.section .text
main:
	pushq %rbp
	movq %rsp, %rbp
	subq $1, %rdi
	cmpq	$1,%rdi
	jne ABORT
	cmpq $0, %rdi
	jz SKIP_ARGS
	movq %rdi, %rcx
	addq $8, %rsi
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
SKIP_ARGS:
	call	_newton
	jmp END
ABORT:
	movq $errout, %rdi
	call puts
END:
	movq %rax, %rdi
	call exit
_newton:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rdi
	subq $0, %rsp
	pushq	$0
	movq $STR0, %rsi
	movq $strout, %rdi
	call printf
	movq -8(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $STR1, %rsi
	movq $strout, %rdi
	call printf
	movq -8(%rbp), %rdi
	movq $1, %rsi
	call _improve
	movq %rax, %rsi
	movq $intout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	movq $0, %rax
	leave
	ret
_improve:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rdi
	pushq %rsi
	subq $8, %rsp
	pushq	$0
	movq -16(%rbp), %rax
	pushq %rax
	pushq %rdx
	pushq %rdx
	movq -16(%rbp), %rax
	pushq %rax
	movq $2, %rax
	mulq (%rsp)
	popq %rdx
	popq %rdx
	pushq %rax
	pushq %rdx
	movq -16(%rbp), %rax
	pushq %rax
	movq -16(%rbp), %rax
	mulq (%rsp)
	popq %rdx
	popq %rdx
	pushq %rax
	movq -8(%rbp), %rax
	subq %rax, (%rsp)
	popq %rax
	cqo
	idivq (%rsp)
	popq %rdx
	popq rdx
	subq %rax, (%rsp)
	popq %rax
	movq %rax, -24(%rbp)
	movq -24(%rbp), %rax
	pushq %rax
	movq -16(%rbp), %rax
	subq %rax, (%rsp)
	popq %rax
	movq $0, %rbx
	cmpq %rax, %rbx
jne	 ELSE0
	movq -24(%rbp), %rax
	pushq %rax
	movq $1, %rax
	subq %rax, (%rsp)
	popq %rax
	leave
	ret
	jmp END_IF0
ELSE0:
	movq -8(%rbp), %rdi
	movq -24(%rbp), %rsi
	call _improve
	leave
	ret
END_IF0:
