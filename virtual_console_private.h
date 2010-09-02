#ifndef _VIRTUAL_CONSOLE_PRIVATE_H_
#define _VIRTUAL_CONSOLE_PRIVATE_H_

#define VC_HEADER 3

#include <virtual_console_line_mode.h>
#include <virtual_console_ansi.h>
#include <termios.h>

#include <file.h>

struct vc_private {
  int cur_row;
  int cur_col;

  int win_start_row;
  int win_height;
  int win_start_col;
  int win_width;

  ansi_state_machine_t ansi_sm;
  ansi_state_machine_t line_mode_sm;
  virtual_console_line_mode_t line_mode;

  termios_t termios;
  /** current graphics attribute*/
  int attribute;

  file_t* keyboard;
};
typedef struct vc_private vc_private_t;

#define private(this) ((vc_private_t*)((this)->private))

#endif
