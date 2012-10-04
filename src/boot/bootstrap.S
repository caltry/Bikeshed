/* FancyCat compliant booloader - TODO - Document specifics of 
 * switch_to_real_mode and switch_to_protected_mode
 *
 * Date: 07/07/2012
 *
 * Author: Walter Litwinczyk
 * Contributors: David C. Larsen, Sean T. Congden
 *
 * Adapted from the Rochester Institute of Technology - Systems Programming 2
 * course
 *     Author: Jon Coles
 *     Contributors: Warren R. Carithers, K. Reek, Garrett C. Smith
 */

/* GDT Constants */
GDT_CODE  = 0x0010 /* All of memory, R/E */
GDT_DATA  = 0x0018 /* All of memory, R/W */
GDT_STACK = 0x0020 /* All of memory, R/W */
GDT_CODE_16  = 0x0028
GDT_DATA_16  = 0x0030
GDT_STACK_16 = 0x0038

/* Stack constants */
REAL_STACK = 0x7C00-2 /* Start the stack right before the bootloader code */
PROT_STACK = REAL_STACK - 0x1000 /* Give the real mode 4KiB of stack */
/* This should put the realmode stack at 0x6C00-0x7c00
 * and the protected mode stack gets from the end of the
 * GDT and IDT (0x2D00 = ~11KiB) to 0x6C00, which should
 * be more than enough for the bootloaders purposes.
 * Especially because the bootloader does very little 
 * stack allocation. This also allows the copy buffer to
 * be larger.
 */

/* GDT constants */
GDT_SEGMENT = 0x0050
GDT_ADDRESS = 0x0500

/* IDT constants */
IDT_SEGMENT = 0x0250
IDT_ADDRESS = 0x2500

/* Memory map constants */
MMAP_SEGMENT = 0x02D0
MMAP_ADDRESS = 0x2D00
MMAP_EXT_LO = 0x00
MMAP_EXT_HI = 0x02
MMAP_CFG_LO = 0x04
MMAP_CFG_HI = 0x06

/* Other constants */
BOOT_SEGMENT  = 0x07c0 /* Default BIOS address to load the bootsector */
BOOT_ADDRESS  = 0x7c00 /* 32-bit address of the bootloader */
START_SEGMENT = 0x0000
START_OFFSET  = 0x7E00
SECTOR_SIZE   = 0x0200 /* 512 bytes */
OFFSET_LIMIT  = 65536 - SECTOR_SIZE

/* Copy buffer constants */
/* This is the location of the buffer used
 * to copy data from real mode to protected
 * mode
 */
BUFFER_START = 0x8200 /* XXX - Adjust this value if the bootloader grows! */
BUFFER_SEG = 0x0820 /* XXX - Adjust this value if the bootloader grows! */
BUFFER_OFF = 0
BUFFER_SECTORS = 100
COPY_START_FOR_PROTECTED_MODE = BUFFER_START + 0x8 /* Skip the header information */

.code16
#.globl begtext
#begtext:
.globl bootloader_entry
bootloader_entry:
	/* Setup the realmode runtime stack */
	movw $BOOT_SEGMENT, %ax /* Load the data segment */
	movw %ax, %ds

	xorw %ax, %ax
	movw %ax, %ss /* Stack segment starts at 0x0 */
	movw $REAL_STACK, %ax /* Stack starts before our bootloader */
	movw %ax, %sp

	/* Verify that we have a working disk */
	movb $0x01, %ah /* Test the disk status and make sure */
	movb drive, %dl /* it's safe to proceed */
	int $0x13
	jnc disk_okay

	movw $error_disk_status, %si /* Something went wrong; print a message */
	call display_message
	/* Intentionally drop through to the halt, an error has happend */

/* Halts the processor and does nothing
 *
 * Parameters:
 *     None
 */
halt: /* Halt the processor, there's nothing we can do */
	hlt
	jmp halt

