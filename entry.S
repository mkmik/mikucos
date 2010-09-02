#include "init.h"
#include "config.h"
		
.text
.globl _start
_start:
	mov	%eax, mb_magic
	mov	%ebx, mb_info

#if !USE_MAL
	lgdt	gdt_desc
	ljmp	$(__KERNEL_CS), $1f
1:	
	mov	$__KERNEL_DS, %ax
	mov	%eax, %ds
	mov	%eax, %es
	mov	%eax, %fs
	mov	%eax, %gs
	mov	%eax, %ss
#endif	
	/* init stack */
	lea	init_stack, %eax
	add	$INIT_STACK_SIZE, %eax
	mov	%eax, %esp
#if !USE_MAL	
	/* now we can call subroutines */
	call	load_idt
#endif
	call	enable_cache

	cld		/* gcc wants dir flag cleared */
	/* call C entry point */
	call	init
#if !USE_MAL
/* Activates the interrupt description table */
load_idt:
	call	setup_idt
	lidt	idt_desc
	ret
#endif
	
enable_cache:
	mov	%cr0, %eax
	and	$0x9FFFFFFF, %eax
	mov	%eax, %cr0
	ret
#if !USE_MAL
/* Fill the interrupt descriptor table
 * with the default handler.
 * 
 * idt format:	offset	b3	b2	|	b1	b0
 *		0	segment selector|	low offset
 *		4	high offset	|	flags	*/
setup_idt:
	lea	ignore_isr, %edx
	mov	$__KERNEL_CS, %eax
	shl	$16, %eax
	movw	%dx, %ax
	movw	$0x8E00,%dx        /* interrupt gate - dpl=0, present */
	
	lea	idt_table, %edi
	mov	$256, %ecx
idt_fill:
	movl	%eax, (%edi)
	movl	%edx, 4(%edi)
	add	$8, %edi
	dec	%ecx
	jne	idt_fill
	ret

/* This is the default interrupt "handler" */
.globl ignore_isr		
ignore_isr:
	pusha
	pushw	$'i'
	lea	libc_video_ops, %eax
	push	%eax
	call	dputchar
	add	$2, %esp
eless:	jmp	eless	
	popa
	iret
	
.data	
.align 4
	.word	0
idt_desc:
	.word	256*8-1
	.long 	idt_table
	.word	0
gdt_desc:
	.word	256*8-1
	.long 	gdt_table
#endif
