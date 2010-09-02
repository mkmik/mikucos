#ifndef __MISC_H__
#define __MISC_H__

#define BIT_ASSIGN(value, bit, en) ((en) ? ((value) | (1<<(bit))) : ((value) & ~(1<<(bit))))
#define BIT_SET(value, bit) BIT_ASSIGN(value, bit, 1)
#define BIT_CLEAR(value, bit) BIT_ASSIGN(value, bit, 0)
#define BIT_TEST(value, bit) (value & (1<<bit))

#define MIN(a,b) ({__typeof__(a) aa = (a); __typeof__(b) bb = (b); (aa < bb) ? (aa) : (bb);})
#define MAX(a,b) ({__typeof__(a) aa = (a); __typeof__(b) bb = (b); (aa > bb) ? (aa) : (bb);})

#endif