disk_okay:
	movw $0, %ax /* Reset the disk */
	movb drive, %dl
	int $0x13

	/* Get the drive's parameters to determine the number of heads
	 * and sectors per track. 
	 */
	xorw %ax, %ax /* Set ES:DI = 0000:0000 in case of BIOS bugs */
	movw %ax, %es
	movw %ax, %di
	movb $0x08, %ah /* Get drive parameters */
	movb drive, %dl /* Hard disk, USB, or floppy */
	int $0x13

	/* Store (max + 1) - CL[5:0] = maximum head, DH = maximum head */
	andb $0x3F, %cl
	incb %cl
	incb %dh

	movb %cl, max_sectors
	movb %dh, max_heads

/* We have verified the disk is okay. Now we have to load the rest of the
 * bootloader into memory
 */
 	movw $message_loading, %si /* Print the loading message */
	call display_message

	movw $2, %ax /* Sector count = 2, we have a large bootloader :) */
	movw $START_SEGMENT, %bx /* Read the sectors to bx:es */
	movw %bx, %es /* Which happens to be right after this code */
	movw $START_OFFSET, %bx
	call read_sectors

	jmp second_stage
	
/* Read sectors off the disk into a specified memory location
 *
 * Parameters:
 *     AX - number of sectors to Read
 *     ES:BX - Starting address for the block
 */
current_sector: .word 2  /* Cylinder = 0, Sector = 1 */
current_head:   .word 0  /* Head = 0 */
max_sectors:    .byte 19 /* Up to 18 sectors per floppy track */
max_heads:      .byte 2  /* Only two R/W heads per floppy drive */

read_sectors:
	pushw %ax /* Save the sector count */

	movw $3, %cx /* Our retry count */
rs_retry:
	pushw %cx /* Push the retry count onto the stack */

	movw current_sector, %cx /* Get our current sector offset */
	movw current_head, %dx /* Get our current head number */
	movb drive, %dl

	movw $0x0201, %ax /* Read one sector */
	int $0x13
	jnc rs_read_continue

	movw $error_disk_read, %si /* Display an error message, we couldn't */
	call display_message /* read the disk */
	popw %cx /* Get the retry count and */
	loop rs_retry /* try again */
	movw $error_disk_fail, %si /* We failed to read the disk even after */
	call display_message /* retrying */

	jmp halt /* Failed, halt the processor */

rs_read_continue:
	movw $message_dot, %si /* Print status, a dot */
	call display_message
	cmpw $OFFSET_LIMIT, %bx /* Have we reached the offset limit? */
	je rs_adjust /* Yes, we must adjust the es register */
	addw $SECTOR_SIZE, %bx /* No, just add the blocksize to */
	jmp rs_read_continue_2 /* the offset and continue */

rs_adjust:
	movw $0, %bx /* Start offset over again */
	movw %es, %bx
	addw $0x1000, %ax /* Move the segment pointer to the next chunk */
	movw %ax, %es

rs_read_continue_2:
	incb %cl /* Not done, move to the next sector */
	cmpb max_sectors, %cl /* Only 18 per track, see if we need */
	jnz rs_save_sector /* to switch heads or tracks */

	movb $1, %cl /* Reset sector number */
	incb %dh /* First switch heads */
	cmpb max_heads, %dh /* There are only two, if we've already */
	jnz rs_save_sector /* used both, we need to switch tracks */

	xorb %dh, %dh /* Reset the head to 0 */
	incb %ch /* Increment the track number */
	cmpb $80, %ch /* 80 tracks per side, have we read all of them? */
	jnz rs_save_sector

	movw $error_too_big, %si /* Report the error */
	call display_message
	jmp halt /* Failed, halt the processor */

rs_save_sector:
	movw %cx, current_sector /* Save the current sector number */
	movw %dx, current_head /* Save the current head number */

	popw %ax /* Discard the retry count */
	popw %ax /* Get the sector count back from the stack */
	decw %ax /* Decrement the sector count */
	jg read_sectors /* If it's zero, we're done reading */

	movw $message_bar, %si /* Print a message saying this block has been read */
	call display_message
	ret /* Return to who called us */

/* Prints a message to the screen using BIOS calls
 *
 * Parameters:
 *     SI - The message to display (pointer to it)
 */
