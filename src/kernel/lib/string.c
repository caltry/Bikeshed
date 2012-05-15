/*
 * String manipulation functions.
 */

#include "string.h"

static const char null_chr = '\0';

const char* _kstrchr( const char *str, char c )
{
	while( *str != null_chr && *str != c )
	{
		str++;
	}

	return str;
}

const char* _kstrrchr( const char *str, char c )
{
	const char* last_location = _kstrchr( str, c );

	while( *str != null_chr )
	{
		if( *str == c )
		{
			last_location = str;
		}
		str++;
	}

	return last_location;
}

int _kstrncmp( const char *s1, const char *s2, unsigned int n )
{
	for
	(unsigned int i = 0;
	i < n && *s1 != null_chr && *s2 != null_chr;
	++i, ++s1, ++s2 )
	{
		if( *s1 != *s2 )
		{
			return (*s1)-(*s2);
		}
	}

	return 0;
}

unsigned int _kstrlen( const char *string )
{
	unsigned int i = 0;
	while( *string++ )
	{
		++i;
	}

	return i;
}
