#include <x86.h>

#define _set_tssldt_desc(n,addr,limit,type) \
__asm__ __volatile__ ("movw %w3,0(%2)\n\t" \
        "movw %%ax,2(%2)\n\t" \
        "rorl $16,%%eax\n\t" \
        "movb %%al,4(%2)\n\t" \
        "movb %4,5(%2)\n\t" \
        "movb $0,6(%2)\n\t" \
        "movb %%ah,7(%2)\n\t" \
        "rorl $16,%%eax" \
        : "=m"(*(n)) : "a" (addr), "r"(n), "ir"(limit), "i"(type))

void set_tss_desc(struct segment_desc* desc, void *addr) {
  _set_tssldt_desc(desc, (int)addr, 235, 0x89);
}
