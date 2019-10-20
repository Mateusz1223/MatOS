[bits 16]

error:
  ;print error
  mov ax, 0xb800
  mov es, ax
  xor di, di

  mov word [es:di], 0x8445
  add di, 2

  mov word [es:di], 0x8452
  add di, 2

  mov word [es:di], 0x8452
  add di, 2

  mov word [es:di], 0x844f
  add di, 2

  mov word [es:di],  0x8452
  add di, 2

  mov word [es:di],  0x8421
  add di, 2

  jmp $

  ret

[bits 32]

enable_a20:
  ;set a20 http://wiki.osdev.org/A20 (Fast A20 Gate)
  pushad ;test a20
  mov edi,0x112345  ;odd megabyte address.
  mov esi,0x012345  ;even megabyte address.
  mov [esi],esi     ;making sure that both addresses contain diffrent values.
  mov [edi],edi     ;(if A20 line is cleared the two pointers would point to the address 0x012345 that would contain 0x112345 (edi)) 
  cmpsd             ;compare addresses to see if the're equivalent.
  popad
  jne A20_on        ;if not equivalent , A20 line is set.
    
  in al, 0x92 ;enable a20
  test al, 2
  jnz A20_on
  or al, 2
  and al, 0xFE
  out 0x92, al
   
  A20_on:

  ret

;when kernel img was loaded at 0x20000000 instead of 0x00007e00 was 0x200000 because PhysicalAddress = Segment * 16 + Offset so 0x2000:0000 is equal to 0x20000

load_kernel: ; ret eax = entry point
  mov eax, [0x00007E00+0x3C];PE offset

  xor ecx, ecx
  mov cx, [0x00007E00+eax+6] ;number of section

  mov edx, [0x00007E00+eax+0x34] ;image base

  xor ebx, ebx
  mov bx, [0x00007E00+eax+0x14];optional Header Size
  add ebx, 0x00007E00+0x18
  add ebx, eax; ebx - section table

  mov eax,[0x00007E00+eax+0x28]
  add eax, edx ;eax - AddressOfEntryPoint

  .l1:

  push ecx

  mov ecx, [ebx+0x10];SizeOfRawData
  mov edi, [ebx+0xC];VirtualAdress
  mov esi, [ebx+0x14];PointerToRawData

  add edi, edx
  add esi, 0x00007E00

  cld
  rep movsb 

  add ebx, 0x28 
  pop ecx

  cmp ecx, 5
  loop .l1

  ret

print_something32: ;remove later

  lea eax, [0xb8000]  ; mov eax, 0xb8000
  
  mov dword [eax], 0x41414141

  jmp $

  ret

[bits 16]


