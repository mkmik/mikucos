#include "user_programs.h"

#include <stdio.h>
#include <unistd.h>
#include <version.h>

ONLY_INTERNAL;

int MAIN(uname, int argc, char** argv) {
#define UNAME_KERNEL_NAME       (1<<0)
#define UNAME_NODENAME          (1<<1)
#define UNAME_KERNEL_RELEASE    (1<<2)
#define UNAME_KERNEL_VERSION    (1<<3)
#define UNAME_MACHINE           (1<<4)
#define UNAME_PROCESSOR         (1<<5)
#define UNAME_HARDWARE_PLATFORM (1<<6)
#define UNAME_OPERATING_SYSTEM  (1<<7)
#define UNAME_ALL               0xFF

  int features = 0;

  const char* kernel_name = "KaOS ";
  const char* nodename = "lokaost ";
  const char* kernel_release = KERNEL_RELEASE " ";
  const char* kernel_version = "every five minutes, it's CVS man! ";
  const char* machine = "shit86 ";
  const char* processor = "Genuine VMWARE ";
  const char* hardware_platform = "Intel ";
  const char* operating_system = "GRU/KaOS ";

  const char* usage = "Usage: uname [OPTIONS] ... \n"
    "Print certain system information.  With no OPTION, same as -s.\n"
    "\n"
    "  -a, --all                print all information, in the following order:\n"
    "  -s, --kernel-name        print the kernel name\n"
    "  -n, --nodename           print the network node hostname\n"
    "  -r, --kernel-release     print the kernel release\n"
    "  -v, --kernel-version     print the kernel version\n"
    "  -m, --machine            print the machine hardware name\n"
    "  -p, --processor          print the processor type\n"
    "  -i, --hardware-platform  print the hardware platform\n"
    "  -o, --operating-system   print the operating system\n"
    "      --help     display this help and exit\n"
    "      --version  output version information and exit\n"
    "\n"
    "Report bugs to <whoknows@devnull.net>.\n";

  const char* version = "uname (KaOS shell builtins) 0.1\n"
    "Written by Kaos crew\n\nThis is free software\n";

  int i;
  int none = 1;
  for(i=1; i<argc; i++) {
    none = 0;
    if(!(strcmp(argv[i], "--help"))) {
      printf("%s", usage);
      return 0;
    } else if(!(strcmp(argv[i], "--version"))) {
      printf("%s", version);
      return 0;
    } else if(!(strcmp(argv[i], "-a") && strcmp(argv[i], "-all"))) {
      features |= UNAME_ALL;
    } else if(!(strcmp(argv[i], "-s") && strcmp(argv[i], "--kernel-name"))) {
      features |= UNAME_KERNEL_NAME;
    } else if(!(strcmp(argv[i], "-n") && strcmp(argv[i], "--nodename"))) {
      features |= UNAME_NODENAME;
    } else if(!(strcmp(argv[i], "-r") && strcmp(argv[i], "--kernel-release"))) {
      features |= UNAME_KERNEL_RELEASE;
    } else if(!(strcmp(argv[i], "-v") && strcmp(argv[i], "--kernel-version"))) {
      features |= UNAME_KERNEL_VERSION;
    } else if(!(strcmp(argv[i], "-m") && strcmp(argv[i], "--machine"))) {
      features |= UNAME_MACHINE;
    } else if(!(strcmp(argv[i], "-p") && strcmp(argv[i], "--processor"))) {
      features |= UNAME_PROCESSOR;
    } else if(!(strcmp(argv[i], "-i") && strcmp(argv[i], "--hardware-platform"))) {
      features |= UNAME_HARDWARE_PLATFORM;
    } else if(!(strcmp(argv[i], "-o") && strcmp(argv[i], "--operating-system"))) {
      features |= UNAME_OPERATING_SYSTEM;
    } else {
      printf("Uname: invalid option -- %s\n"
	     "Try `uname --help' for more information.\n", argv[i]);
      return 0;
    }
  }

  if(none)
    features = UNAME_KERNEL_NAME;

  if(features & UNAME_KERNEL_NAME)
    printf("%s", kernel_name);
  if(features & UNAME_NODENAME)
    printf("%s", nodename);
  if(features & UNAME_KERNEL_RELEASE)
    printf("%s", kernel_release);
  if(features & UNAME_KERNEL_VERSION)
    printf("%s", kernel_version);
  if(features & UNAME_MACHINE)
    printf("%s", machine);
  if(features & UNAME_PROCESSOR)
    printf("%s", processor);
  if(features & UNAME_HARDWARE_PLATFORM)
    printf("%s", hardware_platform);
  if(features & UNAME_OPERATING_SYSTEM)
    printf("%s", operating_system);
  
  printf("\n");
  return 0;
}
