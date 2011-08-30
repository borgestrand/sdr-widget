

bin_PROGRAMS = dspserver

dspserver_CFLAGS = -g -O3 -Wall -I../DttSP
dspserver_CPPFLAGS = -I/usr/local/include
dspserver_LDADD = ../DttSP/libDttSP.a



# dspserver_LDFLAGS = -lm
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
complex.h      \
codec2loc.h    \
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
