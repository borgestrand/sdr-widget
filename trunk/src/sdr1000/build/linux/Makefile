SRCDIR = ../../src
COMMONSRC = $(SRCDIR)/common
LINUXSRC = $(SRCDIR)/linux
USBSRC = $(LINUXSRC)/usb
TSTDIR = ../../test
SERVERDIR = ../../server
BINDIR = ../../../../bin

OBJ =  server.o sdr1000.o sdr1000io.o client.o receiver.o util.o listener.o hw_sdr1000.o ad9854.o \
	ad9854_reg.o pio_reg.o rfe_reg.o \
	hw_reg.o sdr1kusb.o

$(BINDIR)/sdr1000server: $(OBJ)
	g++ -o $(BINDIR)/sdr1000server $(OBJ) -lusb -lpthread -lportaudio -fPIC -O2

server.o : $(SERVERDIR)/server.cpp $(LINUXSRC)/hw_sdr1000.h $(COMMONSRC)/ad9854.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(USBSRC)/sdr1kusb.h
	g++ -c $(SERVERDIR)/server.cpp -I$(TSTDIR) -I$(COMMONSRC) -I$(LINUXSRC) -I$(LINUXSRC)/usb/ -fPIC -O2 -Wno-write-strings

sdr1000.o : $(SERVERDIR)/sdr1000.c $(LINUXSRC)/hw_sdr1000.h $(COMMONSRC)/ad9854.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(USBSRC)/sdr1kusb.h
	gcc -c $(SERVERDIR)/sdr1000.c -I$(TSTDIR) -I$(COMMONSRC) -I$(LINUXSRC) -I$(LINUXSRC)/usb/ -fPIC -O2 -Wno-write-strings

sdr1000io.o : $(SERVERDIR)/sdr1000io.c $(LINUXSRC)/hw_sdr1000.h $(COMMONSRC)/ad9854.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(USBSRC)/sdr1kusb.h
	gcc -c $(SERVERDIR)/sdr1000io.c -I$(TSTDIR) -I$(COMMONSRC) -I$(LINUXSRC) -I$(LINUXSRC)/usb/ -fPIC -O2 -Wno-write-strings

client.o : $(SERVERDIR)/client.c $(LINUXSRC)/hw_sdr1000.h $(COMMONSRC)/ad9854.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(USBSRC)/sdr1kusb.h
	gcc -c $(SERVERDIR)/client.c -I$(TSTDIR) -I$(COMMONSRC) -I$(LINUXSRC) -I$(LINUXSRC)/usb/ -fPIC -O2 -Wno-write-strings

receiver.o : $(SERVERDIR)/receiver.c $(LINUXSRC)/hw_sdr1000.h $(COMMONSRC)/ad9854.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(USBSRC)/sdr1kusb.h
	gcc -c $(SERVERDIR)/receiver.c -I$(TSTDIR) -I$(COMMONSRC) -I$(LINUXSRC) -I$(LINUXSRC)/usb/ -fPIC -O2 -Wno-write-strings

util.o : $(SERVERDIR)/util.c $(LINUXSRC)/hw_sdr1000.h $(COMMONSRC)/ad9854.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(USBSRC)/sdr1kusb.h
	gcc -c $(SERVERDIR)/util.c -I$(TSTDIR) -I$(COMMONSRC) -I$(LINUXSRC) -I$(LINUXSRC)/usb/ -fPIC -O2 -Wno-write-strings

listener.o : $(SERVERDIR)/listener.c $(LINUXSRC)/hw_sdr1000.h $(COMMONSRC)/ad9854.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(USBSRC)/sdr1kusb.h
	gcc -c $(SERVERDIR)/listener.c -I$(TSTDIR) -I$(COMMONSRC) -I$(LINUXSRC) -I$(LINUXSRC)/usb/ -fPIC -O2 -Wno-write-strings

hw_test.o : $(TSTDIR)/hw_test.cpp $(LINUXSRC)/hw_sdr1000.h $(COMMONSRC)/ad9854.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(USBSRC)/sdr1kusb.h
	g++ -c $(TSTDIR)/hw_test.cpp -I$(TSTDIR) -I$(COMMONSRC) -I$(LINUXSRC) -I$(LINUXSRC)/usb/ -fPIC -O2 -Wno-write-strings

hw_sdr1000.o : $(LINUXSRC)/hw_sdr1000.cpp $(LINUXSRC)/hw_sdr1000.h $(COMMONSRC)/ad9854.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(USBSRC)/sdr1kusb.h
	g++ -c $(LINUXSRC)/hw_sdr1000.cpp -I$(COMMONSRC) -I$(LINUXSRC) -I$(USBSRC) -fPIC -O2 -Wno-write-strings

ad9854.o : $(COMMONSRC)/ad9854.cpp $(COMMONSRC)/ad9854.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(LINUXSRC)/hw_sdr1000.h $(USBSRC)/sdr1kusb.h
	g++ -c $(COMMONSRC)/ad9854.cpp -I$(COMMONSRC) -I$(LINUXSRC) -I$(USBSRC) -fPIC -O2 -Wno-write-strings

ad9854_reg.o : $(COMMONSRC)/ad9854_reg.cpp $(COMMONSRC)/ad9854_reg.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(LINUXSRC)/hw_sdr1000.h $(USBSRC)/sdr1kusb.h
	g++ -c $(COMMONSRC)/ad9854_reg.cpp -I$(TSTDIR) -I$(COMMONSRC) -I$(LINUXSRC) -I$(USBSRC) -fPIC -O2 -Wno-write-strings

pio_reg.o : $(COMMONSRC)/pio_reg.cpp $(COMMONSRC)/pio_reg.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(LINUXSRC)/hw_sdr1000.h $(USBSRC)/sdr1kusb.h
	g++ -c $(COMMONSRC)/pio_reg.cpp -I$(TSTDIR) -I$(COMMONSRC) -I$(LINUXSRC) -I$(USBSRC) -fPIC -O2 -Wno-write-strings

rfe_reg.o : $(COMMONSRC)/rfe_reg.cpp $(COMMONSRC)/rfe_reg.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(LINUXSRC)/hw_sdr1000.h $(USBSRC)/sdr1kusb.h
	g++ -c $(COMMONSRC)/rfe_reg.cpp -I$(TSTDIR) -I$(COMMONSRC) -I$(LINUXSRC) -I$(USBSRC) -fPIC -O2 -Wno-write-strings

sdr1kusb.o : $(USBSRC)/sdr1kusb.cpp $(USBSRC)/sdr1kusb.h $(COMMONSRC)/common.h
	g++ -c $(USBSRC)/sdr1kusb.cpp -I$(TSTDIR) -I$(COMMONSRC) -I$(LINUXSRC) -I$(USBSRC) -fPIC -O2 -Wno-write-strings

hw_reg.o : $(COMMONSRC)/hw_reg.cpp $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h
	g++ -c $(COMMONSRC)/hw_reg.cpp -I$(TSTDIR) -I$(COMMONSRC) -I$(LINUXSRC) -I$(USBSRC) -fPIC -O2 -Wno-write-strings

all: hw_test

.PHONY : clean

clean :
	rm -f hw_test $(OBJ)
