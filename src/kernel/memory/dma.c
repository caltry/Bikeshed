#include "dma.h"

typedef struct Stack_Node
{
	struct Stack_Node* next;
} dma_stack_node_t;

typedef struct Stack
{
	dma_stack_node_t* stack_start;
	Uint32 stack_size;
} dma_stack_t;

dma_stack_t dma_stack;

static void* __dma_pop_stack()
{

}

static void __dma_push_stack(void* address)
{

}

void __dma_init()
{
	// Push all the valid addresses onto the stack
}
