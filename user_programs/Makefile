FILES=$(patsubst %.c,%.o,$(wildcard *.c))
PROGRAMS=$(patsubst %.c,%,$(shell for i in *.c; do grep -q "ONLY_INTERNAL" $$i || echo $$i;done))

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

all: libprograms.a internals.h $(PROGRAMS)

libprograms.a: $(FILES)
	ar cru $@ $(FILES)
	ranlib $@

$(PROGRAMS): 
	echo $(PROGRAMS)
	$(MAKE) $(addsuffix _diet.o,$@)
	@echo linking $@
	$(DIET) -o $@ $(addsuffix _diet.o,$@)

internals.h: $(FILES)
	@echo -e "#ifndef _USER_INTERNALS_H_" >$@
	@echo -e "#define _USER_INTERNALS_H_\n" >>$@
	@for i in *.c ; do grep -q "MAIN" $$i && echo "int internal_$${i%%.c} (int argc, char** argv);" >>$@; done
	@echo -e "\n#endif" >>$@

clean:
	rm *.o *.a internals.h $(PROGRAMS) -f

.deps:	
	gcc -MM $(INCLUDES) *.c >.deps

ifneq ($(SHOW_CMDLINE),yes)

%_diet.o : %.c
	@echo diet compiling $<
	$(DIET) -o $@ -c $(CFLAGS) $<

%.o : %.c
	@echo Compiling $<
	@$(CC) -c $(EMBEDDED_CFLAGS)  $<

else

%_diet.o : %.c
	$(DIET) -o $@ -c $(CFLAGS) $<

%.o : %.c
	$(CC) -c $(EMBEDDED_CFLAGS) $<

endif


-include .deps