compiler= gcc
cflags= -O2 -pipe -Wall
includes=  `pkg-config libusb-1.0  --cflags` `sdl-config --cflags`
libdir= `pkg-config libusb-1.0 --libs` `sdl-config --libs`
link= -lm -lGL -lGLU
target:= vcaptfx2

source_files:=$(wildcard *.c) 
object_files:=$(patsubst %.c,%.o,$(source_files))


$(target): $(object_files)
	$(compiler) $(cflags) $^ $(libdir) $(link) -o $@

%.o: %.c
	$(compiler) $(cflags) $(includes)  -c $<
	
clean: 
	rm -f $(target)
	rm -f $(object_files)

hex:
	@echo "#include <stdio.h>  /* For NULL... */" > vcapt_hex.c
	@echo >> vcapt_hex.c
	@echo "const char *vcapt_firmware[]={" >> vcapt_hex.c
	@bash -c 'while read line ; do echo -e "\t\"$$line\"," >> vcapt_hex.c ; done < ../sigrok-firmware-fx2lafw/hw/cypress-fx2/fx2lafw-cypress-fx2.ihx'
#	@echo -e "\tNULL" >> vcapt_hex.c
	@echo  "\tNULL" >> vcapt_hex.c
	@echo "};" >> vcapt_hex.c
	@echo "Created vcapt_hex.c"



