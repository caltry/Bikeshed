/*
** SCCS ID:	@(#)headers.h	1.1	4/5/12
**
** File:	headers.h
**
** Author:	4003-506 class of 20113
**
** Contributor:
**
** Description:	Standard includes needed in all C source files
*/

#ifndef _HEADERS_H
#define _HEADERS_H

#ifndef __ASM__20113__

// only pull these in if we're not in assembly language

#include "defs.h"
#include "types.h"

#include "c_io.h"
#include "support.h"


#ifdef __KERNEL__20113__

// OS needs the kernel library headers and the system headers
#include "system.h"
#include "klib.h"

#else

// User mode needs only the user library headers
#include "ulib.h"

#endif

#endif

#endif
