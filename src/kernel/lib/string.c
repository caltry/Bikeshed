/*
 * String manipulation functions.
 */

#include "string.h"

static const char null_chr = '\0';

const char* strchr( const char *str, char c )
{
	while( *str != null_chr && *str != c )
	{
		str++;
	}

	return str;
}

int strncmp( const char *s1, const char *s2, unsigned int n )
{
	for(	unsigned int i = 0;
		i < n && s1 != '\0' && s2 != '\0';
		++i, ++s1, ++s2 )
	{
		if( s1 != s2 )
		{
			return s1-s2;
		}
	}

	return 0;
}
