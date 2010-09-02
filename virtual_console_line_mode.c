#include <virtual_console.h>
#include <virtual_console_line_mode.h>
#include <virtual_console_private.h>

ansi_sm_ops_t virtual_console_ansi_sm_line_mode_ops = {
  .reset = virtual_console_reset,
  .move_cursor = virtual_console_line_mode_move_cursor,
  .cls = virtual_console_line_mode_cls,
  .new_line = virtual_console_line_mode_new_line,
  .draw_char = virtual_console_line_mode_draw_char,
  .sync_cursor = virtual_console_sync_cursor,
  .backspace = virtual_console_line_mode_backspace
};

static void add_to_buffer(device_t* this, char ch) {
  virtual_console_line_mode_t *lm = &private(this)->line_mode;
  lm->line_buffer[lm->idx++] = ch;
}

static void backspace_buffer(device_t* this) {
  virtual_console_line_mode_t *lm = &private(this)->line_mode;
  lm->idx--;
}

void virtual_console_line_mode_new_line(device_t* this) {
  virtual_console_new_line(this);
  // add newline to the buffer
  add_to_buffer(this, '\n');
}

void virtual_console_line_mode_cls(device_t* this) {
  uprintf("LINE MODE CLS\n");
}

void virtual_console_line_mode_move_cursor(device_t *this, int rrow, int rcol) {
  uprintf("LINE MODE CURSOR %d %d\n", rrow, rcol);
  
  virtual_console_line_mode_t *lm = &private(this)->line_mode;
  //  beep();
  return;
 
  if(rcol == -1) {
    lm->idx -= 3; // cursor movement
    if(lm->idx > 0)
      lm->idx--;
    virtual_console_move_cursor(this, 0, -1);
  }
}

void virtual_console_line_mode_backspace(device_t *this) {
  if(private(this)->line_mode.idx) {
    backspace_buffer(this);
    virtual_console_move_cursor(this, 0, -1);
    virtual_console_draw_char(this, ' ', 
			      private(this)->cur_row,
			      private(this)->cur_col);
  } else {
    // beep();
  }
}

void virtual_console_line_mode_draw_char(device_t* this, char ch, 
					 int row, int col) {
  //  uprintf("drawing char (idx = %d)\n",  private(this)->line_mode.idx);
  add_to_buffer(this, ch);

  if(private(this)->termios.c_lflag & ECHO)    
    virtual_console_draw_char(this, ch, row, col);
}
