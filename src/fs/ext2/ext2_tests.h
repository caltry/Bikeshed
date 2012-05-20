/*
 * A collection of tests for various ext2 functionality (and bug reports).
 *
 * Author: David Larsen <dcl9934@cs.rit.edu>, May 2012
 */

#ifndef _EXT2_TESTS_H
#define _EXT2_TESTS_H

/*
 * Run all of the tests and return the number of failed tests.
 *
 * Some tests may print to serial out.
 */
Uint32 test_all(void);

#endif //_EXT2_TESTS_H
