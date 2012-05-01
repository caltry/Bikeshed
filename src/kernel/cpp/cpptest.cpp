extern "C"
{
	#include "../serial.h"

	void _test_cpp(void)
	{
		const char* output_test = "CPP SUPPORT EXISTS!\n";
		serial_printf(output_test);
	}
}
