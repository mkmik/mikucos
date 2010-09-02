#ifndef _VIRTUAL_CONSOLE_LINE_MODE_H_
#define _VIRTUAL_CONSOLE_LINE_MODE_H_

#include <virtual_console_ansi.h>

struct virtual_console_line_mode {
  char line_buffer[4096];
  int idx;
};
typedef struct virtual_console_line_mode virtual_console_line_mode_t;

extern void virtual_console_line_mode_new_line(device_t* this);
extern void virtual_console_line_mode_cls(device_t* this);
extern void virtual_console_line_mode_move_cursor(device_t *this, int rrow, int rcol);
extern void virtual_console_line_mode_backspace(device_t *this);
extern void virtual_console_line_mode_draw_char(device_t* this, char ch, 
						int row, int col);
extern ansi_sm_ops_t virtual_console_ansi_sm_line_mode_ops;

#endif
