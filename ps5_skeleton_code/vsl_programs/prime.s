.section .rodata
intout: .string "%ld "
strout: .string "%s "
errout: .string "Wrong number of arguments"
STR0:	.string "is a prime factor"
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
_factor:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rdi
	subq $16, %rsp
	pushq	$0
	pushq %rdx
	movq $2, %rax
	pushq %rax
	movq -8(%rbp), %rax
	cqo
	idivq (%rsp)
	popq %rdx
	popq rdx
	movq %rax, -16(%rbp)
WHILE0:
	movq -8(%rbp), %rax
	pushq %rax
	pushq %rdx
	pushq %rdx
	movq -16(%rbp), %rax
	pushq %rax
	movq -8(%rbp), %rax
	cqo
	idivq (%rsp)
	popq %rdx
	popq rdx
	pushq %rax
	movq -16(%rbp), %rax
	mulq (%rsp)
	popq %rdx
	popq %rdx
	subq %rax, (%rsp)
	popq %rax
	movq $0, %rbx
	cmpq %rax, %rbx
jge	 DONE0
	movq -16(%rbp), %rax
	pushq %rax
	movq $1, %rax
	subq %rax, (%rsp)
	popq %rax
	movq %rax, -16(%rbp)
	jmp WHILE0
DONE0:
	movq -16(%rbp), %rax
	movq $1, %rbx
	cmpq %rax, %rbx
jge	 ELSE0
	movq -16(%rbp), %rdi
	call _factor
	movq %rax, -24(%rbp)
	pushq %rdx
	movq -16(%rbp), %rax
	pushq %rax
	movq -8(%rbp), %rax
	cqo
	idivq (%rsp)
	popq %rdx
	popq rdx
	movq %rax, %rdi
	call _factor
	movq %rax, -24(%rbp)
	jmp END_IF0
ELSE0:
	movq -8(%rbp), %rsi
	movq $intout, %rdi
	call printf
	movq $STR0, %rsi
	movq $strout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
END_IF0:
	movq $0, %rax
	leave
	ret
_main:
	pushq %rbp
	movq %rsp, %rbp
	subq $0, %rsp
	movq $1836311903, %rdi
	call _factor
	leave
	ret
