#
# Makefile for Linux version of ghpsdr3
# modified by Dave Larsen, KV0S
#
CC            = gcc
CFLAGS        = 
LIBS          = -lpthread -lusb-1.0
TARGET 	=	../../bin/server
SOURCES = 	bandscope.c  \
		listener.c \
		receiver.c \
		transmitter.c \
		util.c \
		client.c \
		ozy.c \
		ozyio.c \
		metis.c \
		server.c
OBJECTS =       bandscope.o  \
                listener.o \
                receiver.o \
                transmitter.o \
                util.o \
                client.o \
                ozy.o \
                ozyio.o \
                metis.o \
                server.o

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"


all: Makefile $(TARGET)

$(TARGET):  $(OBJECTS)
	$(CC) -o $(TARGET) $(LIBS) $(OBJECTS) 

clean:
	-rm -rf $(TARGET)
	-rm -rf $(OBJECTS)
