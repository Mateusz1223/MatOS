#pragma once
#include "include/common.h"
#include "include/int_asm_handlers.h"
#include "include/PIC.h"

//#pragma pack(push,1)
struct IDTEntry
{
   uint16_t offset_0_15; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t flags; // type and attributes, see below
   uint16_t offset_16_31; // offset bits 16..31
}__attribute__((packed));

struct IDTR
{
	uint16_t limit;
	uint32_t base;
}__attribute__((packed));
//#pragma pack(pop)

typedef struct IDTEntry IDTEntry;
typedef struct IDTR IDTR;


void idt_init();