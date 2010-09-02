#include <version.h>
#include <multiboot.h>
#include <dog.h>
#include <libc.h>
#include <io.h>
#include <init.h>
#include <hal.h>
#include <pic.h>
#include <timer.h>
#include <keyboard.h>
#include <speaker.h>
#include <thread_test.h>
#include <cmos.h>
#include <application.h>

int main(int argc, char** argv) {
  cls();
  
  printf("KaOS " KERNEL_RELEASE " started from %s\n", 
	 multiboot_loader_name());
  printf("cmdline: %s\n", argv[0]);
  multiboot_print();
  setAttr(0xF);    
  keyboard_set_led(KEYLED_CAPS_LOCK | KEYLED_SCROLL_LOCK);
  
  printf("CMOS time %d:%d:%d %d.%d.%d\n",
	 cmos_hours(), cmos_minutes(), cmos_seconds(),
	 cmos_day_of_month(), cmos_month(), cmos_year());
  
  applications_init();
  applications_run();
  
  return 0;
}
