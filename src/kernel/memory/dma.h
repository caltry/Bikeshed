#ifndef __DMA_H__
#define __DMA_H__

#include "headers.h"

/* Initialize the DMA memory allocator
 */
void __dma_init();

void* __dma_get_address();

void __dma_free_address(void* address);

#endif
