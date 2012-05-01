#include "cpptest.hpp"
extern "C"
{
	#include "../serial.h"

	void _test_cpp(void)
	{
		Object obj(10);
		const char* output_test = "CPP SUPPORT EXISTS!\n";
		serial_printf(output_test);
		serial_printf("Object before: %d\n", obj.get_i());
		obj.increment();
		serial_printf("Object after: %d\n", obj.get_i());
	}
}

Object::Object(int i) : _i(i)
{ 
	serial_printf("Object constructing!\n");
}

Object::~Object()
{
	serial_printf("Object destructing!\n");
}

void Object::increment()
{
	_i++;
}

int Object::get_i()
{
	return _i;
}
