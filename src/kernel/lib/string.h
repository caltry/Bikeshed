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
const char* _kstrchr( const char *str, char c );

/*
 * Find the last occurrence of the character _c_ in _str_.
 *
 * Returns a pointer to the location of the last _c_ in _str_ or a '\0' if none
 * was found.
 */
const char* _kstrrchr( const char *str, char c);

/*
 * Compare the two strings lexicographically.
 */
int _kstrcmp( const char *s1, const char *s2 );

/*
 * Compare the two strings up, but only compare up to _n_ numbers.
 */
int _kstrncmp( const char *s1, const char *s2, unsigned int n );

/*
 * Find the length of a cstring.
 */
unsigned int _kstrlen( const char *string );

#endif //_STRING_H_
