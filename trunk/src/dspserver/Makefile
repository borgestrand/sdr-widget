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
#OPTIONS=-g -O3 -msse -msse3 -ffast-math -march=core2 -Wall
OPTIONS=-g -O3 -Wall
INCLUDES=-I. 
LIBS=-L. -L../DttSP -lDttSP -lpthread\
     -lm -lfftw3f -lcodec2

BIN=../../bin

DTTSP=../DttSP/libDttSP.a

PROGRAM=dspserver

COMPILE=$(CC) $(INCLUDES)

SOURCES= main.c\
         ozy.c\
         soundcard.c\
         client.c\
         audiostream.c

OBJS= main.o\
      ozy.o\
      soundcard.o\
      client.o\
      audiostream.o


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

