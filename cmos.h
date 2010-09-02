#ifndef __CMOS_H__
#define __CMOS_H__

#include <types.h>

#define CMOS_ADDRESS_PORT 0x70
#define CMOS_DATA_PORT 0x71

extern int cmos_year();
extern int cmos_month();
extern int cmos_hours();
extern int cmos_minutes();
extern int cmos_seconds();
extern int cmos_day_of_week();
extern int cmos_day_of_month();

extern u_int8_t cmos_read(int addr);

#endif
