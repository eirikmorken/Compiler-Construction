.section .rodata
intout: .string "%ld "
strout: .string "%s "
errout: .string "Wrong number of arguments"
STR0:	.string "t is"
STR1:	.string "This never executes"
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
	call	_hello
	jmp END
ABORT:
	movq $errout, %rdi
	call puts
END:
	movq %rax, %rdi
	call exit
_hello:
	pushq %rbp
	movq %rsp, %rbp
	subq $0, %rsp
	movq $STR0, %rsi
	movq $strout, %rdi
	call printf
	movq $64, %rdi
	call _test
	movq %rax, %rsi
	movq $intout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
	movq $0, %rax
	leave
	ret
_test:
	pushq %rbp
	movq %rsp, %rbp
	pushq %rdi
	subq $24, %rsp
	movq $32, %rax
	movq %rax, -16(%rbp)
	movq $20, %rax
	movq %rax, -24(%rbp)
	movq $64, %rax
	movq %rax, -32(%rbp)
	movq -32(%rbp), %rax
	pushq %rax
	movq -8(%rbp), %rax
	addq %rax, (%rsp)
	popq %rax
	leave
	ret
	movq $STR1, %rsi
	movq $strout, %rdi
	call printf
	movq $0x0A, %rdi
	call putchar
