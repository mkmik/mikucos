#include <config.h>
#include <keyboard.h>
#include <keycodes.h>
#include <scancodes.h>
#include <keymap.h>
#include <keyboard_api.h>
#include <malloc.h>
#include <i8255.h>
#include <hal.h>
#include <mutex.h>
#include <condvar.h>
#include <libc.h>
#include <queue.h>

#include <speaker.h>
#include <io.h>


condvar_t keyboard_queue_condvar;
mutex_t keyboard_wait_lock;

driver_t keyboard_driver;
device_t keyboard_device;
device_ops_t keyboard_ops = {
  .init = keyboard_device_init,
  .read = keyboard_read,
  .set_device_option = keyboard_set_device_option
};

struct keyboard_private {
  int mode;
};
typedef struct keyboard_private keyboard_private_t;

#define private(this) ((keyboard_private_t*)((this)->private))

/** convert a scancode to a keycode 
 * if the scancode is an escape returns -1
 * and remember the escape */
static int scancode_to_keycode(int scancode);
/** convert the keycode to an ascii value with ansi
 * escape sequences for extended characters (cursors etc) 
 * and enqueues the resulting characters */
static void keycode_to_ansi(int keycode);

/** mantains modifiers golobal state 
 * in sync with key press/release events */
static void sync_modifiers(int keycode);

/** sounds a note depending on the key */
static void key_sound(int scancode);

void keyboard_device_init(device_t *this) {
  this->private = malloc(sizeof(keyboard_private_t));
  private(this)->mode = KEYBOARD_MODE_ANSI;
}

int keyboard_set_device_option(device_t *this, const char* name, va_list args) {
  if(strcmp(name, "translation") == 0) { 
    int mode = va_arg(args, int);
    uprintf("setting keyboard device option '%s' to %d \n", name, mode);
    private(this)->mode = mode;
    return 0;
  }
  return -1;
}

void keyboard_init() {
  i8255_init();

  condvar_init_condvar(&keyboard_queue_condvar);
  mutex_init_mutex(&keyboard_wait_lock);

  driver_init_driver(&keyboard_driver);
  keyboard_driver.name = "Keyboard";
  driver_register(&keyboard_driver, "");

  device_init_device(&keyboard_device);
  keyboard_device.name = "0";
  keyboard_device.ops = &keyboard_ops;

  device_attach_driver(&keyboard_device, &keyboard_driver);

  enable_irq(1);
}

void keyboard_set_led(int mask) {  
  i8255_set_led(mask);
}

#if KEYSOUND
static char ch_pressed[128];
#endif
static int modifiers = 0;

char_queue_t keyboard_queue;

void keyboard_enqueue(char ch) {
  mutex_lock(&keyboard_wait_lock);

  char_enqueue_nolock(&keyboard_queue, ch);  

  condvar_broadcast(&keyboard_queue_condvar);
  mutex_unlock(&keyboard_wait_lock);
}

char keyboard_dequeue() {
  return char_dequeue_nolock(&keyboard_queue);
}

void keyboard_isr(int scancode) {
  // restores interrupts. keyboard int is not yet acked 
  // and it will be after processing this one
  hal_irql_set(IRQL_KERNEL); 

  if(scancode == 0xfa)
    return; // ignore commands

  int keycode = scancode_to_keycode(scancode);

  // handle modifiers
  sync_modifiers(keycode);

  if(private(&keyboard_device)->mode == KEYBOARD_MODE_SCANCODE)
    keyboard_enqueue(scancode);
  else {
    if(keycode > 0) {
      if(private(&keyboard_device)->mode == KEYBOARD_MODE_KEYCODE) 
	keyboard_enqueue(keycode);
      else if(private(&keyboard_device)->mode == KEYBOARD_MODE_ANSI) 
	keycode_to_ansi(keycode);
    }    
  }

  key_sound(scancode);
}

ssize_t keyboard_read(struct device *this, void* buffer, 
		      size_t len, off_t *off) {
  int size = 0;
  while(len) {
    mutex_lock(&keyboard_wait_lock);
    while(char_queue_empty_nolock(&keyboard_queue))
      condvar_wait(&keyboard_queue_condvar, &keyboard_wait_lock);
    
    char ch;
    while(len-- && (ch = keyboard_dequeue()))
      ((char*)buffer)[size++] = ch;
    len++;

    mutex_unlock(&keyboard_wait_lock); 
  }

  return size;
}


