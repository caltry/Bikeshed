/* NOTE: This file assumes that __kmalloc() is initialized and working!!!!
 */

extern "C"
{
	#include "memory/kmalloc.h"
	#include "serial.h"
	#include "defs.h"

	struct atexit_entry
	{
		void (*destructor) (void*);
		void* argument;
		void* dso_handle;

		atexit_entry* next;
	};

	static atexit_entry* head_at_exit_entry = NULL;
	static atexit_entry* tail_at_exit_entry = NULL;

	/* Required by GCC */
	void* __dso_handle;

	/* Required if you want to have static/global object support by GCC
	 *
	 * Inserts the destructor and argument into a table
	 */
	int __cxa_atexit(void (*destructor) (void *), void* arg, void* __dso_handle)
	{
		serial_printf("At exit called: %x\n", destructor);

		// Create and fill the entry
		atexit_entry* entry = (atexit_entry *)__kmalloc(sizeof(atexit_entry));
		entry->destructor = destructor;
		entry->argument = arg;
		entry->dso_handle = __dso_handle;
		entry->next = NULL;

		if (head_at_exit_entry == NULL)
		{
			head_at_exit_entry = tail_at_exit_entry = entry;
		} else {
			tail_at_exit_entry->next = entry;
			tail_at_exit_entry = entry;
		}

		return 0; /* 0 means success */
	}

	/* Called to destruct and object */
	void __cxa_finalize(void* func)
	{
		if (func == NULL)
		{
			// We need to call all of the destructors!
			atexit_entry* current = head_at_exit_entry;
			while (current != NULL)
			{
				if (current->destructor != NULL)
				{
					// Call the destructor
					current->destructor(current->argument);
				}

				// Cleanup this entry
				atexit_entry* temp = current->next;
				__kfree(current);
				current = temp;
			}

			// Set the head and tail pointers to NULL just in case
			head_at_exit_entry = tail_at_exit_entry = NULL;
		} else {
			// Loop through the destructors until we find the one we're 
			// looking for. We also remove the destructor from the list so
			// it can't be called a second time, which isn't allowed
			atexit_entry* current  = head_at_exit_entry;	
			atexit_entry* previous = tail_at_exit_entry;
			while (current != NULL)
			{
				if (current->destructor == func)
				{
					// Call this destructor!
					current->destructor(current->argument);

					// Now we need to cleanup this node
					if (current == head_at_exit_entry)
					{
						// We're at the beginning of the list
						head_at_exit_entry = current->next;
						__kfree(current);

						// Fix the tail if necessary
						if (head_at_exit_entry == NULL) { head_at_exit_entry = NULL; }
					}
					else if (current == tail_at_exit_entry)
					{
						// We're at the end of the list
						tail_at_exit_entry = previous;
						previous->next = NULL;
						__kfree(current);
					}
					else
					{
						// We're in the middle of the list
						atexit_entry* temp = current->next;
						__kfree(current);
						previous->next = temp;
					}

					// Stop looping, we found what we were looking for
					return; 
				}

				// Move onto the next node
				previous = current;
				current = current->next;
			}
		}
	}
}
