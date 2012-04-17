/*
 * C Pre-processor directives to give hints to GCC or do build time error
 * checking.
 *
 * David Larsen <dcl9934@cs.rit.edu>, April 2012
 */

/*
 * Compile time error checking, from the Linux kernel.
 */
#define COMPILE_ERROR_ON( condition ) ((void)sizeof(char[1 - 2*!!(condition)]))

/*
 * Hint to the optimizer that we're likely to take this branch.
 */
#define likely( condition ) __builtin_expect( !!(condition), 1 )

/*
 * Hint to the optimizer that we're unlikely to take this branch
 */
#define unlikely( condition ) __builtin_expect( !!(condition), 0 )
