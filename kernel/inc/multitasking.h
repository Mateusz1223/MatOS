// This is far from being ready

#pragma once
#include "inc/common.h"

typedef struct CPUState CPUState;

struct CPUState
{
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;

	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;

	/*
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;
	*/
	uint32_t error;

	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t esp;
	uint32_t ss;        
} __attribute__((packed));

void multitasking_init();

CPUState *multitasking_schedule(CPUState *cpu_state);

bool multitasking_add_task(void entrypoint()); //returns false when failed to add a task

void multitasking_RTC_irq_resident(CPUState *cpu_state);