[bits 16]
org 7C00h

start:

cli

; stack at 0x0007FFF0 ;0x00007E00-0x0007FFFF	480.5 KiB	RAM (guaranteed free for use)	Conventional memory
mov ax, 0x7000
mov ss, ax
mov sp, 0xFFF0

; Print BOOTING... message
call print_booting

;http://www.ctyme.com/intr/rb-0607.html
;reading stage2 (in CHS mode)
mov ax, 0x0000 ; stage2 at 0x0000:7E00
mov es, ax
mov bx, 0x7E00
mov ah, 2  ; read sectors into memory
mov al, 2 ;size of stage2
mov ch, 0
mov cl, 2 ;starting sector number
mov dh, 0
; dl is set by BIOS
clc
sti
int 13h
cli
jc error

; read kernel (in LBA mode)
; Set DS:SI -> Disk Address Packet in memory
mov ax, 0x0000
mov ds, ax
mov si, disk_adress_packet
mov ah, 0x42
; dl is set by BIOS
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

jmp dword 0x0000:0x7E00 ;jump stage2

print_booting:
  mov ax, 0xb800
  mov fs, ax
  xor bx, bx

  mov word [fs:bx], 0x8A42 ; B
  add bx, 2

  mov word [fs:bx], 0x8A4F ; O
  add bx, 2

  mov word [fs:bx], 0x8A4F ; O
  add bx, 2

  mov word [fs:bx], 0x8A54 ; T
  add bx, 2

  mov word [fs:bx],  0x8A49 ; I
  add bx, 2

  mov word [fs:bx],  0x8A4E ; N
  add bx, 2
  
  mov word [fs:bx],  0x8A47 ; G
  add bx, 2
  
  mov word [fs:bx],  0x8A2E ; .
  add bx, 2
  
  mov word [fs:bx],  0x8A2E ; .
  add bx, 2
  
  mov word [fs:bx],  0x8A2E ; .
  add bx, 2

  ret

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

  mov word [es:di],  0x8452 ; R
  add di, 2

  mov word [es:di],  0x8421 ; !
  add di, 2

  jmp $

  ret
  
disk_adress_packet:
db 16 ; size of packet
db 0x00 ; reserved (0)
dw 127 ; number of blocks to transfer (max 007Fh -> 127 for Phoenix EDD)
dw 0x0000 ; transfer buffer ; linear 0x00010000; ;0x00007E00-0x0007FFFF	480.5 KiB	RAM (guaranteed free for use)	Conventional memory
dw 0x1000
dd 3; lower 32-bits of 48-bit starting LBA
dd 0; upper 16-bits of 48-bit starting LBA

  
%if ($ - $$) > 446
  %fatal "Bootloader code exceed 512 bytes."
%endif

times 446-($-$$) db 0

; MBR partition table https://wiki.osdev.org/Partition_Table
times 48 db 0 ; empty entries
db 0x80 ; bootable partition
db 0x0 ; starting head
db 0x2 ; Starting sector (Bits 6-7 are the upper two bits for the Starting Cylinder field.) (Sectors are ennumerated from 1)
db 0x0 ; Starting Cylinder
db 0x4D ; System ID ('M' for MatOS in this case) not important
; For drives bigger than 8GB, generally the CHS fields are set to Cylinder = 1023, Head = 254 or 255, Sector = 63 -- which is considered an invalid setting.
db 0xFF ; Ending Head
db 0xFF ; Ending Sector (Bits 6-7 are the upper two bits for the ending cylinder field)
db 0xFF ; Ending Cylinder
dd 0x00000001 ; Relative Sector (to start of partition -- also equals the partition's starting LBA value)
dd 16515071; Total Sectors in partition (- 1 ????) 1024*256*63 - 1 (starts at sector 2)

; bootsector signature
dw  0xAA55










