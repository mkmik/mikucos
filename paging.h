#ifndef __PAGING_H__
#define __PAGING_H__

#include <x86.h>
#include <hal.h>

#define PTE_PSE      0x80 // 4MB page
#define PTE_USER     0x4
#define PTE_RW       0x2
#define PTE_PRESENT  0x1

/** initialize identity paging of first 4MB of ram */
int init_paging();

/** page aligned buffer containing page directory entries (pte_t)
 * which points to page tables */
extern pte_t *page_directory;
/** page table for the identity mapping of the first 4M of physical memory */
extern pte_t *identity_page_table;

/** modify the physical address field in the pte */
void set_pte_base(pte_t *pte, void *base);
/** modify the flags of the pte  */
void set_pte_flags(pte_t *pte, unsigned short flags);
/** get the physical address of the page pointed by the pte */
void *get_pte_base(pte_t *pte);
/** get the pte flags */
unsigned short get_pte_flags(pte_t *pte);

/** extract the page directory index of addr */
#define page_dir_index(addr) ((u_int32_t)addr >> 22)
/** extract the page table index of addr */
#define page_table_index(addr) (((u_int32_t)addr >> 12) & 0x3FF)

#endif
