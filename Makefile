PROJECT=test

CC=avr-gcc
OC=avr-objcopy
AD=avrdude

CFLAGS=-g -mmcu=atmega168 -DF_CPU=20000000UL -Os -Wall -Wstrict-prototypes -mcall-prologues -DHD44780_BIT=4
LDFLAGS=-g -mmcu=atmega168 -lprintf_flt -lm -uvfprintf

ADFLAGS=-c stk500v2 -p m168
OCFLAGS=-j .text -j .data -O ihex 

all:		$(PROJECT).hex

.PHONY:		all help clean install

all:		$(PROJECT).hex

rdfuses:
		@echo -n "High Fuse Byte: "; $(AD) $(ADFLAGS) -F -q -q -U hfuse:r:-:h
		@echo -n "Low  Fuse Byte: "; $(AD) $(ADFLAGS) -F -q -q -U lfuse:r:-:h


clean:
		rm -f *.o *.map *.elf *.hex *.out

install:	$(PROJECT).hex
		$(AD) $(ADFLAGS) -e -U flash:w:$^

%.o:		%.c 
		$(CC) $(CFLAGS) -c $^

test.elf:	test.o lcd.o
		$(CC) $(LDFLAGS) -o $@ $^ -lm -lprintf_flt
		#$(CC) $(LDFLAGS) -o $@ $^ "$(TOOLPATHWIN)\\avr\\lib\\libm.a" "$(TOOLPATHWIN)\\avr\\lib\\libprintf_flt.a"

%.hex:		%.elf 
	    	$(OC) $(OCFLAGS) $^ $@

# fuse byte settings:
#  Atmel AVR ATmega8 
#  Fuse Low Byte      = 0xe1 (1MHz internal), 0xe3 (4MHz internal), 0xe4 (8MHz internal)
#  Fuse High Byte     = 0xd9 
#  Factory default is 0xe1 for low byte and 0xd9 for high byte
#  hi=df lo=b7 -> external 
# Check this with make rdfuses
