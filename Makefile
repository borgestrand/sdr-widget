##
## make both the widget
## and the widget-control that matches
## the features in the widget
##
## assumes that you've set the AVR32BIN environment
## to point to the directory containing avr32-gcc

# Pre 20120410
all:: Release/widget.elf widget-control

# Pre 20120410
Release/widget.elf::
	./make-widget

# Updated 20120410
audio-widget::
	rm -f Release/widget.elf Release/src/features.o
	./make-widget-bsb audio-widget

# Updated 20120410
sdr-widget::
	rm -f Release/widget.elf Release/src/features.o
	./make-widget-bsb sdr-widget

# Pre 20120410
widget-control: widget-control.c src/features.h
	gcc -o widget-control widget-control.c -lusb-1.0

# Pre 20120410
clean::
	cd Release && make clean
	rm -f widget-control
