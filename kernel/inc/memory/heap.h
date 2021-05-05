#pragma once
#include "inc/common.h"

void heap_init(void *addr, size_t size); // size in bytes

void *heap_malloc(size_t size);

void heap_free(void *addr);