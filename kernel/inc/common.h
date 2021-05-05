#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

#define UNUSED(a) (void)(a)

void *memmove (void *dest, const void *src, size_t size);
void *memsetk(void *dest, uint8_t c, size_t n);
