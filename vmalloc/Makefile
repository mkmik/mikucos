#
-include Makefile.defs

VMALLOC_FILES=malloc.o   vmclose.o   vmdisc.o   vmprivate.o  vmsegment.o  vmtrace.o vmbest.o   vmdcheap.o  vmexit.o  vmopen.o   vmprofile.o  vmset.o      vmwalk.o vmclear.o  vmdebug.o   vmlast.o  vmpool.o   vmregion.o   vmstat.o


INCLUDES=-I../ -I.
CC=gcc
CFLAGS=-O2 -Wall -ffast-math -nostdinc $(INCLUDES) $(DEFS) -I$(shell gcc --print-file-name=include)
CXXFLAGS=$(CFLAGS)

SUDO=sudo

all: vmalloc

vmalloc: .deps $(VMALLOC_FILES) 
	ar cru libvmalloc.a $(VMALLOC_FILES)

.deps:	
	gcc -MM $(INCLUDES) *.c >.deps

clean:
	rm -f .deps libvmalloc.a *.o

distclean: clean
	rm -f *~ */*~ Makefile.defs 

ifneq ($(SHOW_CMDLINE),yes)

%.o : %.cpp
	@echo Compiling $<
	@$(CXX) -c $(CXXFLAGS) $<

%.o : %.c
	@echo Compiling $<
	@$(CC) -c $(CFLAGS) $<

%.o : %.S
	@echo Assembling $<
	@$(CC) -c  $< -D__ASSEMBLY__
else

%.o : %.S
	$(CC) -c  $< -D__ASSEMBLY__

endif




-include .deps
