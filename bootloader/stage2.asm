[bits 16]
org 0500h

jmp start

%include "bootloader\common.inc"
%include "bootloader\multiboot.inc"
%include "bootloader\gdt.inc"
%include "bootloader\detect_memory.inc"


boot_info:
istruc multiboot_info
	at multiboot_info.flags,			dd 0
	at multiboot_info.memoryLo,			dd 0
	at multiboot_info.memoryHi,			dd 0
	at multiboot_info.bootDevice,		dd 0
	at multiboot_info.cmdLine,			dd 0
	at multiboot_info.mods_count,		dd 0
	at multiboot_info.mods_addr,		dd 0
	at multiboot_info.syms0,			dd 0
	at multiboot_info.syms1,			dd 0
	at multiboot_info.syms2,			dd 0
	at multiboot_info.mmap_length,		dd 0
	at multiboot_info.mmap_addr,		dd 0x00002548 ; start of fifth byte in 0x00000500-0x00007BFF region (free for use, almost 30KiB)
	at multiboot_info.drives_length,	dd 0
	at multiboot_info.drives_addr,		dd 0
	at multiboot_info.config_table,		dd 0
	at multiboot_info.bootloader_name,	dd 0
	at multiboot_info.apm_table,		dd 0
	at multiboot_info.vbe_control_info,	dd 0
	at multiboot_info.vbe_mode_info,	dw 0
	at multiboot_info.vbe_interface_seg,dw 0
	at multiboot_info.vbe_interface_off,dw 0
	at multiboot_info.vbe_interface_len,dw 0
iend

start:

mov eax, 0x0
mov ds, ax ;dubts !!!!!
mov es, ax ;dubts !!!!! 
mov	di, 0x2548 ; mmap_addr
call BiosGetMemoryMap

mov eax, 24
mul bp

mov dword [boot_info+multiboot_info.mmap_length], eax

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

sti
mov eax, 0x2BADB002 ;multiboot magic number
lea ebx, [boot_info]
push ebx
call ecx ;call kernel

times 1024-($-$$) db 0