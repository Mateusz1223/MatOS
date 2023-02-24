#pragma once
#include "inc/common.h"

void outb(uint16_t port, uint8_t data);
void outw(uint16_t port, uint16_t data);
void outd(uint16_t port, uint32_t data);

uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t ind(uint16_t port);

void insb(uint16_t port, uint8_t *buffer, int n); // ???? Used in ATA driver
void insw(uint16_t port, uint16_t *buffer, int n);
void insd(uint16_t port, uint32_t *buffer, int n);

void io_wait(void);