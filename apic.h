#ifndef __APIC_H__
#define __APIC_H__

#include <x86.h>

#define APIC_LOCAL_BASE 0xFEE00000

#define APIC_MASKED_MASK (1<<16)

void apic_mask(int* lvt);
void apic_unmask(int* lvt);
int apic_get_masked(int* lvt);

#define APIC_TIMER_LVT (int*)(APIC_LOCAL_BASE | 0x20)
#define APIC_TIMER_DIVIDER (int*)(APIC_LOCAL_BASE | 0xE0)
#define APIC_TIMER_INITIAL_COUNT (int*)(APIC_LOCAL_BASE | 0x80)
#define APIC_TIMER_CURRENT_COUNT (int*)(APIC_LOCAL_BASE | 0x90)

#define APIC_TIMER_MODE_MASK (1<<17)

enum {APIC_TIMER_ONESHOT, APIC_TIMER_PERIODIC};

void apic_set_timer(int initial_count, 
		    int divider, 
		    int mode);
void apic_set_timer_vector(int vector);

int apic_get_timer_count();
int apic_get_timer_initial_count();
int apic_get_timer_divider();
int apic_get_timer_mode();
int apic_get_timer_vector();

#endif
