#include <virtual_console_ansi.h>
#include <virtual_console.h>
#include <virtual_console_private.h>

#include <debug.h>
#include <misc.h>
#include <libc.h>

static void unknown_escape(device_t* this, ansi_state_machine_t *sm);
static void state_normal(device_t* this, char ch, ansi_state_machine_t *sm);
static void state_escaped(device_t* this, char ch, ansi_state_machine_t *sm);
static void state_escbracket(device_t* this, char ch, ansi_state_machine_t *sm);

void ansi_numeric_prefixes_reset(ansi_state_machine_t *sm) {
  sm->numeric_prefixes_last = 0;
  sm->numeric_prefixes[sm->numeric_prefixes_last] = 0;
}

void ansi_numeric_prefix_set(ansi_state_machine_t *sm, int val) {
  sm->numeric_prefixes[sm->numeric_prefixes_last] = val;
}

int ansi_numeric_prefix_get(ansi_state_machine_t *sm) {
  return sm->numeric_prefixes[sm->numeric_prefixes_last];
}

int ansi_numeric_prefix_push(ansi_state_machine_t *sm) {
  int res = ++sm->numeric_prefixes_last < ANSI_NUMERIC_PREFIX_DEPTH;
  if(!res)
    return res;
  sm->numeric_prefixes[sm->numeric_prefixes_last] = 0; // initial value
  return res;
}

int ansi_numeric_prefix_pop(ansi_state_machine_t *sm) {
  return sm->numeric_prefixes_last-- > 0;
}

void virtual_console_putchar(device_t* this, char ch) {
  virtual_console_putchar_sm(this, ch, &private(this)->ansi_sm);
}

void virtual_console_putchar_sm(device_t* this, char ch, ansi_state_machine_t *sm) {
  switch(sm->state) {
  case ANSI_STATE_NORMAL:
    state_normal(this, ch, sm);
    break;
  case ANSI_STATE_ESCAPED:
    state_escaped(this, ch, sm);
    break;
  case ANSI_STATE_ESCBRACKET:
    state_escbracket(this, ch, sm);
    break;
  default:
    xprintf("state %d\n", sm->state);
    panicf("Unknown ansi sm state");
  }
}

void unknown_escape(device_t* this, ansi_state_machine_t *sm) {
  uprintf("got unknown escape sequence\n");
  sm->state = ANSI_STATE_NORMAL;
}

void state_normal(device_t* this, char ch, ansi_state_machine_t *sm) {
  if(ch == ANSI_ESCAPE)
    sm->state = ANSI_STATE_ESCAPED;
  else if(ch == '\n' || ch == '\r') 
    sm->ops->new_line(this);
  else if(ch == 0x7f)
    sm->ops->backspace(this);
  else {
    sm->ops->draw_char(this, ch, 
			      private(this)->cur_row,
			      private(this)->cur_col++);
    // wrap 
    if(private(this)->cur_col >= private(this)->win_width)
      sm->ops->new_line(this);
  }  
  sm->ops->sync_cursor(this);
}

void state_escaped(device_t* this, char ch, ansi_state_machine_t *sm) {
  switch(ch) {
  case '[':
    sm->state = ANSI_STATE_ESCBRACKET;
    ansi_numeric_prefixes_reset(sm);
    break;
  case 'c':
    sm->ops->reset(this);
  default:
    unknown_escape(this, sm);
  }
}

void state_escbracket(device_t* this, char ch, ansi_state_machine_t *sm) {
  switch(ch) {
  case '0'...'9':
    ansi_numeric_prefix_set(sm, ansi_numeric_prefix_get(sm) * 10 + (ch-'0'));
    // state not changed
    break;
  case ';': // additional parameter
    if(!ansi_numeric_prefix_push(sm)) {
      unknown_escape(this, sm);
      break;
    }
    // state not changed
    break;
  case 'm': // set attributes
    do {
      int attr = ansi_numeric_prefix_get(sm);
      switch(attr) {
      case 0: // clear all attribute
	private(this)->attribute = 0x7; 
	break;	
      case 1: // set bold
	private(this)->attribute = (private(this)->attribute | 0x8);
	break;
      case 30 ... 37: // set foreground color
	attr -= 30;
	attr = (attr & 2) | ((attr & 1) << 2) | (attr >> 2);
	private(this)->attribute = (private(this)->attribute & ~0x7) | attr;
	break;
      case 40 ... 47: // set background color
	attr -= 40;
	attr = (attr & 2) | ((attr & 1) << 2) | (attr >> 2);
	uprintf("setting background color %d\n", attr);
	private(this)->attribute = (private(this)->attribute & ~0x70) | (attr << 4);
	break;
      default:
	uprintf("unknown attribute number %d\n", attr);
      }

    } while(ansi_numeric_prefix_pop(sm));
    
    sm->state = ANSI_STATE_NORMAL;
    break;
  case 'A': // move cursor up 'prefix' lines
    sm->ops->move_cursor(this, -MAX(ansi_numeric_prefix_get(sm), 1), 0);
    sm->state = ANSI_STATE_NORMAL;
    break;
  case 'B': // move cursor down 'prefix' lines
    sm->ops->move_cursor(this, MAX(ansi_numeric_prefix_get(sm), 1), 0);
    sm->state = ANSI_STATE_NORMAL;
    break;
  case 'D': // move cursor left 'prefix' columns
    sm->ops->move_cursor(this, 0, -MAX(ansi_numeric_prefix_get(sm), 1));
    sm->state = ANSI_STATE_NORMAL;
    break;
  case 'C': // move cursor right 'prefix' columns
    sm->ops->move_cursor(this, 0, MAX(ansi_numeric_prefix_get(sm), 1));
    sm->state = ANSI_STATE_NORMAL;
    break;
  case 'J': // erase screen
    switch(ansi_numeric_prefix_get(sm)) {
    case 2: // erase whole screen
      sm->ops->cls(this);
      sm->state = ANSI_STATE_NORMAL;
      break;
    default:
      unknown_escape(this, sm);
    }
    break;
  default:
    unknown_escape(this, sm);
  }

}
