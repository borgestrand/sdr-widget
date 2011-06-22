SRCDIR = ../../src
COMMONSRC = $(SRCDIR)/common
CYGWINSRC = $(SRCDIR)/cygwin
PORTTALK = $(CYGWINSRC)/porttalk
USBSRC = $(CYGWINSRC)/usb

TSTDIR = ../../test

OBJ = hw_test.o hw_sdr1000.o ad9854.o \
	ad9854_reg.o pio_reg.o rfe_reg.o \
	hw_reg.o sdr1kusb.o pt_ioctl.o

hw_test : $(OBJ) 
	g++ -o hw_test $(OBJ) -lusb -lpthread -fPIC -O2

hw_test.o : $(TSTDIR)/hw_test.cpp $(CYGWINSRC)/hw_sdr1000.h $(COMMONSRC)/ad9854.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(USBSRC)/sdr1kusb.h
	g++ -c $(TSTDIR)/hw_test.cpp -I$(TSTDIR) -I$(COMMONSRC) -I$(CYGWINSRC) -I$(USBSRC)/ -I$(PORTTALK) -fPIC -O2

hw_sdr1000.o : $(CYGWINSRC)/hw_sdr1000.cpp $(CYGWINSRC)/hw_sdr1000.h $(COMMONSRC)/ad9854.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(USBSRC)/sdr1kusb.h
	g++ -c $(CYGWINSRC)/hw_sdr1000.cpp -I$(COMMONSRC) -I$(CYGWINSRC) -I$(USBSRC) -I$(PORTTALK) -fPIC -O2

ad9854.o : $(COMMONSRC)/ad9854.cpp $(COMMONSRC)/ad9854.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(CYGWINSRC)/hw_sdr1000.h $(USBSRC)/sdr1kusb.h
	g++ -c $(COMMONSRC)/ad9854.cpp -I$(COMMONSRC) -I$(CYGWINSRC) -I$(USBSRC) -I$(PORTTALK) -fPIC -O2

ad9854_reg.o : $(COMMONSRC)/ad9854_reg.cpp $(COMMONSRC)/ad9854_reg.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(CYGWINSRC)/hw_sdr1000.h $(USBSRC)/sdr1kusb.h
	g++ -c $(COMMONSRC)/ad9854_reg.cpp -I$(TSTDIR) -I$(COMMONSRC) -I$(CYGWINSRC) -I$(USBSRC) -I$(PORTTALK) -fPIC -O2

pio_reg.o : $(COMMONSRC)/pio_reg.cpp $(COMMONSRC)/pio_reg.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(CYGWINSRC)/hw_sdr1000.h $(USBSRC)/sdr1kusb.h
	g++ -c $(COMMONSRC)/pio_reg.cpp -I$(TSTDIR) -I$(COMMONSRC) -I$(CYGWINSRC) -I$(USBSRC) -I$(PORTTALK) -fPIC -O2

rfe_reg.o : $(COMMONSRC)/rfe_reg.cpp $(COMMONSRC)/rfe_reg.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(CYGWINSRC)/hw_sdr1000.h $(USBSRC)/sdr1kusb.h
	g++ -c $(COMMONSRC)/rfe_reg.cpp -I$(TSTDIR) -I$(COMMONSRC) -I$(CYGWINSRC) -I$(USBSRC) -I$(PORTTALK) -fPIC -O2

sdr1kusb.o : $(USBSRC)/sdr1kusb.cpp $(USBSRC)/sdr1kusb.h $(COMMONSRC)/common.h
	g++ -c $(USBSRC)/sdr1kusb.cpp -I$(TSTDIR) -I$(COMMONSRC) -I$(CYGWINSRC) -I$(USBSRC) -I$(PORTTALK) -fPIC -O2

hw_reg.o : $(COMMONSRC)/hw_reg.cpp $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h
	g++ -c $(COMMONSRC)/hw_reg.cpp -I$(TSTDIR) -I$(COMMONSRC) -I$(CYGWINSRC) -I$(USBSRC) -fPIC -O2

pt_ioctl.o:	$(PORTTALK)/pt_ioctl.c $(COMMONSRC)/ad9854_reg.h $(COMMONSRC)/hw_reg.h $(COMMONSRC)/common.h $(CYGWINSRC)/hw_sdr1000.h $(USBSRC)/sdr1kusb.h
	g++ -c $(PORTTALK)/pt_ioctl.c -I$(TSTDIR) -I$(COMMONSRC) -I$(CYGWINSRC) -I$(USBSRC) -I$(PORTTALK) -I/usr/include/w32api/ddk -fPIC -O2

all: hw_test

.PHONY : clean

clean :
	rm -f hw_test $(OBJ)
