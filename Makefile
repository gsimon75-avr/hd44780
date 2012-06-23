PROJECT=test

TOOLPATH=/cygdrive/c/Program\ Files/WinAvr
TOOLPATHWIN="c:\\Program\ Files\\WinAvr"
CC=$(TOOLPATH)/bin/avr-gcc
OC=$(TOOLPATH)/bin/avr-objcopy
AD=$(TOOLPATH)/bin/avrdude

CFLAGS=-g -mmcu=atmega168 -DF_CPU=18432000UL -Os -Wall -Wstrict-prototypes -mcall-prologues
LDFLAGS=-g -mmcu=atmega168 -lprintf_flt -lm -uvfprintf

ADFLAGS=-c stk500v2 -p m168
OCFLAGS=-j .text -j .data -O ihex 

all:		$(PROJECT).hex

.PHONY:			all help clean install

all:		    $(PROJECT).hex

clean:
			    rm -f *.o *.map *.elf *.hex *.out

install:		$(PROJECT).hex
			    $(AD) $(ADFLAGS) -e -U flash:w:$^

%.o:			%.c 
			    $(CC) $(CFLAGS) -c $^

test.elf:	    test.o lcd.o
			    $(CC) $(LDFLAGS) -o $@ $^ "$(TOOLPATHWIN)\\avr\\lib\\libm.a" "$(TOOLPATHWIN)\\avr\\lib\\libprintf_flt.a"

%.hex:			%.elf 
	    		$(OC) $(OCFLAGS) $^ $@
