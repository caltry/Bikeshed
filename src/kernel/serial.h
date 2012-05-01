#ifndef __SERIAL_H__
#define __SERIAL_H__

void serial_char(char);

void serial_string(const char *);

void serial_install(void);

void serial_printf(const char *fmt, ...);
#endif
