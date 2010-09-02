#include <init.h>
#include <video.h>
#include <pic.h>
#include <uart.h>
#include <timer.h>
#include <keyboard.h>
#include <psaux.h>
#include <thread.h>
#include <process.h>
#include <config.h>
#include <libc.h>
#include <paging.h>
#include <driver.h>

// fs
#include <rootfs.h>
#include <devfs.h>
#include <tarfs.h>
#include <fat.h>

// devices
#include <virtual_console.h>
#include <harddisk.h>

/** initial stack. used before the threading environment */
char init_stack[INIT_STACK_SIZE] = {0};

#if !USE_MAL
/** interrupt descriptor table*/
struct interrupt_desc idt_table[256] __attribute__((aligned(8))) 
     = {{0,0,0,0},};
/** global descriptor table */
struct segment_desc gdt_table[256] __attribute__((aligned(8))) 
     = {{0x0},                /* NULL descriptor */
	{0x0},                /* not used */
	{0x00cf9a000000ffff}, /* 0x10 kernel 4GB code at 0x00000000 */
	{0x00cf92000000ffff}, /* 0x18 kernel 4GB data at 0x00000000 */
	{0x00cffa000000ffff}, /* 0x23 user   4GB code at 0x00000000 */
	{0x00cff2000000ffff}, /* 0x2b user   4GB data at 0x00000000 */
	{0x0},                /* not used */
	{0x0},                /* not used */
	{0x0},                /* current-TSS  (new cache line) */
	{0x0},                /* current-LDT (not used) */
     };

#endif
/** forward declaration of main thread function (main.c) */
extern void main(int argc, char** argv);

/** encapsulates main() arguments */
struct main_args {
  int argc;
  char *argv[15];
};

/** thread entry points can receive only one argument.
 * main() has two arguments. This func receives a pointer to a
 * struct main_args structure which contains the argc and argv 
 * arguments and then calls the real main. */
static void main_thread_wrapper(void* arg);


void init() {
  struct main_args args = {0, {0,}};
  char *cmdline;

  video_init();    // simplest 1st. useful for debugging output
#if USE_MAL
  mal_gdtInit();
  mal_idtInit();
#endif  
  init_paging();   // initialize paging with initial phisical mapping (see paging.h)
  malloc_init();   // initialize heap management
  driver_init();   // initialize driver database
  rootfs_init();   // initialize root fs but don't mount it
  devfs_init();    // initialize device fs but don't mount it
  pic_init();      // initialize the programmable interrupt controller
  uart_init();     // initialize the serial port 
  timer_init();    // initialize the timer
  keyboard_init(); // initialize the keyboard controller and handlers
  psaux_init();    // initialize the PS/2 mouse controller
  thread_init();   // initialize threading environment 
  process_init();  // initialize process environment

  virtual_console_init(); // initialize virtual console device
  harddisk_init(); // initializes generic harddisk driver
  //pcnet32_init();   // inizialize pcnet32 ethernet card
  rootfs_mount();  // mount root fs

  // filesystem registration
  tarfs_init();
  fat_init();

  // split the commandline (pseudo, TODO...) 
  cmdline = (char*)multiboot_cmdline();
  while(*cmdline && *cmdline != ' ')
    cmdline++;
  cmdline++;
  args.argv[args.argc++] = cmdline;

  hal_enable_interrupts(); // enable interrupts


#if STARTUP_THREADING 
  // create main thread
  thread_create(main_thread_wrapper, &args, 0);

  // switch the stack and jump to idle thread entry point
  // the idle thread will schedule the next thread 
  // (in this case the main thread)
  asm("mov %0, %%esp\n\t"
      "jmp *%1\n\t"
      :: "m"(idle_tcb->context.esp),
      "m"(idle_tcb->context.eip));
#else // OBSOLETE
  main(args.argc, args.argv);
#endif
}

void main_thread_wrapper(void *_args) {
  struct main_args *args = (struct main_args*)_args;
  main(args->argc, args->argv);
}

