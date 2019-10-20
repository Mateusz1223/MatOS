#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>

#define UNUSED(a) (void)(a)

void *memmove (void *dest, const void *src, size_t size);
void *memset(void *dest, int c, size_t n);
