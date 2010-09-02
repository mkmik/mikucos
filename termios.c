#include <termios.h>

void termios_init_termios(termios_t *t) {
  t->c_iflag = ICRNL | IXON | IMAXBEL;
  t->c_oflag = OPOST;
  t->c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE;

  int i;
  for(i=0; i<NCCS; i++)
    t->c_cc[i] = -1;

  t->c_cc[VINTR] = 0x3;  // CTRL-C
  t->c_cc[VERASE] = 0x8; // BS
  t->c_cc[VEOF] = 0x4; // CTRL-D
  t->c_cc[VSTART] = 0x11; // CTRL-Q
  t->c_cc[VSTOP] = 0x13; // CTRL-S
  
}
