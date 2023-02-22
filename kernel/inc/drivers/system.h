#pragma once
#include "inc/common.h"
#include "inc/UI/terminal.h"

struct systemInfo{
	char CPUVendor[13];
	char CPUBrandStr[48];
	uint32_t CPUModel;
}System;

void system_init();

void system_print_informations(Terminal *term);