display_message:
	pushw %ax /* Save general registers */
	pushw %bx

dm_repeat:
	lodsb /* Grab the next character */

	movb $0x0E, %ah /* Write and advance the cursor */
	movw $0x07, %bx /* Page 0, White on blank, no blink */
	orb %al, %al /* AL is the character to write */
	jz dm_done

	int $0x10 /* Print the character to the screen */
	jmp dm_repeat

dm_done:
	popw %bx /* Restore the stack */
	popw %ax
	ret /* Return to the caller */

/* Move the GDT entries from where they are to 0000:0000
 *
 * Parameters:
 *     None
 */
move_gdt:
	movw %cs, %si
	movw %si, %ds
	movw $start_gdt + BOOT_ADDRESS, %si
	movw $GDT_SEGMENT, %di
	movw %di, %es
	xorw %di, %di
	movl $gdt_len, %ecx
	cld
	rep movsb
	ret

/* Determine how much physical memory is available
 *
 * Parameters:
 *     None
 */
check_memory:
	pushw %ax
	pushw %ds
	movw $MMAP_SEGMENT, %ax
	movw %ax, %ds /* Write relative to the memory map start */
	movw $0xE801, %ax /* Get the memory size for >64MiB configurations */
	int $0x15
	movw %ax, MMAP_EXT_LO /* Extended memory between 1MiB - 16MiB in KiB */
	movw %bx, MMAP_EXT_HI /* Extended memory > 16MiB in 64KiB blocks */
	movw %cx, MMAP_CFG_LO /* Configured memory between 1MiB - 16MiB in KiB */
	movw %dx, MMAP_CFG_HI /* Configured memory > 16MiB in 64KiB blocks */
	popw %ds
	popw %ax
	ret

/* The GDT and IDTR contents
 */
gdt_48:
	.word 0x2000 /* 1024 GDT entries * 8 bytes per entry = 8192 */
	.quad GDT_ADDRESS

idt_48:
	.word 0x0800 /* 256 interrupts */
	.quad IDT_ADDRESS

/* Error messages */
message_loading:
	.asciz "Loading"
message_dot:
	.asciz "."
message_go:
	.asciz "Done"
message_bar:
	.asciz "|"
error_disk_status:
	.asciz "Disk not ready\n\r"
error_disk_read:
	.asciz "Read failed\n\r"
error_too_big:
	.asciz "Too big\n\r"
error_disk_fail:
	.asciz "Can't proceed\n\r"

/* End the first sector of the boot program. The last two bytes of
 * this sector must be AA55 in order for the disk to be recognized
 * by the BIOS as bootable
 */
.org SECTOR_SIZE-4
drive: 
	.word 0x80 /* 0x00 = floppy, 0x80 = USB */
boot_sig:
	.word 0xAA55

/* This marks the second stage of our bootloader, this is the part of the
 * bootloader that isn't automatically loaded into RAM by the BIOS
 */
second_stage:
	call floppy_off
	call enable_A20
	call move_gdt
	call check_memory
	
	/* IMPORTANT, make sure we fix the data segment register! */
	movw $BOOT_SEGMENT, %bx
	movw %bx, %ds

	/* Load the modules */
ss_load_loop:
	call load_module /* EAX contains -1 if we're done */
	cmpl $0xFFFFFFFF, %eax
	jne ss_load_loop

	/* The kernel expects to be in protected mode */
	call switch_to_protected_mode
	.code32

	/* Make sure interrupts aren't on when we start the kernel */
	cli

	/* Alright everything should be good, lets jump to the kernel! */
	movl kernel_location+BOOT_ADDRESS, %eax /* Add BOOT_ADDRESS because */
	push %eax                               /* segmentation is off */
	ret

	/* In case the kernel returns */
	cli
kernel_returned:
	hlt
	jmp kernel_returned 

	.code16 /* The rest of the bootloader is still realmode code */

/* Loads a single module from the disk
 *
 * Parameters:
 *     None
 *
 * Returns:
 *     EAX - Contains -1 if there are no more modules to load.
 *           Otherwise it contains a non -1 value.
 */
