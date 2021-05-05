#include "inc/common.h"

void *memmove (void *dest, const void *src, size_t size)
{
	uint8_t *dest_byte = (uint8_t*)dest;
	const uint8_t *src_byte = (const uint8_t*)src;
	
	if(dest < src)
	{
		for(size_t i=0; i<size; i++)
			dest_byte[i] = src_byte[i];
	}else{
		for(size_t i=size; i>0; i--)
			dest_byte[i-1] = src_byte[i-1];
	}
	return dest;
}

void *memsetk(void *dest, uint8_t c, size_t n)
{
	uint8_t *dest_byte = (uint8_t*)dest;
	for(size_t i=0; i<n; i++)
	{
		dest_byte[i] = c;
	}
	return dest;
}