#include "cpptest.hpp"
extern "C"
{
	#include "../serial.h"

	void _test_cpp(void)
	{
		Object obj(10);
		const char* output_test = "CPP SUPPORT EXISTS!\n";
		serial_printf(output_test);
		serial_printf("Object 1 before: %d\n", obj.get_i());
		obj.increment();
		serial_printf("Object 1 after: %d\n", obj.get_i());

		serial_printf("Starting new test:\n");
		Object* obj2 = new Object(20);
		serial_printf("Object 2 before: %d\n", obj2->get_i());
		obj2->increment();
		serial_printf("Object 2 after: %d\n", obj2->get_i());
		delete obj2;
	}
}

Object::Object(int i) : _i(i)
{ 
	serial_printf("Object %d constructing!\n", i);
}

Object::~Object()
{
	serial_printf("Object %d destructing!\n", _i);
}

void Object::increment()
{
	_i++;
}

int Object::get_i()
{
	return _i;
}
