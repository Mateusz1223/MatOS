[bits 16]
org 7C00h

start:

mov ax, 0x0007 ;0x00007E00-0x0007FFFF	480.5 KiB	RAM (guaranteed free for use)	Conventional memory (stack here)
mov ss, ax
mov sp, 0xFFF0

;http://www.ctyme.com/intr/rb-0607.html

;reading stage2 from floppy
mov ax, 0x0000 ; stage2 at 0x0000:0x0500 (1 KB)
mov es, ax
mov bx, 0x0500

mov ah, 2  ; read sectors into memory
mov al, 2 ;size of stage2
mov ch, 0
mov cl, 2 ;starting sector number
mov dh, 0
;mov dl, 0 ; Set by BIOS
int 13h

mov ax, 0x0000 ; 0x0000:0x7E00; ;0x00007E00-0x0007FFFF	480.5 KiB	RAM (guaranteed free for use)	Conventional memory (kernel img at the bottom and stack at the top)
mov es, ax
mov bx, 0x7E00

mov ah, 2  ; read sectors into memory
mov al, 0xcc  ; fix by build.py 
nop
nop
mov ch, 0
mov cl, 4 ;starting sector number
mov dh, 0
;mov dl, 0 ; Set by BIOS
int 13h 

cli

jmp dword 0x0000:0x0500 ;jump stage2

times 510-($-$$) db 0
dw  0xAA55