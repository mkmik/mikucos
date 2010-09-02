#include <malloc.h>
#include <hal.h>
#include <libc.h>
#include <list.h>

/* this implementation is only a temporary hack until the memory
 * handling is done */

#define POOL_SIZE (4096*100)

static char malloc_pool[POOL_SIZE];
static char* alloc_base = malloc_pool;

Void_t* memoryf(Vmalloc_t *region, Void_t* obj,
		   size_t csz, size_t nsz, Vmdisc_t *disc) {
  uprintf("called memoryf, %d, %d\n", csz, nsz);
  if(csz == 0)
    return hmalloc(nsz);
  else {
    return 0;
  }
}

Vmdisc_t disc = {memoryf, 0, PAGE_SIZE*2};

void malloc_init() {
  Vmregion->disc = &disc;
}

void *hmalloc(size_t size) {
  void *res = alloc_base;
  alloc_base += size;
  if(alloc_base >= malloc_pool + POOL_SIZE) {
    xprintf("HMALLOC POOL FULL addr %d size %d!\n", alloc_base-malloc_pool, POOL_SIZE);
    panicf("HMALLOC POOL FULL\n");
  }
  //  printf("HMALLOC called %d (%x)\n", size, size);
  return res;
}

#if MALLOC_DEBUGGER
struct mem_block {
  void* addr;
  char* str;
  struct list_head list;
};

LIST_HEAD(allocated_blocks);
LIST_HEAD(free_blocks);
LIST_HEAD(locked_blocks);

void* fucking_calloc(size_t memb, size_t size) {
  void *addr = fucking_malloc(size*memb);
  memset(addr, 0, size*memb);
  return addr;
}

void* fucking_malloc(size_t size) {
  void* addr = real_malloc(size);
  //  return addr;
  if(!addr)
    return 0;

  struct mem_block *f = real_malloc(sizeof(struct mem_block));
  f->addr = addr;
  list_add(&f->list, &allocated_blocks);
  //  uprintf("adding block %p to allocated_blocks\n", f);
  return addr;
}

void fucking_free(void *addr) {
  struct list_head *pos;
  struct mem_block *block = 0;

  list_for_each(pos, &free_blocks) {
    if((list_entry(pos, struct mem_block, list))->addr == addr) {
      xprintf("!!!!!!!!!double free of %p\n");;
      panicf("DOUBLE FREE\n");
    }
  }

  list_for_each(pos, &allocated_blocks) {
    void *a = (list_entry(pos, struct mem_block, list))->addr;
    //    uprintf("iterating allocated blocks: %p vs. %p\n", a, addr);
    if(a == addr) {
      block = list_entry(pos, struct mem_block, list);
      break;
    }
  }
  if(!block) {
    xprintf("!+!+!+!+!+!freeing not allocated block %p\n", addr);
    panicf("NOT ALLOCATED\n");
  }

  list_for_each(pos, &locked_blocks) {
    if((list_entry(pos, struct mem_block, list))->addr == addr) {
      xprintf("********* ADDRESS AT %p is LOCKED. message: '%s'\n", 
	      (list_entry(pos, struct mem_block, list))->addr,
	      (list_entry(pos, struct mem_block, list))->str);
      panicf("LOCKED\n");
    }
  }

  list_del(&block->list);
  list_add(&block->list, &free_blocks);

}

void debug_memory_lock(void*addr, char* str) {
  struct mem_block *f = real_malloc(sizeof(struct mem_block));
  f->addr = addr;
  f->str = str;
  list_add(&f->list, &locked_blocks);
}
#endif

