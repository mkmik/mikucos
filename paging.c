#include <paging.h>

#include <init.h>


static char page_directory_buffer[PAGE_SIZE*2];
static char page_table_buffer[PAGE_SIZE*2];

pte_t *page_directory;
pte_t *identity_page_table;


void set_pte_base(pte_t *pte, void *base) {
  pte->data &= ~(0xFFF);
  pte->data |= (u_int32_t)base & ~(0xFFF);
}

void set_pte_flags(pte_t *pte, unsigned short flags) {
  pte->data &= ~(0x1FF);
  pte->data |= flags;
}

void *get_pte_base(pte_t *pte) {
  return (void*)(pte->data & ~(0xFFF));
}

unsigned short get_pte_flags(pte_t *pte) {
  return pte->data & ~(0x1FF);
}

int init_paging() {
  int i;
  page_directory = (pte_t*)((u_int32_t)(page_directory_buffer + PAGE_SIZE) & ~0xFFF);
  identity_page_table = (pte_t*)((u_int32_t)(page_table_buffer + PAGE_SIZE) & ~0xFFF);

  set_pte_base(&page_directory[0], identity_page_table);
  set_pte_flags(&page_directory[0], PTE_RW | PTE_PRESENT | PTE_USER);

  for(i=0; i<1024; i++) {
    set_pte_base(&identity_page_table[i], (void*)(i * PAGE_SIZE));
    set_pte_flags(&identity_page_table[i], PTE_RW | PTE_PRESENT);
  }

  WRITE_PAGE_DIRECTORY(page_directory);
  //  ENABLE_PAGING(); 

  return 0;
}
