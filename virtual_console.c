#include <virtual_console.h>
#include <virtual_console_ansi.h>
#include <virtual_console_private.h>
#include <virtual_console_line_mode.h>

#include <libc.h>
#include <malloc.h>
#include <misc.h>

#include <spin_lock.h>
#include <keyboard.h>

#include <video.h>

driver_t virtual_console_driver;
device_t virtual_console_device;
device_ops_t virtual_console_ops = {
  .init = virtual_console_device_init,
  .read = virtual_console_read,
  .write = virtual_console_write,
  .set_device_option = virtual_console_set_device_option,
  .get_device_option = virtual_console_get_device_option
};

ansi_sm_ops_t virtual_console_ansi_sm_ops = {
  .reset = virtual_console_reset,
  .move_cursor = virtual_console_move_cursor,
  .cls = virtual_console_cls,
  .new_line = virtual_console_new_line,
  .draw_char = virtual_console_draw_char,
  .sync_cursor = virtual_console_sync_cursor,
  .backspace = virtual_console_backspace
};

void virtual_console_device_init(device_t* this) {
  this->private = malloc(sizeof(vc_private_t));
  
  private(this)->cur_row = 0;
  private(this)->cur_col = 0;

  private(this)->win_start_row = VC_HEADER;
  private(this)->win_height = video_get_rows() - VC_HEADER;
  private(this)->win_start_col = 0;
  private(this)->win_width = video_get_columns();
  private(this)->ansi_sm.state = ANSI_STATE_NORMAL;
  private(this)->ansi_sm.ops = &virtual_console_ansi_sm_ops;
  private(this)->line_mode_sm.ops = &virtual_console_ansi_sm_line_mode_ops;
  private(this)->attribute = 0x7;

  termios_init_termios(&private(this)->termios);

  fnode_t *keyboard_fnode = file_resolve_path("/Devices/Keyboard/0");
  assert(keyboard_fnode);
  private(this)->keyboard = file_open_fnode_file(keyboard_fnode, 0);
  assert(private(this)->keyboard);
}

int virtual_console_close(device_t* this, int flags) {
  return 0;
}

ssize_t virtual_console_read(struct device *this, void* buffer, 
			     size_t len, off_t *off) {
  if(private(this)->termios.c_lflag & ICANON) {
    int res;

    private(this)->line_mode_sm.state = ANSI_STATE_NORMAL;
    private(this)->line_mode_sm.ops = &virtual_console_ansi_sm_line_mode_ops;
    private(this)->line_mode.idx = 0;

    while((private(this)->line_mode.idx < sizeof(private(this)->line_mode.line_buffer)) 
	  && (private(this)->line_mode.idx < len)) {

      u_int8_t ch;
      res = file_read(private(this)->keyboard, &ch, 1);
      assert(res == 1);
      
      virtual_console_putchar_sm(this, ch,
				 &private(this)->line_mode_sm);
      if(ch == '\r')
	break;
    }
    
    res = MIN(len, private(this)->line_mode.idx);
    memcpy(buffer, private(this)->line_mode.line_buffer, res);
    
    *off += res;
    return res;
  } else {
    int res = file_read(private(this)->keyboard, buffer, 1);
    int _off;
    if(private(this)->termios.c_lflag & ECHO)    
      virtual_console_write(this, buffer, res, &_off);
    
    *off += res;
    return res;
  }
}

ssize_t virtual_console_write(struct device *this, const void* buffer, 
			      size_t len, off_t *off) {
  int i;
  for(i=0; i<len; i++)
    virtual_console_putchar(this, ((const char*)buffer)[i]);

  return len;
}

int virtual_console_set_device_option(device_t *this, 
				      const char* name, va_list args) {
  if(strcmp(name, "termios") == 0) { 
    termios_t *termios = va_arg(args, termios_t*);
    assert(termios);
    uprintf("setting termios\n");

    memcpy(&private(this)->termios, termios, sizeof(termios_t));
    return 0;
  }
  return -1;
}

int virtual_console_get_device_option(device_t *this, 
				      const char* name, va_list args) {
  if(strcmp(name, "termios") == 0) { 
    termios_t *termios = va_arg(args, termios_t*);
    assert(termios);
    uprintf("getting termios\n");

    memcpy(termios, &private(this)->termios, sizeof(termios_t));
    return 0;
  }
  return -1;
}

// end device ops

void virtual_console_set_position(device_t* this, int row, int col) {
  private(this)->cur_row = row;
  private(this)->cur_col = col;
  virtual_console_sync_cursor(this);
}

void virtual_console_sync_cursor(device_t* this) {
  video_set_cursor_pos(private(this)->cur_row + private(this)->win_start_row,
		       private(this)->cur_col + private(this)->win_start_col); 
}

void virtual_console_cls(device_t* this) {
  video_scroll(private(this)->win_start_row, 
	       private(this)->win_start_col, 
	       private(this)->win_width, 
	       private(this)->win_height,
	       -private(this)->win_height);
  virtual_console_set_position(this, 0, 0);
}

void virtual_console_reset(device_t *this) {
  ansi_state_machine_t *sm = &private(this)->ansi_sm;
  sm->state = ANSI_STATE_NORMAL;
  private(this)->attribute = 0x7;

  virtual_console_cls(this);
}

void virtual_console_backspace(device_t *this) {
  uprintf("BACKSPACE\n");
}

void virtual_console_draw_char(device_t* this, char ch, int row, int col) {
  assert(row < private(this)->win_height);

  setAttr(private(this)->attribute);
  video_draw_char(ch, 
		  private(this)->win_start_row + row, 
		  private(this)->win_start_col + col);
}

void virtual_console_new_line(device_t* this) {
  private(this)->cur_col = 0;
  if(private(this)->cur_row >= (private(this)->win_height - 1))
    video_scroll(private(this)->win_start_row, 
		 private(this)->win_start_col, 
		 private(this)->win_width, 
		 private(this)->win_height,
		 -1);
  else
    private(this)->cur_row++;

  virtual_console_sync_cursor(this);
}

void virtual_console_move_cursor(device_t *this, int rrow, int rcol) {
  int nrow = MIN(private(this)->cur_row+rrow, private(this)->win_height-1);
  int ncol = MIN(private(this)->cur_col+rcol, private(this)->win_width-1);
  virtual_console_set_position(this, MAX(nrow, 0), MAX(ncol, 0));
}

void virtual_console_init() {
  // driver
  driver_init_driver(&virtual_console_driver);
  virtual_console_driver.name = "VirtualConsole";
  driver_register(&virtual_console_driver, "");

  // device 0
  device_init_device(&virtual_console_device);
  virtual_console_device.name = "0";
  virtual_console_device.ops = &virtual_console_ops;

  device_attach_driver(&virtual_console_device, &virtual_console_driver);
}
