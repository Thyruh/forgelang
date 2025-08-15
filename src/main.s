	.file	"main.cpp"
	.text
	.section	.rodata
	.align 8
.LC0:
	.string	"Error: One argument required: a file to compile.\n"
	.align 8
.LC1:
	.string	"       Correct syntax: ./forge <name>.forge"
.LC2:
	.string	"./myfile"
.LC3:
	.string	"Exited with code 0"
	.text
	.globl	main
	.type	main, @function
main:
.LFB2273:
	.cfi_startproc
	.cfi_personality 0x9b,DW.ref.__gxx_personality_v0
	.cfi_lsda 0x1b,.LLSDA2273
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%rbx
	subq	$552, %rsp
	.cfi_offset 3, -24
	movl	%edi, -548(%rbp)
	movq	%rsi, -560(%rbp)
	cmpl	$2, -548(%rbp)
	je	.L2
	movl	$10, %edi
.LEHB0:
	call	putchar@PLT
	movq	stderr(%rip), %rax
	movq	%rax, %rcx
	movl	$49, %edx
	movl	$1, %esi
	leaq	.LC0(%rip), %rax
	movq	%rax, %rdi
	call	fwrite@PLT
	leaq	.LC1(%rip), %rax
	movq	%rax, %rdi
	call	puts@PLT
	movl	$1, %ebx
	jmp	.L4
.L2:
	leaq	-544(%rbp), %rax
	movq	%rax, %rdi
	call	_ZNSt13basic_fstreamIcSt11char_traitsIcEEC1Ev@PLT
.LEHE0:
	leaq	-544(%rbp), %rax
	movl	$8, %edx
	leaq	.LC2(%rip), %rcx
	movq	%rcx, %rsi
	movq	%rax, %rdi
.LEHB1:
	call	_ZNSt13basic_fstreamIcSt11char_traitsIcEE4openEPKcSt13_Ios_Openmode@PLT
	leaq	.LC3(%rip), %rax
	movq	%rax, %rdi
	movl	$0, %eax
	call	printf@PLT
.LEHE1:
	movl	$0, %ebx
	leaq	-544(%rbp), %rax
	movq	%rax, %rdi
	call	_ZNSt13basic_fstreamIcSt11char_traitsIcEED1Ev@PLT
.L4:
	movl	%ebx, %eax
	jmp	.L7
.L6:
	movq	%rax, %rbx
	leaq	-544(%rbp), %rax
	movq	%rax, %rdi
	call	_ZNSt13basic_fstreamIcSt11char_traitsIcEED1Ev@PLT
	movq	%rbx, %rax
	movq	%rax, %rdi
.LEHB2:
	call	_Unwind_Resume@PLT
.LEHE2:
.L7:
	movq	-8(%rbp), %rbx
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2273:
	.globl	__gxx_personality_v0
	.section	.gcc_except_table,"a",@progbits
.LLSDA2273:
	.byte	0xff
	.byte	0xff
	.byte	0x1
	.uleb128 .LLSDACSE2273-.LLSDACSB2273
.LLSDACSB2273:
	.uleb128 .LEHB0-.LFB2273
	.uleb128 .LEHE0-.LEHB0
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB1-.LFB2273
	.uleb128 .LEHE1-.LEHB1
	.uleb128 .L6-.LFB2273
	.uleb128 0
	.uleb128 .LEHB2-.LFB2273
	.uleb128 .LEHE2-.LEHB2
	.uleb128 0
	.uleb128 0
.LLSDACSE2273:
	.text
	.size	main, .-main
	.hidden	DW.ref.__gxx_personality_v0
	.weak	DW.ref.__gxx_personality_v0
	.section	.data.rel.local.DW.ref.__gxx_personality_v0,"awG",@progbits,DW.ref.__gxx_personality_v0,comdat
	.align 8
	.type	DW.ref.__gxx_personality_v0, @object
	.size	DW.ref.__gxx_personality_v0, 8
DW.ref.__gxx_personality_v0:
	.quad	__gxx_personality_v0
	.ident	"GCC: (Debian 12.2.0-14+deb12u1) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
