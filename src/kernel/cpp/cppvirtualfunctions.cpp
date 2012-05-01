// This file is required in order to support virtual functions
// in C++. The function defined here is called if the virtual
// function to be called can't be found. This should never happen
// because you shouldn't be able to construct an object with
// a pure virtual function. If this is the case we perform a 
// kernel panic because something is very wrong

extern "C"
{
	#include "lib/klib.h"
}

// Implementation and information is taken from http://wiki.osdev.org/C%2B%2B
extern "C" void __cxa_pure_virtual()
{
	// Error because this should be impossible
	_kpanic("C++", "Pure virtual function implementation not found!\n", 0);
}
