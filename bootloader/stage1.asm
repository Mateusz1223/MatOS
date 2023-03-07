[bits 16]
org 7C00h

start:

cli

mov ax, 0x0007 ;0x00007E00-0x0007FFFF	480.5 KiB	RAM (guaranteed free for use)	Conventional memory (stack here)
mov ss, ax
mov sp, 0xFFF0

;http://www.ctyme.com/intr/rb-0607.html

;reading stage2 from floppy
mov ax, 0x0000 ; stage2 at 0x0000:0x0500 (1 KB)
mov es, ax
mov bx, 0x0500

; read stage2
mov ah, 2  ; read sectors into memory
mov al, 2 ;size of stage2
mov ch, 0
mov cl, 2 ;starting sector number
mov dh, 0
;mov dl, 0 ; Set by BIOS
sti
int 13h
cli

mov ax, 0x0000 ; 0x0000:0x7E00; ;0x00007E00-0x0007FFFF	480.5 KiB	RAM (guaranteed free for use)	Conventional memory (kernel img at the bottom and stack at the top)
mov es, ax
mov bx, 0x7E00

; read kernel
mov ah, 2  ; read sectors into memory
; fix by build.py 
mov al, 0xcc
nop
nop
mov ch, 0
; fix by build.py 
mov cl, 4 ;starting sector number
mov dh, 0
;mov dl, 0 ; Set by BIOS
clc
sti
int 13h
cli
jc error

;http://www.ctyme.com/intr/rb-0091.htm
;set 0. display page
xor eax, eax
mov ah, 0x05
mov al, 0
sti
int 10h
cli

jmp dword 0x0000:0x0500 ;jump stage2

error:
  ;print error
  mov ax, 0xb800
  mov es, ax
  xor di, di

  mov word [es:di], 0x8445 ; E
  add di, 2

  mov word [es:di], 0x8452 ; R
  add di, 2

  mov word [es:di], 0x8452 ; R
  add di, 2

  mov word [es:di], 0x844f ; O
  add di, 2

  mov word [es:di],  0x8452 ; O
  add di, 2

  mov word [es:di],  0x8421 ; !
  add di, 2

  jmp $

  ret

times 510-($-$$) db 0
dw  0xAA55