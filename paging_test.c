#include <paging_test.h>

#include <libc.h>
#include <uart.h>
#include <paging.h>
#include <timer.h>
#include <init.h>

#define STACK_SIZE 1024

static struct tss_struct default_tss;
static char kernel_stack[STACK_SIZE+1];

static char *test_string = "this is a string";

CREATE_EXCEPTION_DISPATCHER(page_fault_test);
CREATE_EXCEPTION_DISPATCHER(gpf_test);
CREATE_EXCEPTION_DISPATCHER(double_fault_test);
CREATE_ISR_DISPATCHER(syscall_test);

void simple_usermode_image();

void paging_test_init() {
}

void paging_test_run() {
  char *paddr,*vaddr;
  u_int32_t userpage;
  void (*user_mode_entry)();
  pte_t * user_pte;

  printf("Paging test\n");
  uprintf("Paging test\n");

  printf("\naliasing test\n");

  paddr = test_string;
  vaddr = (char*)((u_int32_t)paddr | (1000 << 22));
  printf("orig addr %p, alias addr %p\n", paddr, vaddr);
  page_directory[page_dir_index(vaddr)] = page_directory[0];

  printf("original: %s\n", paddr);
  printf("alias: %s\n", vaddr);

  set_isr(8, double_fault_test_isr);
  set_isr(14, page_fault_test_isr);
  set_isr(13, gpf_test_isr);

  // mark 0 page not present.
  set_pte_flags(&identity_page_table[0], 0);
  printf("\ndereferencing NULL pointer\n");
  printf("value at 0: %x\n", *(int*)0);

  // user mode setup

  printf("\nsetting up user mode environment\n");
  userpage = 0x200000;
  //  user_mode_entry = userpage;
  user_mode_entry = simple_usermode_image;
  memcpy((void*)userpage, simple_usermode_image, 0x100);

  user_pte = &identity_page_table[page_table_index(userpage)];
  set_pte_flags(user_pte, get_pte_flags(user_pte) | PTE_USER | PTE_PRESENT);
  
  set_isr(0x80, syscall_test_isr);
  //  idt_table[0x80].flags |=  3<<13; // TODO: move to mal

  printf("calling user mode entry\n");
  uprintf("calling user mode entry\n");

  default_tss.esp0 = (u_int32_t)(kernel_stack+STACK_SIZE);
  default_tss.ss0 = __KERNEL_DS;
  
  //  set_tss_desc(&gdt_table[8], &default_tss); // TODO: move to mal
  asm("ltr %%ax" : : "a"(0x8*8));

  delay(10);
  //  for(i=0;i<15;i++)
  //  disable_irq(0);

  //  set_isr(0x20, syscall_test_isr);

#if 0
  wrmsr(MSR_IA32_SYSENTER_CS, __KERNEL_CS, 0);
  wrmsr(MSR_IA32_SYSENTER_ESP, 0x000000, 0);
  wrmsr(MSR_IA32_SYSENTER_EIP, 0x000000, 0);

  asm("sysexit" : : "d"(0x200000), "c"(0x200000)); 
#endif

#if 1
  asm("mov   $0x2b, %eax\n"
      "pushl %eax\n"      // user stack segmnet
      "pushl $0x200100\n" // user stack base
      "pushl $0x0200\n"   // user flags // enable interrupts
      "pushl $0x23\n"     // user code segment
      "pushl $0x200000\n" // user eip
      "mov   %ax, %ds\n"  // user data segment
      "iret\n"); 
#endif
}

void page_fault_test_cisr() {
  u_int32_t addr;
  u_int16_t error;
  u_int32_t eip;  
  u_int16_t cs;
  asm("mov 40(%%ebp), %0" : "=r"(error));
  asm("mov 44(%%ebp), %0" : "=r"(eip));
  asm("mov 46(%%ebp), %0" : "=r"(cs));
  asm("mov %%cr2, %0" : "=r"(addr));

  printf("PAGE FAULT at %p (%x, %x)                \n", 
	 addr, page_dir_index(addr), page_table_index(addr));
  printf(" eip %p, cs %p error %x                  \n", eip, cs, error);
  printf(" fault in %s mode, %s.                \n"
	 "                                         \n"
	 "                                         \n",
	 (error & 0x4 ? "user" : "kernel"), (error & 0x1 ? "protection" : "non present"));

  set_pte_flags(&identity_page_table[0], PTE_RW | PTE_PRESENT);
  flush_tlb_one(0);
}

void gpf_test_cisr() {
  u_int16_t error;
  u_int32_t eip;  
  u_int16_t cs;
  asm("mov 40(%%ebp), %0" : "=r"(error));
  asm("mov 44(%%ebp), %0" : "=r"(eip));
  asm("mov 46(%%ebp), %0" : "=r"(cs)); 

  printf("general protection fault (bad sel %p, eip %p, cs %p)\n", error, eip, cs);
  uprintf("general protection fault (bad sel %p, eip %p, cs %p)\n", error, eip, cs);
  asm("hlt");
}

void double_fault_test_cisr() {
  printf("DOUBLE FAULT      \n");
  asm("hlt");
}

void syscall_test_cisr() {
  u_int32_t eflags;
  asm("pushf ;popl %0" : "=g"(eflags));
  printf("system call called, flags %x\n", eflags);

  hal_enable_interrupts();
  delay(500);
  hal_disable_interrupts();
}


/* 
   for(i=0; i<4; i++)
     syscall();
   while(1);
*/
asm(".text\n"
    "simple_usermode_image:\n"
    "mov $0x3, %eax\n"
    "volp:\n"
    "int $0x80\n"
    "dec %eax\n"
    "jnz volp\n"
    "int $0x80\n"
    "back: inc %eax\n"
    "jmp back");
