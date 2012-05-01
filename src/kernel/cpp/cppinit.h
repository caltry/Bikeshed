#ifndef __CPP_INIT_H__
#define __CPP_INIT_H__

/* Initializes C++ support by calling all of the C++
 * static/global objects constructors
 */
extern void __cpp_init(void);

/* Deinitializes C++ support by calling all of the C++
 * static/global objects destructors
 */
extern void __cpp_deinit(void);

#endif
