#include "vm8086.h"

void _vm8086_init(void)
{
	__install_isr(INT_VEC_GPF, _isr_gpf);
}

void _isr_gpf(int vector, int code)
{

}