/* Some helper variables */
kernel_location: .quad 0x0
load_location:   .quad 0x0
remaining: 		 .quad 0x1
.code16
load_module:
	pushw %bx
	pushw %cx
	pushw %dx
	pushw %es

	/* Read in 1 sector of information */
	movw $1, %ax
	movw $BUFFER_SEG, %bx
	movw %bx, %es
	movw $BUFFER_OFF, %bx
	call read_sectors

	/* Get the load location */
	movl %es:(0), %eax /* The first part of the header is the load location */
	cmpl $0xFFFFFFFF, %eax /* Check the load location, -1 means we're done */
	je lm_exit

	/* Okay this is a valid module, lets continue loading */
	movl %eax, load_location /* Save the load location */

	/* Also, if it's the first module, save it into kernel_location */
	movl kernel_location, %ebx
	testl %ebx, %ebx
	jnz lm_kernel_already_saved
	movl %eax, kernel_location /* Save the kernel's location */

lm_kernel_already_saved:
	movl %es:(4), %eax /* Grab the size of the module in sectors */
	movl %eax, remaining /* Save the size in sectors */

	/* Copy this first sector over */
	movl $COPY_START_FOR_PROTECTED_MODE, %eax /* We want to skip the header */
	movl $1, %ebx /* infomation */
	call copy_buffer

	/* This is a one time deal, since we skipped the first 8 bytes, but we
	 * loaded and copied 512 bytes with the copy_buffer, we need to fix the
	 * load_location by subtracting 8 bytes, otherwise we'll have a hole in
	 * our data
	 *
	 * NOTE: load_location is adjusted by copy_buffer
	 */
	movl load_location, %edx
	subl $8, %edx
	movl %edx, load_location

	/* Check if there's any more left to load */
lm_loop:
	movl remaining, %eax
	testl %eax, %eax
	jz lm_exit /* If there's nothing left, exit */

	/* We need to load more! */
	movl remaining, %eax
	movl $BUFFER_SECTORS, %ecx /* Check if we're loading too much */
	cmpl %eax, %ecx
	jge lm_enough_sectors

	movl %ecx, %eax /* We're loading too much, fix it! */

lm_enough_sectors:
	/* Read in some sectors */
	pushw %ax /* Save EAX in case read_sectors modifies it */

	movw $BUFFER_SEG, %bx
	movw %bx, %es
	movw $BUFFER_OFF, %bx
	call read_sectors

	movl remaining, %eax

	popw %bx /* Restore the sector count, in bx for preparation to call */
	movl $BUFFER_START, %eax /* copy_buffer, We don't want to skip 8 bytes */
	call copy_buffer /* time */

	jmp lm_loop

lm_exit:
	popw %es
	popw %dx
	popw %cx
	popw %bx
	ret

/* Copies data from a buffer to a 32-bit address
 *
 * Parameters:
 *     EAX - The source address
 *     EBX - The number of sectors to copy
 *
 * This method uses load_location to find the destination address
 * it will also update load_location and remaining with the appropriate
 * values after copying
 */
.code16
copy_buffer:
	pushw %cx
	pushw %dx
	
	/* Switch to protected mode uses only EAX and EBX, so lets not use those! 
	 *
	 * We can't use the stack because it will be different after we call
	 * switch_to_protected_mode
	 */
	movl %eax, %ecx
	movl %ebx, %edx

	/* We need to switch to protected mode */
	call switch_to_protected_mode
	.code32

	/* Restore our registers */
	movl %ecx, %eax
	movl %edx, %ebx

	/* Grab the current load_location */
	movl load_location+BOOT_ADDRESS, %edx /* Add BOOT_ADDRESS because 
	                                       * segmentation is off
                                           */
	
	/* Update remaining while we're here */
	movl remaining+BOOT_ADDRESS, %ecx
	subl %ebx, %ecx
	movl %ecx, remaining+BOOT_ADDRESS

	/* Multiply EBX by 512 to get the number of bytes to copy */
	shl $9, %ebx

