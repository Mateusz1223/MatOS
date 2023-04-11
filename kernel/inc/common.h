#pragma once
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

#define UNUSED(a) (void)(a)

void *memmove(void *dest, const void *src, size_t size);
void *memsetk(void *dest, uint8_t c, size_t n);

int strcmp(const char * str1, const char * str2); // returns 0 if strings different, 1 if the same

int strcmpn(const char * str1, const char * str2, uint32_t n); // returns 0 if strings different, 1 if the same
