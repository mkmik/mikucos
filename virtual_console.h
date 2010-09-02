#ifndef _VIRTUAL_CONSOLE_H_
#define _VIRTUAL_CONSOLE_H_

#include <types.h>
#include <device.h>
#include <virtual_console_ansi.h>

extern void virtual_console_init();

extern void virtual_console_device_init();

extern ssize_t virtual_console_read(struct device *this, void* buffer, 
				    size_t len, off_t *off);
extern ssize_t virtual_console_write(struct device *this, const void* buffer, 
				    size_t len, off_t *off);
extern int virtual_console_set_device_option(device_t *this, 
					     const char* name, va_list args);
extern int virtual_console_get_device_option(device_t *this, 
					     const char* name, va_list args);

// pseudo private

extern void virtual_console_putchar_sm(device_t* this, char ch, 
				       ansi_state_machine_t *sm);
extern void virtual_console_putchar(device_t* this, char ch);

extern void virtual_console_draw_char(device_t* this, char ch, int row, int col);
extern void virtual_console_new_line(device_t* this);
extern void virtual_console_cls(device_t* this);
extern void virtual_console_set_position(device_t* this, int row, int col);
extern void virtual_console_sync_cursor(device_t* this);
extern void virtual_console_reset(device_t *this);
extern void virtual_console_backspace(device_t *this);
/** move relative to current position 
 * doesn't go out of the screen, doesn't wrap. **/
extern void virtual_console_move_cursor(device_t *this, int rrow, int rcol);

#endif
