[bits 16]
org 0500h

jmp start

%include "bootloader\common.inc"
%include "bootloader\bootinfo.inc"
%include "bootloader\gdt.inc"
%include "bootloader\detect_memory.inc"


boot_info:
istruc bootinfo
	at bootinfo.mmap_addr,			dd 0x00002548 ; start of fifth byte in 0x00000500-0x00007BFF region (free for use, almost 30KiB)
iend

start:

sti
xor eax, eax
int 11h
mov word [boot_info+bootinfo.BIOS_equipment_list], ax
cli

mov eax, 0x0
mov ds, ax ;dubts !!!!!
mov es, ax ;dubts !!!!! 
mov	di, 0x2548 ; mmap_addr
call BiosGetMemoryMap

mov eax, 24
mul bp

mov dword [boot_info+bootinfo.mmap_length], eax

;load gdt
lgdt [GDT_ADRESS]
mov eax, cr0
or 	eax, 1
mov cr0, eax

jmp dword 0x8:start_32


start_32:
[bits 32]

mov ax,10h
mov ds,ax
mov es,ax
mov fs,ax
mov gs,ax
mov ss,ax

mov esp, 0x0007FFF0 ;0x00007E00-0x0007FFFF	480.5 KiB	RAM (guaranteed free for use)	Conventional memory (stack here)

call enable_a20

call load_kernel
mov ecx, eax

mov dword [boot_info+bootinfo.kernel_base], edx
mov dword [boot_info+bootinfo.kernel_img_size], edi

sti

lea ebx, [boot_info]
push ebx

call ecx ;call kernel

times 1024-($-$$) db 0