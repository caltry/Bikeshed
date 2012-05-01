// The implementation for our C++ support. It simply forwards
// calls to __kmalloc()


extern "C"
{
	#include "memory/kmalloc.h"
}

// We need to define size_t
typedef unsigned int size_t;

void* operator new(size_t size)
{
	return __kmalloc(size);
}

void* operator new[](size_t size)
{
	return __kmalloc(size);
}

void operator delete(void* p)
{
	__kfree(p);
}

void operator delete[](void* p)
{
	__kfree(p);
}
