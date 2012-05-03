/*
 * String manipulation functions.
 */

#ifndef _STRING_H_
#define _STRING_H_

/*
 * Find the first occurrence of the character _c_ in _str_.
 *
 * Returns a pointer to the location of _c_ in _str_ or a '\0' if none was
 * found.
 */
const char* strchr( const char *str, char c );

/*
 * Compare the two strings up, but only compare up to _n_ numbers.
 */
int strncmp( const char *s1, const char *s2, unsigned int n );

#endif //_STRING_H_
