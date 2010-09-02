#ifndef _PLATFORM_MAL_IDT_H_
#define _PLATFORM_MAL_IDT_H_

#include <desc.h>

// from acaros

typedef u_int16_t mal_vector_t;

void mal_idtInit();
void mal_setGateOffset(mal_gate_t *gat, pointer_t offset);
mal_gate_t* mal_gateAtVector(mal_vector_t vec);
void mal_setInterruptVector(mal_vector_t vec, void* isr);
void mal_setInterrupt(mal_vector_t vec, void* isr, int fixup);

/** attach 'isr' to vector 'vec'. it expects to be called 
 * as an normal interrupt */
void mal_attachIsr(mal_vector_t vec, void* isr);
/** attach 'isr' to vector 'vec'. it expects to be called 
 * as an exception (with additional flags pushed on the stack) */
void mal_attachException(mal_vector_t vec, void* isr);

#endif