/* TODO implement a faster version */
cb_copy_loop:
	movl (%eax), %ecx /* EAX is the source */
	movl %ecx, (%edx) /* EDX is the destination */
	subl $4, %ebx
	addl $4, %eax
	addl $4, %edx
	testl %ebx, %ebx
	jnz cb_copy_loop

	/* Save the new load_location for next time */
	movl %edx, load_location+BOOT_ADDRESS

	/* Before returning we need to switch back to realmode */
	call switch_to_real_mode
	.code16

	popw %dx	
	popw %cx
	ret

/* Switches from protected mode to realmode
 *
 * Parameters:
 *     None
 *
 * Returns:
 *     Nothing
 */
 PROT_MODE_ESP: .quad PROT_STACK
 PROT_MODE_EBP: .quad PROT_STACK
 .code32
 switch_to_real_mode:
 	cli /* Turn off interrupts, this is tricky business */

	/* Save the current ESP and EBP values */
	movl %esp, PROT_MODE_ESP+BOOT_ADDRESS
	movl %ebp, PROT_MODE_EBP+BOOT_ADDRESS

	/* Setup the real mode stack */	
	movl (%esp), %ecx /* Copy the return address for later */

	movl REAL_MODE_ESP+BOOT_ADDRESS, %esp /* Switch to the realmode stack */
	movl REAL_MODE_EBP+BOOT_ADDRESS, %ebp

	/* Load the real mode GDT selectors */
	movw $GDT_DATA_16, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw $GDT_STACK_16, %ax
	movw %ax, %ss

	/* This sets the code segment register */
	ljmp $GDT_CODE_16, $(_not_quite_real+BOOT_ADDRESS)
_not_quite_real:
	
	/* Switch off the protected mode flag */
	movl %cr0, %eax /* Get the current CR0 */
	andl $0xFFFFFFFE, %eax /* Clear the PE bit */
	movl %eax, %cr0 /* And store it back */

	/* Long jump to a real mode code segment */
	.byte 0x66
	ljmp $0x0, $_real_mode_segment+BOOT_ADDRESS

.code16
_real_mode_segment:

	/* Setup the real mode segments */
	movw $BOOT_SEGMENT, %ax
	movw %ax, %ds
	/* Clear the general purpose registers */
	xorw %ax, %ax
	movw %ax, %ss /* Stack segment starts at 0! */
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	/* Load the real mode IDT */
	lidt (real_idt)

	push %cx
	sti
	ret

/* Switches from real mode to protected mode
 *
 * Parameters:
 *     None
 *
 * Returns:
 *     Nothing
 */
REAL_MODE_ESP: .quad REAL_STACK
REAL_MODE_EBP: .quad REAL_STACK
.code16
switch_to_protected_mode:
	cli /* Turn off interrupts, this is tricky business! */
	movb $0x08, %al /* Disable NMIs */
	outb %al, $0x70

	/* Load the protected mode IDT */
	lidt (idt_48)
	lgdt (gdt_48)

	/* Set the protected mode flag */
	movl %cr0, %eax /* Get the current CR0 */
	orb $0x1, %al /* Set the PE bit */
	movl %eax, %cr0 /* And store it back */

	/* Long jump to a real mode code segment */
	.byte 0x66
	.code32
	ljmp $GDT_CODE, $(_protected_mode_segment + BOOT_ADDRESS)
_protected_mode_segment:

	cli /* Sometimes interrupts are turned back on? */
	movb $0x08, %al /* Disable NMIs */
	outb %al, $0x70

	/* Load the real mode GDT selectors */
	xorw %bx, %bx
	movw $GDT_DATA, %ax
	movw %ax, %ds
	movw %bx, %es
	movw %bx, %fs
	movw %bx, %gs
	movw $GDT_STACK, %ax
	movw %ax, %ss
	
	/* Get rid of the current return address */
	popw %bx
	andl $0x0000FFFF, %ebx

	/* Save the current ESP and EBP */
	movl %esp, REAL_MODE_ESP+BOOT_ADDRESS
	movl %ebp, REAL_MODE_EBP+BOOT_ADDRESS

	/* Switch to the protected mode stack */
	movl PROT_MODE_ESP+BOOT_ADDRESS, %esp /* Restore the saved ESP */
	movl PROT_MODE_EBP+BOOT_ADDRESS, %ebp /* Restore the saved EBP */

	push %ebx
	ret

	.code16 /* Just in case */


