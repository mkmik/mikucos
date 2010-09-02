#ifndef __MALLOC_H__
#define __MALLOC_H__

#include <types.h>
#include <vmalloc/vmhdr.h>
#include <vmalloc/vmalloc.h>

/** initializes heap memory management structures */
void malloc_init();

/** returns "size" bytes from fixed heap. cannot be freed */
void* hmalloc(size_t size);

// debug allocators
void *fucking_malloc(size_t size);
void *fucking_calloc(size_t memb, size_t size);
void fucking_free(void*arg);
/** panic if trying to free this address.
 * @arg str is displayed when free catches the access */
void debug_memory_lock(void*arg, char* str);

#if MALLOC_DEBUGGER
# define free(arg) fucking_free(arg)
# define real_malloc(arg) vmalloc(Vmheap, arg)
# ifdef malloc
#  undef malloc
# endif
# ifdef calloc
#  undef calloc
# endif
# define malloc(arg) fucking_malloc(arg)
# define calloc(memb, size) fucking_calloc(memb, size)
#else
# define debug_memory_lock(arg, str) do {} while(0)
#endif

#endif
