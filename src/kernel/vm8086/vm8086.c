#include "vm8086.h"
#include "serial.h"
#include "scheduler.h"


void __vm8086_init(void)
{
	__install_isr(INT_VEC_GPF, _isr_gpf);
}

void _isr_gpf(int vector, int code)
{
	UNUSED(vector);

	if (code == 0)
	{
		// Not a segment issue
		serial_printf("Instruction Pointer: %x\n", _current->context->eip);
	} else {
		// Segment issue
		serial_printf("Segment: %d\n", code);
	}

	asm ("cli");
	asm ("hlt");
}
