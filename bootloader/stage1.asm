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

; enable a20
call check_a20
cmp ax, 1
je a20_activated
call enable_a20_1 ; int 15

call check_a20
cmp ax, 1
je a20_activated
call enable_a20_2 ; keyboard controller

call check_a20
cmp ax, 1
je a20_activated
call enable_a20_3 ; fast a20

a20_activated:

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

; Function: check_a20
;
; Purpose: to check the status of the a20 line in a completely self-contained state-preserving way.
;          The function can be modified as necessary by removing push's at the beginning and their
;          respective pop's at the end if complete self-containment is not required.
;
; Returns: 0 in ax if the a20 line is disabled (memory wraps around)
;          1 in ax if the a20 line is enabled (memory does not wrap around)
check_a20:
  pushf
  push ds
  push es
  push di
  push si
 
  cli
 
  xor ax, ax ; ax = 0
  mov es, ax
 
  not ax ; ax = 0xFFFF
  mov ds, ax
 
  mov di, 0x0500
  mov si, 0x0510
 
  mov al, byte [es:di]
  push ax
 
  mov al, byte [ds:si]
  push ax
 
  mov byte [es:di], 0x00
  mov byte [ds:si], 0xFF
 
  cmp byte [es:di], 0xFF
 
  pop ax
  mov byte [ds:si], al
 
  pop ax
  mov byte [es:di], al
 
  mov ax, 0
  je check_a20__exit
 
  mov ax, 1
 
  check_a20__exit:
  pop si
  pop di
  pop es
  pop ds
  popf
 
  ret

enable_a20_1:
  mov     ax,2403h                ;--- A20-Gate Support ---
  int     15h
  jb      enable_a20_1_exit                  ;INT 15h is not supported
  cmp     ah,0
  jnz     enable_a20_1_exit                  ;INT 15h is not supported
   
  mov     ax,2402h                ;--- A20-Gate Status ---
  int     15h
  jb      enable_a20_1_exit              ;couldn't get status
  cmp     ah,0
  jnz     enable_a20_1_exit              ;couldn't get status
   
  cmp     al,1
  jz      enable_a20_1_exit           ;A20 is already activated
   
  mov     ax,2401h                ;--- A20-Gate Activate ---
  int     15h
  jb      enable_a20_1_exit              ;couldn't activate the gate
  cmp     ah,0
  jnz     enable_a20_1_exit              ;couldn't activate the gate
   
  enable_a20_1_exit:
  
  ret
	
enable_a20_2:
 call    a20wait
 mov     al,0xAD
 out     0x64,al

 call    a20wait
 mov     al,0xD0
 out     0x64,al

 call    a20wait2
 in      al,0x60
 push    eax

 call    a20wait
 mov     al,0xD1
 out     0x64,al

 call    a20wait
 pop     eax
 or      al,2
 out     0x60,al

 call    a20wait
 mov     al,0xAE
 out     0x64,al

 call    a20wait
 ret

a20wait:
  in      al,0x64
  test    al,2
  jnz     a20wait
  ret


a20wait2:
  in      al,0x64
  test    al,1
  jz      a20wait2
  ret

enable_a20_3:
  in al, 0x92
  or al, 2
  out 0x92, al
  
  ret

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










