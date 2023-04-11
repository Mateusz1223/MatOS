[bits 16]
org 7E00h

jmp start

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

call load_kernel
mov ecx, eax

mov dword [boot_info+bootinfo.kernel_base], edx
mov dword [boot_info+bootinfo.kernel_img_size], edi

lea ebx, [boot_info]
push ebx

call ecx ;call kernel

; PhysicalAddress = Segment * 16 + Offset so 0x1000:0000 is equal to 0x10000
load_kernel: ; ret eax = entry point , edx = ImageBase, edi = SizeOfImage
  mov eax, [0x00010000+0x3C];PE offset

  xor ecx, ecx
  mov ecx, [0x00010000+eax+0x50] ;SizeOfImage
  push ecx; push SizeOfImage

  xor ecx, ecx
  mov cx, [0x00010000+eax+6] ;number of sections

  mov edx, [0x00010000+eax+0x34] ;image base
  push edx ; push ImageBase

  xor ebx, ebx
  mov bx, [0x00010000+eax+0x14];optional Header Size
  add ebx, 0x00010000+0x18
  add ebx, eax; ebx - section table

  mov eax,[0x00010000+eax+0x28]
  add eax, edx ;eax - AddressOfEntryPoint

  .l1:

  push ecx

  mov ecx, [ebx+0x10];SizeOfRawData
  mov edi, [ebx+0xC];VirtualAdress
  mov esi, [ebx+0x14];PointerToRawData

  add edi, edx
  add esi, 0x00010000

  cld
  rep movsb 

  add ebx, 0x28 
  pop ecx

  cmp ecx, 5
  loop .l1

  pop edx ; edx = ImageBase
  pop edi ; edi = SizeOfImage

  ret

times 1024-($-$$) db 0