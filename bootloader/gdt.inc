GDT_ADRESS:
dw (GDT_END - GDT)-1
dd 0x00000 + GDT

GDT:
dq 0 ;null descriptor

dd 0x0000FFFF
dd (10 << 8) | (1 << 12) | (1 << 15) | (0xF << 16) | (1 << 22) | (1 << 23);code segment, ring 0

dd 0x0000FFFF
dd (2 << 8) | (1 << 12) | (1 << 15) | (0xF << 16) | (1 << 22) | (1 << 23);data segment, ring 0

GDT_END: