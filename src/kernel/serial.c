#include "serial.h"
#include "startup.h"

#define SERIAL_PORT_A 0x3F8

void serial_install(void)
{
	__outb(SERIAL_PORT_A + 1, 0x00);
	__outb(SERIAL_PORT_A + 3, 0x80);
	__outb(SERIAL_PORT_A + 0, 0x03);
	__outb(SERIAL_PORT_A + 1, 0x00);
	__outb(SERIAL_PORT_A + 3, 0x03);
	__outb(SERIAL_PORT_A + 2, 0xC7);
	__outb(SERIAL_PORT_A + 4, 0x08);
}

int serial_transmit_empty(void)
{
	return __inb(SERIAL_PORT_A + 5) & 0x20;
}

void serial_char(char out)
{
	while (serial_transmit_empty() == 0);
	__outb(SERIAL_PORT_A, out);
}

void serial_string(const char *out)
{
	while (*out != 0)
	{
		serial_char(*out);
		++out;
	}
}

static const char digits [] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static void ui2a(unsigned int num, unsigned int base, int uc)
{
	int n = 0;
	unsigned int d = 1;
	while ((num / d) >= base)
	{
		d *= base;
	}

	while (d != 0) 
	{
		int dgt = num / d;
		num %= d;
		d /= base;
		if (n || dgt > 0 || d == 0) {
			serial_char(dgt + (dgt < 10 ? '0' : (uc ? 'A' : 'a') - 10));
			++n;
		}
	}
}

void serial_printf(char *string, unsigned long val)
{
	while (*string != 0)
	{
		if (*string == '%')
		{
			string++;
			if (string == 0)
				return;
		
			switch (*string)
			{
				case '%':
					serial_char(*string);
					break;
				case 'x':
				case 'X':
				{
					serial_string("0x");
					char total[17];
					total[16] = 0;
					int count = 15;
					unsigned long hex = val; 
					while (hex != 0 && count >= 0)
					{
						total[count] = digits[hex % 16];
						hex /= 16;
						count--;
					}
					while (count >= 0)
					{
						total[count] = digits[0];
						count--;
					}
					serial_string(total);
				}
					break;
				case 'd':
					{
						ui2a(val, 10, 0);
					}
					break;
			}
			string++;
			continue;
		}
		
		serial_char(*string);
		string++;
	}
}
