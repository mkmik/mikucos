#include <gdt.h>
#include <libc.h>

// from acaros

mal_segment_t mal_gdtTable[256] __attribute__((aligned(8)));

mal_segment_t* mal_descriptorAtSelector(mal_selector_t sel) {
  return &mal_gdtTable[sel/sizeof(mal_desc_t)];
}

void mal_setSegmentBase(mal_segment_t *seg, pointer_t base) {
  seg->base_lo = base & 0xFFFF;
  seg->base_me = (base >> 16) & 0xFF;
  seg->base_hi = base >> 24;
}

void mal_setSegmentLimit(mal_segment_t *seg, size_t size) {
  if(size >= (1<<20)) {
    seg->flags |= FLAG_GRAN4K;
    size >>= 12;
  }
  seg->limit_lo = size & 0xFFFF;
  seg->limit_hi = (size >> 16) & 0xF;
}

struct {
  u_int16_t len;
  mal_segment_t* table;
} PACKED mal_gdtDesc __attribute__((aligned(4))) = {256*8-1, mal_gdtTable} ;

void mal_gdtInit() {
  //  xprintf("Initializing GDT\n");

  mal_segment_t *ds = mal_descriptorAtSelector(__KERNEL_DS);
  mal_setSegmentBase(ds, 0);
  mal_setSegmentLimit(ds, 0xFFFFFFFF);
  ds->flags |= FLAG_32BIT;
  ds->type = TYPE_DATA | DATA_RW;
  ds->present = 1;

  mal_segment_t *cs = mal_descriptorAtSelector(__KERNEL_CS);;
  mal_setSegmentBase(cs, 0);
  mal_setSegmentLimit(cs, 0xFFFFFFFF);
  cs->flags |= FLAG_32BIT;
  cs->type = TYPE_CODE | CODE_READABLE;
  cs->present = 1;

  asm("lgdt %0" : : "m"(mal_gdtDesc));
  asm("ljmp %0, $1f\n"
      "1: movw %w1, %%ds\n" 
      "movw %w1, %%ds\n"
      "movw %w1, %%ss\n"
      "movw %w1, %%es\n"
      "movw %w1, %%fs\n"
      "movw %w1, %%gs\n" 
      :: "i"(__KERNEL_CS), "r"(__KERNEL_DS));
}