/* IDTs and GDTs */
.code16
real_idt:
	.short 0x3FF
	.long 0x0

/* Turns the floppy disk off 
 */
floppy_off:
	push %dx
	movw $0x3F2, %dx
	xorb %al, %al
	outb %al, %dx
	pop %dx
	ret

/* Enable the A20 gate for full memory access
 */
enable_A20:
	call a20wait
	movb $0xad, %al
	outb %al, $0x64

	call a20wait
	movb $0xd0, %al
	outb %al, $0x64	

	call a20wait2
	inb $0x60, %al	
	pushl %eax

	call a20wait
	movb $0xd1, %al
	outb %al, $0x64

	call a20wait
	popl %eax
	orb $2, %al
	outb %al, $0x60

	call a20wait
	mov $0xae, %al
	out %al, $0x64

	call a20wait
	ret

a20wait: /* Wait until bit 1 of the device register is clear */
	movl $65536, %ecx /* Loop a lot if need be */
wait_loop:
	inb $0x64, %al /* Grab the byte */
	test $2, %al /* Is the bit clear? */
	jz wait_exit /* Yes */
	loop wait_loop /* No, so loop */
	jmp a20wait /* If still not clear, go again */
wait_exit:
	ret

a20wait2: /* Like a20wait, but waits until bit 0 is set */
	movl $65536, %ecx
wait2_loop:
	in $0x64, %al
	test $1, %al
	jnz wait2_exit
	loop wait2_loop
	jmp a20wait2
wait2_exit:
	ret

/* The GDT
 */
start_gdt:
	.word 0,0,0,0 /* First GDT entry is always null */

linear_seg: /* Limit 0xFFFFF, base 0, R/W data segment, 32-bit 4K */
	.word 0xFFFF /* limit[15:0] */
	.word 0x0000 /* base[15:0] */
	.byte 0x00   /* base[23:16] */
	.byte 0x92   /* access byte */
	.byte 0xCF   /* granularity */
	.byte 0x00   /* base[31:24] */

code_seg: /* Limit 0xFFFFF, base 0, R/E code segment, 32-bit 4K */
	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.byte 0x9A /* 1 00 1 1010: present, prio 0, C/D, R/E code */
	.byte 0xCF /* 1 1 00 1111: 4K, 32-bit, 0, 0, limit[19:16] */
	.byte 0x00

data_seg: /* Limit 0xFFFFF, base 0, R/W, data segment, 32-bit 4K */
	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.byte 0x92 /* 1 00 1 0010: present, prio 0, C/D, R/W data */
	.byte 0xCF
	.byte 0x00

stack_seg: /* Limit 0xFFFFF, base 0, R/W data seg, 32-bit 4K */
	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.byte 0x92
	.byte 0xCF
	.byte 0x00

code_seg_16: /* Limit 64K, base 0, R/E code segment, 16-bit 4K */
	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.byte 0x9A
	.byte 0x00
	.byte 0x00

data_seg_16: /* Limit 64K, base 0, R/W code segment, 16-bit 4K */
	.word 0xFFFF
	.word 0x7C00
	.byte 0x00
	.byte 0x92
	.byte 0x00
	.byte 0x00

stack_seg_16: /* Limit 64K, base 0, R/W code segment, 16-bit 4K */
	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.byte 0x92
	.byte 0x00
	.byte 0x00

data_seg_16_2: /* Limit 64K, base 0, R/W code segment, 16-bit 4K */
	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.byte 0x92
	.byte 0x00
	.byte 0x00

stack_seg_16_2: /* Limit 64K, base 0, R/W code segment, 16-bit 4K */
	.word 0xFFFF
	.word 0x0000
	.byte 0x00
	.byte 0x92
	.byte 0x00
	.byte 0x00

end_gdt:
gdt_len = end_gdt - start_gdt

.org 1536
