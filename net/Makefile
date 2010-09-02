FILES=$(patsubst %.c,%.o,$(wildcard *.c))

GCC_INCLUDES=-I$(shell gcc --print-file-name=include)

-include ../Makefile.defs

DIET=diet $(CC)
INCLUDES=-I.. 
ifneq ($(DEBUG),yes)
 CFLAGS=-O2 -Wall -ffast-math $(DEFS) 
else
 CFLAGS=-g3 -Wall -ffast-math $(DEFS)
endif
CXXFLAGS=$(CFLAGS)
EMBEDDED_CFLAGS=$(CFLAGS) -nostdinc $(INCLUDES) -DUSER_PROGRAM_LIBRARY $(GCC_INCLUDES)

all: libnet.a

libnet.a: $(FILES)
	ar cru $@ $(FILES)
	ranlib $@

clean:
	rm *.o *.a -f

.deps:	
	gcc -MM $(INCLUDES) *.c >.deps

ifneq ($(SHOW_CMDLINE),yes)

%.o : %.c
	@echo Compiling $<
	@$(CC) -c $(EMBEDDED_CFLAGS)  $<

else

%.o : %.c
	$(CC) -c $(EMBEDDED_CFLAGS) $<

endif


-include .deps