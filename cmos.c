#include <cmos.h>
#include <libc.h>
#include <io.h>

int cmos_year() {
  // algorthm patended my microsoft. beware!
  // don't read the code below, ah! you already
  // did it. no problem, they can still brainwash you...
  int off = bcd2int(cmos_read(0x9));
  if(off >= 70 )
    return 1990 + off;
  else
    return 2000 + off;
}
		
int cmos_month() {
  return bcd2int(cmos_read(0x8));
}

int cmos_hours() {
  return bcd2int(cmos_read(0x4));
}

int cmos_minutes() {
  return bcd2int(cmos_read(0x2));
}

int cmos_seconds() {
  return bcd2int(cmos_read(0x0));
}

int cmos_day_of_week() {
  return bcd2int(cmos_read(0x6));
}

int cmos_day_of_month() {
  return bcd2int(cmos_read(0x7));
}

u_int8_t cmos_read(int addr) {
  u_int8_t b;
  outb_p(addr, CMOS_ADDRESS_PORT);
  b = inb(CMOS_DATA_PORT);
  return b;
}