// partially stolen from linux
int scancode_to_keycode(int scancode) {
  int keycode;
  int released = 0;
  static int prev_scancode;
  
  /* special prefix scancodes.. */
  if (scancode == 0xe0 || scancode == 0xe1) {
    prev_scancode = scancode;
    return 0;
  }

  released = scancode & 0x80;
  scancode &= 0x7f;
  
  
  if (prev_scancode) {
    /*
     * usually it will be 0xe0, but a Pause key generates
     * e1 1d 45 e1 9d c5 when pressed, and nothing when released
     */
    if (prev_scancode != 0xe0) {
      if (prev_scancode == 0xe1 && scancode == 0x1d) {
	prev_scancode = 0x100;
	return 0;
      } else if (prev_scancode == 0x100 && scancode == 0x45) {
	keycode = E1_PAUSE;
	prev_scancode = 0;
      } else {
	uprintf("keyboard: unknown e1 escape sequence\n");
	prev_scancode = 0;
	return -1;
      }
    } else {
      prev_scancode = 0;
      /*
       *  The keyboard maintains its own internal caps lock and
       *  num lock statuses. In caps lock mode E0 AA precedes make
       *  code and E0 2A follows break code. In num lock mode,
       *  E0 2A precedes make code and E0 AA follows break code.
       *  We do our own book-keeping, so we will just ignore these.
       */
      /* 
       *  For my keyboard there is no caps lock mode, but there are
       *  both Shift-L and Shift-R modes. The former mode generates
       *  E0 2A / E0 AA pairs, the latter E0 B6 / E0 36 pairs.
       *  So, we should also ignore the latter. - aeb@cwi.nl
       */
      if(scancode == 0x2a || scancode == 0x36)
	return -1;
      
      if(e0_keys[scancode])
	keycode = e0_keys[scancode];
      else {
	uprintf("keyboard: unknown scancode e0 %02x\n",
		scancode);
	return -1;
      } 
    }     
  } else 
    keycode = scancode;
  return keycode | released;
}

static void do_letter(keysym_t sym) {
  keyboard_enqueue(KVAL(sym));
}

static void do_latin(keysym_t sym) {
  keyboard_enqueue(KVAL(sym));
}

static void do_special(keysym_t sym) {
  if(sym == K_ENTER)
    keyboard_enqueue('\r');
  else if(sym == K_INCRCONSOLE)
    uprintf("increment console\n");
  else if(sym == K_DECRCONSOLE)
    uprintf("decrement console\n");
  else if(sym == K_BOOT)
    hal_reboot();
  else
    uprintf("unknown special %d\n", KVAL(sym));
}

static void do_console(keysym_t sym) {
  uprintf("will switch to vc %d\n", KVAL(sym));
}

static void do_cursor(keysym_t sym) {
  static const char *cursors = "BDCA";
  keyboard_enqueue('\e');
  keyboard_enqueue('[');
  keyboard_enqueue(cursors[KVAL(sym)]);
}

void keycode_to_ansi(int keycode) {
  if(keycode > NR_KEYS)
    return;

  int kid = 0;
  if(modifiers & MODIFIER_SHIFT)
    kid += 1;
  if(modifiers & MODIFIER_ALTGR)
    kid += 2;
  if(modifiers & MODIFIER_CTRL)
    kid += 1*4;
  if(modifiers & MODIFIER_ALT)
    kid += 2*4;

  keysym_t *km = key_maps[kid];

  keysym_t sym = km[keycode];
  sym &= 0xfff; // don't know why, but linux keymap have most significant byte set

  switch(KTYP(sym)) {
  case KT_LATIN:
    do_latin(sym);
    break;
  case KT_LETTER:
    do_letter(sym);
    break;
  case KT_ASCII:
    uprintf("got ascii %x\n", KVAL(sym));
    break;
  case KT_CUR:
    do_cursor(sym);
    break;
  case KT_LOCK:
    uprintf("got lock\n");
    break;
  case KT_SPEC:
    do_special(sym);
    break;
  case KT_CONS:
    do_console(sym);
    break;
  case KT_SHIFT:
    break;
  default:
    uprintf("got unknown type %x\n", KTYP(sym));
  }
}

void key_sound(int scancode) {
#if KEYSOUND
  // sound (optional)
  if(scancode & (1<<7)) {
    ch_pressed[scancode & ~(1<<7)] = 0;
    speaker_off();
  } else {
    if(ch_pressed[scancode]) {
      return; // ignore repeat
    }
    ch_pressed[scancode] = 1;
    speaker_set_freq(10*scancode);
    speaker_on();
  }
#endif
}

void sync_modifiers(int keycode) {
  keysym_t sym = plain_map[keycode & 0x7f] & 0xfff;
  int mod = 0;

  if(KTYP(sym) == KT_SHIFT) {
    switch(sym) {
    case K_SHIFT:
      mod = MODIFIER_SHIFT;
      break;
    case K_CTRL:
      mod = MODIFIER_CTRL;
      break;
    case K_ALT:
      mod = MODIFIER_ALT;
      break;
    case K_ALTGR:
      mod = MODIFIER_ALTGR;
      break;
    default:
      uprintf("unknown shifting key\n");
    }
    if(keycode & 0x80)
      modifiers &= ~mod;
    else
      modifiers |= mod;
  }

}
