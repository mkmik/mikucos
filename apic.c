#include <apic.h>

void apic_mask(int* lvt) {
  *lvt |= APIC_MASKED_MASK;
}

void apic_unmask(int* lvt) {
  *lvt &= ~APIC_MASKED_MASK;
}

int apic_get_masked(int* lvt) {
  return (*lvt & APIC_MASKED_MASK) != 0;
}

void apic_set_timer(int initial_count, 
		    int divider, 
		    int mode) {
  *APIC_TIMER_INITIAL_COUNT = initial_count;
  *APIC_TIMER_DIVIDER = divider;
  
  if(mode == APIC_TIMER_ONESHOT)
    *APIC_TIMER_LVT &= ~APIC_TIMER_MODE_MASK;
  else
    *APIC_TIMER_LVT |= APIC_TIMER_MODE_MASK;
}

int apic_get_timer_count() {
  return *APIC_TIMER_CURRENT_COUNT;
}

int apic_get_timer_initial_count() {
  return *APIC_TIMER_INITIAL_COUNT;
}

int apic_get_timer_divider() {
  return *APIC_TIMER_DIVIDER;
}

int apic_get_timer_mode() {
  return *APIC_TIMER_LVT = *APIC_TIMER_LVT & APIC_TIMER_MODE_MASK ?
    APIC_TIMER_PERIODIC : APIC_TIMER_ONESHOT;
}

void apic_set_timer_vector(int vector) {
  *APIC_TIMER_LVT |= (vector & 0xFF);
}

int apic_get_timer_vector() {
  return *APIC_TIMER_LVT & 0xFF;
}
