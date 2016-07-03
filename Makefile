
TRGT = avr-
CC   = $(TRGT)gcc
CCPP = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
OD   = $(TRGT)objdump
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary
F_CPU=1000000  
MCU  = atmega328p

CSRC = main.c 
CPPSRC=  
ASRC = 

PROJECT=main

OPT  = -mmcu=$(MCU)   -Os -mtiny-stack -mcall-prologues    -ffunction-sections -fdata-sections -fverbose-asm 
#-funsigned-bitfields -funsigned-char -fshort-enums -fpack-struct  \
COPT = $(OPT)
CPFLAGS += -MD -MP -MF .dep/$(@F).d $(COPT) -DF_CPU=$(F_CPU) -mmcu=$(MCU) -O3
LDFLAGS += -Wl,--cref,--gc-sections -mmcu=$(MCU) 
ASFLAGS += -DF_CPU=$(F_CPU) -mmcu=$(MCU)


COBJS = $(CSRC:.c=.o)
AOBJS = $(ASRC:.S=.o)
CPPOBJS = $(CPPSRC:.cpp=.o)

OBJS = $(COBJS) $(CPPOBJS)  $(AOBJS)

all: $(OBJS) $(PROJECT).elf $(PROJECT).hex $(PROJECT).bin $(PROJECT).lss Makefile


$(COBJS) : %.o : %.c
	@echo
	$(CC)  -g -S $(CPFLAGS) $(CPPOPT) -I . $(INCDIR) $< -o $@.s
	$(CC)  -g  -c  $(CPFLAGS) $(CPPOPT) -I . $(INCDIR) $< -o $@

$(CPPOBJS) : %.o : %.cpp
	@echo
	$(CCPP)  -g -S $(CXXFLAGS) $(CPPOPT) -I . $(INCDIR) $< -o $@.s
	$(CCPP)  -g -c  $(CXXFLAGS) $(CPPOPT) -I . $(INCDIR) $< -o $@


$(AOBJS) : %.o : %.S
	@echo
	$(AS) -c $(ASFLAGS) -I . $(INCDIR) $< -o $@



%elf: $(OBJS) 
	@echo LINK  $@  $(CCPP) $(TRGT)
	$(CCPP)  $(OBJS) $(LDFLAGS) $(LIBS) -o $@
	$(TRGT)size  main.elf 

%hex: %elf
	$(HEX) $< $@

%bin: %elf
	$(BIN) $< $@



DISOPT=-h -S -d
%lss: %elf
	$(OD) $(DISOPT)    $< > $@

clean:
	-rm -f $(OBJS)
	-rm -f $(PROJECT).elf
	-rm -f $(PROJECT).dmp
	-rm -f $(PROJECT).map
	-rm -f $(PROJECT).lss
	-rm -f $(PROJECT).hex
	-rm -f $(PROJECT).bin
	-rm -f $(CSRC:.c=.c.s)
	-rm -f $(CSRC:.c=.o)
	-rm -f $(CSRC:.c=.o.s)
	-rm -f $(CSRC:.c=.c.s)
	-rm -f $(CPPSRC:.cpp=.o)
	-rm -f $(CPPSRC:.cpp=.o.s)
	-rm -f $(ASRC:.S=.o)  

flash: main.hex
	gpio -g mode 22 out
	gpio -g write 22 0
	/usr/local/avrdude/bin/avrdude -p m328 -P /dev/spidev0.0 -c linuxspi -b 100000  -U flash:w:main.hex:i
	gpio -g write 22 1


-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)


