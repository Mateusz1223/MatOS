#pragma once

#include "inc/common.h"

void PIC_init();

void PIC_sendEOI(uint8_t irq);

void IRQ_set_mask(uint8_t IRQline);

void IRQ_clear_mask(uint8_t IRQline);

uint16_t pic_get_irr(void);

uint16_t pic_get_isr(void);

void PIC_handler();