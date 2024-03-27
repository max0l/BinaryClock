TARGET=main
MCU=atmega48a
SOURCES=main.c dcf77.c
HEADERS=main.h dcf77.h
MCU_dude=m48

PROGRAMMER=avrispmkii
# Uncomment for automatic choice
#PORT=-P usb
BAUD=-B125kHz

OBJECTS=$(SOURCES:.c=.o)
CFLAGS=-c -Og 
LDFLAGS=

all: hex eeprom

hex: $(TARGET).hex

eeprom: $(TARGET)_eeprom.hex

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -O ihex -j .data -j .text $< $@

$(TARGET)_eeprom.hex: $(TARGET).elf
	avr-objcopy -O ihex -j .eeprom --change-section-lma .eeprom=0 $< $@

$(TARGET).elf: $(OBJECTS)
	avr-gcc $(LDFLAGS) -mmcu=$(MCU) $^ -o $@

%.o: %.c $(HEADERS)
	avr-gcc $(CFLAGS) -mmcu=$(MCU) $< -o $@

size: 
	avr-size --mcu=$(MCU) -C $(TARGET).elf

program:
	avrdude -p$(MCU_dude) $(BAUD) $(PORT) -c$(PROGRAMMER) -U flash:w:$(TARGET).hex:a

clean_tmp:
	rm -f *.o *.elf

clean:
	rm -f *.o *.elf *.hex

new:
	make clean
	make
	make program