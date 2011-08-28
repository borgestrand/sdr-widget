

bin_PROGRAMS = dspserver

dspserver_CFLAGS = -g -O3 -Wall -I../DttSP

dspserver_LDADD = ../DttSP/libDttSP.a


dspserver_LDFLAGS = -lpulse-simple -lusb -lm -lfftw3f -lcodec2

dspserver_SOURCES = main.c\
         ozy.c\
         soundcard.c\
         client.c\
         audiostream.c \
	 audiostream.h  \
banal.h        \
buffer.h       \
bufvec.h       \
client.h       \
codec2.h       \
complex.h      \
cxops.h        \
datatypes.h    \
defs.h         \
dttspagc.h     \
dttsp.h        \
fastrig.h      \
fromsys.h      \
iosdr.h        \
main.h         \
ozy.h          \
soundcard.h    \
splitfields.h  \
version.h
