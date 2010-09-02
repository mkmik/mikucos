#ifndef _VIRTUAL_CONSOLE_ANSI_H_
#define _VIRTUAL_CONSOLE_ANSI_H_

#include <types.h>
#include <device.h>

#define ANSI_ESCAPE                0x1b
#define ANSI_BS                    0x8
#define ANSI_LF                    0xa
#define ANSI_CR                    0xd

#define ANSI_STATE_NORMAL          0  
#define ANSI_STATE_ESCAPED         1 // got ESC
#define ANSI_STATE_ESCBRACKET      2 // got ESC[ (most commands)

#define ANSI_NUMERIC_PREFIX_DEPTH  4

struct ansi_sm_ops {
  void (*reset)(device_t *this);
  void (*move_cursor)(device_t *this, int rrow, int rcol);
  void (*cls)(device_t *this);
  void (*new_line)(device_t *this);
  void (*draw_char)(device_t* this, char ch, int row, int col);
  void (*sync_cursor)(device_t *this);
  void (*backspace)(device_t *this);
};

typedef struct ansi_sm_ops ansi_sm_ops_t;

struct ansi_state_machine {
  ansi_sm_ops_t *ops;
  int state;
  int numeric_prefixes[ANSI_NUMERIC_PREFIX_DEPTH];
  int numeric_prefixes_last;
};
typedef struct ansi_state_machine ansi_state_machine_t;

// TODO: change this to a FIFO because this way is INCORRECT
// escapes like \e[0;30;44m are incorrectly parsed

extern void ansi_numeric_prefixes_reset(ansi_state_machine_t *sm);
/** set top value */
extern void ansi_numeric_prefix_set(ansi_state_machine_t *sm, int val);
/** get top value */
extern int ansi_numeric_prefix_get(ansi_state_machine_t *sm);
/** return 0 when the stack is full */
extern int ansi_numeric_prefix_push(ansi_state_machine_t *sm);
/** return 0 when there are no more elements */
extern int ansi_numeric_prefix_pop(ansi_state_machine_t *sm);

#endif
