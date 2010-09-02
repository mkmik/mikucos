VERSION = 0
PATCHLEVEL = 2
SUBLEVEL = 2
EXTRAVERSION = 

SERVER=eolo

#
-include Makefile.defs

KERNEL_RELEASE=$(VERSION).$(PATCHLEVEL).$(SUBLEVEL)$(EXTRAVERSION)

KERNEL_FILES=entry.o multiboot_header.o init.o hal.o x86.o apic.o timer.o \
	     pic.o keyboard.o speaker.o i8259.o i8253.o i8255.o i16550A.o video.o \
	     uart.o psaux.o thread.o dispatcher.o event.o paging.o mm.o file.o file_api.o \
	     main.o dog.o libc.o libc_strtol.o ctype.o cmos.o multiboot.o tar.o \
	     thread_test.o application.o mutex.o condvar.o \
	     bios32.o pci.o pci_names.o pci_test.o lance_driver.o paging_test.o \
	     file_test.o ngulfs_buffering.o ngulfs_low_api.o hd_test.o vfs_test.o ide.o \
	     ngulfs_api.o  malloc.o avl.o process.o rootfs.o devfs.o driver.o device.o \
	     ngulfs_format.o debug.o ngulfs_allocation.o virtual_console.o \
	     virtual_console_ansi.o virtual_console_line_mode.o queue.o gdt.o idt.o \
	     defkeymap.o termios.o shell.o shell_builtins.o harddisk.o process_api.o  \
	     partition.o tarfs.o stdio.o filesystem.o \
	     fat.o fat_low.o

BOOT_FILES=$(shell find boot)

INCLUDES=-I.
LIBS=vmalloc/libvmalloc.a user_programs/libprograms.a
CC=gcc
ifneq ($(DEBUG),yes)
 CFLAGS=-O2 -Wall -ffast-math -nostdinc $(INCLUDES) $(DEFS) \
	-I$(shell gcc --print-file-name=include)
else
 CFLAGS=-g3 -Wall -ffast-math -nostdinc $(INCLUDES) $(DEFS) \
	-I$(shell gcc --print-file-name=include)
endif
CXXFLAGS=$(CFLAGS)

SUDO=sudo

export KERNEL_RELEASE

.PHONY: subdirs vmalloc user_programs

all: subdirs check user_program vmware-ttyS0-server boot/user.com

subdirs: vmalloc user_programs

check:  mkernel
	@mbchk -q mkernel || echo "not multiboot compatible"

mkernel: .deps version.h $(KERNEL_FILES) user_programs/libprograms.a net/libnet.a
#	ld -Ttext 0x100000 -o $@ $(KERNEL_FILES) 
ifneq ($(DEBUG),yes)
	gcc -o $@ $(KERNEL_FILES) $(LIBS) -nostdlib -lgcc -g3 -Wl,-Ttext,0x100000,-S
else
	gcc -o $@ $(KERNEL_FILES) $(LIBS) -nostdlib -lgcc -g3 -Wl,-Ttext,0x100000
endif

vmalloc:
	$(MAKE) -C vmalloc

user_programs:
	$(MAKE) -C user_programs

net:
	$(MAKE) -C net

version.h: Makefile
	@echo "#define KERNEL_RELEASE \""$(KERNEL_RELEASE)"\"" >version.h

user_program: user_program.o
	gcc -o user_program -static -nostdlib user_program.o

vmware-ttyS0-server.o: vmware-ttyS0-server.c
	gcc -c vmware-ttyS0-server.c

vmware-ttyS0-server: vmware-ttyS0-server.o
	gcc -o vmware-ttyS0-server vmware-ttyS0-server.o 

dog.tar.gz:
	tar cvfz dog.tar.gz dog/?

files.tar.gz:  test.txt user_program
	tar cvfz files.tar.gz test.txt user_program

boot.tar.gz: $(BOOT_FILES)
	bash -O extglob -c 'cd boot ; tar cvfz ../boot.tar.gz !(*~|*CVS)'

boot/user.com: 
	@echo "shell ${USER}.com" >boot/user.com

defkeymap.c: defkeymap.map
	set -e ; loadkeys --mktable $< | sed -e 's/^static *//' -e 's:linux/::g' -e 's/kd\.h/keymap\.h/g' -e 's/u_short/u_int16_t/g' -e 's/ushort/u_int16_t/g' > $@

ifeq ($(DEBUG),yes)
# dispatcher need optimization because of register allocation
dispatcher.o: dispatcher.c
	@echo "Compiling (optimized) "$<
	@$(CC) -c $(CFLAGS) -O2 $<
endif

.deps:	version.h
	gcc -MM $(INCLUDES) *.c >.deps

clean:
	make -C vmalloc clean
	make -C user_programs clean
	rm -f .deps version.h mkernel *.o defkeymap.c boot/user.com

distclean: clean
	rm -f *~ */*~ Makefile.defs grub.img.gz

install: all dog.tar.gz files.tar.gz boot.tar.gz
	@cp dog.tar.gz files.tar.gz boot.tar.gz grub
	@cp mkernel /tmp/mkernel
	@gzip /tmp/mkernel
	@mv /tmp/mkernel.gz  grub/mkernel
	@if [ -e /home/tftp ]; then cp grub/mkernel dog.tar.gz files.tar.gz /home/tftp; fi
	@sync

server_install: install
	@scp grub/mkernel $(SERVER):/home/tftp

loop_mount:
	@if [ ! -e ./grub ]; then mkdir grub; fi
	@$(SUDO) mount -o loop,uid=$(USER) ./grub.img grub

loop_unmount:
	@$(SUDO) umount grub
	@if [ -e ./grub ]; then rmdir grub; fi

dist:	distclean
	tar cvfz ../mikucos-$(KERNEL_RELEASE).tar.gz -C .. mikucos --exclude=mikucos/grub.img --exclude=mikucos/grub --exclude=mikucos/boot.tar.gz --exclude=mikucos/dog.tar.gz --exclude=mikucos/grub.img.gz

ifneq ($(SHOW_CMDLINE),yes)

%.o : %.cpp
	@echo Compiling $<
	@$(CXX) -c $(CXXFLAGS) $<

%.o : %.c
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $<

%.o : %.S
	@echo Assembling $<
	@$(CC) -c  $< -D__ASSEMBLY__ $(DEFS)
else

%.o : %.S
	$(CC) -c  $< -D__ASSEMBLY__ $(DEFS)

endif



-include .deps
