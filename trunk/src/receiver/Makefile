#
# ghpsdr Makefile (Linux)
#
ifeq ($(CC),clang)
else
CC=gcc
endif
ifeq ($(CC),clang)
LINK=clang
else
LINK=gcc
endif
OPTIONS=-g -O3 -msse -msse3 -ffast-math -Wall
INCLUDES=-I. -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include\
         -I/usr/include/atk-1.0 -I/usr/include/cairo\
         -I/usr/include/pango-1.0 -I/usr/include/glib-2.0\
         -I/usr/lib/glib-2.0/include -I/usr/include/pixman-1\
         -I/usr/include/freetype2 -I/usr/include/libpng12 \
         -I/usr/include/gdk-pixbuf-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include \
         -I/usr/include
LIBS=-L. -L../DttSP -lDttSP -lpthread\
     -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lpangoft2-1.0 -lgdk_pixbuf-2.0\
     -lm -lpangocairo-1.0 -lgio-2.0 -lcairo -lpango-1.0 -lfreetype -lz\
     -lfontconfig -lgobject-2.0 -lgmodule-2.0 -lglib-2.0 -lfftw3f -lportaudio

BIN=../../bin

DTTSP=../DttSP/libDttSP.a

PROGRAM=ghpsdr

COMPILE=$(CC) $(INCLUDES)

SOURCES= agc.c audio.c band.c bandscope.c bandscope_control.c bandscope_update.c command.c\
         display.c filter.c frequency.c main.c meter.c\
         meter_update.c mode.c ozy.c\
         preamp.c property.c receiver.c\
         setup.c display_setup.c xvtr_setup.c\
         soundcard.c spectrum_buffers.c spectrum.c spectrum_update.c vfo.c volume.c\
         sinewave.c transmit.c\
         iphone.c audiostream.c\
         subrx.c local_audio.c port_audio.c

OBJS= agc.o audio.o band.o bandscope.o bandscope_control.o bandscope_update.o command.o\
      display.o filter.o frequency.o main.o meter.o\
      meter_update.o mode.o ozy.o\
      preamp.o property.o receiver.o\
      setup.o display_setup.o xvtr_setup.o\
      soundcard.o spectrum_buffers.o spectrum.o spectrum_update.o vfo.o volume.o\
      sinewave.o transmit.o\
      iphone.o audiostream.o\
      subrx.o local_audio.o port_audio.o


all: $(BIN)/$(PROGRAM)

$(BIN)/$(PROGRAM): $(OBJS) $(DTTSP)
	$(LINK) -o $(BIN)/$(PROGRAM) $(OBJS) $(LIBS)

.c.o:
	$(COMPILE) $(OPTIONS) -c -o $@ $<

$(DTTSP):
	-cd ../DttSP;make

clean:
	-rm -f *.o
	-rm -f $(BIN)/$(PROGRAM)
	-cd ../DttSP;make clean

