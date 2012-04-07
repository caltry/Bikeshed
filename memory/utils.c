#include "utils.h"

void _kmemset(void *ptr, unsigned char value, uint32 num)
{
	unsigned char *p = (unsigned char *)ptr;
	while (num > 0)
	{
		*p = value;
		++ptr;
		--num;
	}
}

void _kmemcpy(void *dest, const void *source, uint32 num)
{
	unsigned char *d = (unsigned char *)dest;
	unsigned char *s = (unsigned char *)source;
	while (num > 0)
	{
		*d = *s;
		++d;
		++s;
		--num;
	}
}

int32 _kmemcmp(const void *ptr1, const void *ptr2, uint32 num)
{
	unsigned char *p1 = (unsigned char *)ptr1;
	unsigned char *p2 = (unsigned char *)ptr2;
	while (num > 0)
	{
		if (*p1 != *p2)
		{
			return *p1 - *p2;	
		}
		++p1;
		++p2;
		--num;
	}

	return 0;
}
