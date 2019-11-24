#pragma once
#include "inc/common.h"
#include "inc/HAL.h"

void PIC_sendEOI(unsigned char irq);

void PIC_remap(int offset1, int offset2);

void IRQ_set_mask(unsigned char IRQline);

void IRQ_clear_mask(unsigned char IRQline);

static uint16_t __pic_get_irq_reg(int ocw3);

uint16_t pic_get_irr(void);

uint16_t pic_get_isr(void);