##
## make both the widget
## and the widget-control that matches
## the features in the widget
##
## assumes that you've set the AVR32BIN environment
## to point to the directory containing avr32-gcc
HOST_CC ?= gcc
PKG_CONFIG ?= pkg-config
LIBUSB_CFLAGS ?= $(shell $(PKG_CONFIG) --variable=includedir libusb-1.0 2>/dev/null | sed 's|^|-I|')
LIBUSB_LIBS ?= $(shell $(PKG_CONFIG) --libs libusb-1.0 2>/dev/null || echo -lusb-1.0)
HOST_CFLAGS ?=
HOST_LDFLAGS ?=

all:: Release/widget.elf widget-control

Release/widget.elf::
	./make-widget

audio-widget::
	rm -f Release/widget.elf Release/src/features.o
	CFLAGS=-DFEATURE_DEFAULT_BOARD=feature_board_dib ./make-widget

sdr-widget::
	rm -f Release/widget.elf Release/src/features.o
	CFLAGS=-DFEATURE_DEFAULT_BOARD=feature_board_widget ./make-widget

widget-control: widget-control.c src/features.h
	$(HOST_CC) $(HOST_CFLAGS) $(LIBUSB_CFLAGS) -o widget-control widget-control.c $(HOST_LDFLAGS) $(LIBUSB_LIBS)

clean::
	cd Release && make clean
	rm -f widget-control
