#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <spin_lock.h>
#include <device.h>

void keyboard_init();

extern ssize_t keyboard_read(struct device *this, void* buffer, 
			     size_t len, off_t *off);

void keyboard_device_init(device_t *this);
int keyboard_set_device_option(device_t *this, const char* name, va_list args);


#define KEYLED_SCROLL_LOCK (1<<0)
#define KEYLED_NUM_LOCK (1<<1)
#define KEYLED_CAPS_LOCK (1<<2)

#define MODIFIER_SHIFT (1<<0)
#define MODIFIER_CTRL (1<<1)
#define MODIFIER_ALT (1<<2)
#define MODIFIER_ALTGR (1<<3)
#define MODIFIER_COMPOSE (1<<4)

void keyboard_set_led(int mask);
char keyboard_getch();
/** returns the last scancode */
int keyboard_get_key();

void keyboard_isr();

extern void keyboard_enqueue(char ch);
extern char keyboard_dequeue();
#endif
