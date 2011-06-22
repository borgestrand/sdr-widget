SOURCEDIR = ../../src/cygwin
COMMONINC = ../../src/common

CXXFLAGS = -g -O3 -fPIC -I$(SOURCEDIR) -I$(SOURCEDIR)/usb -I/usr/local/include \
	-I/usr/include -I$(COMMONINC) -I$(SOURCEDIR)/usb \
	-I/usr/include/python2.4 -I$(SOURCEDIR)/porttalk

LDFLAGS = -g

LIBS = -lpthread -lusb -lm -lc

OBJ =	ad9854.o\
	ad9854_reg.o\
	hw_reg.o\
	hw_sdr1000.o\
	hw_test.o\
	pio_reg.o\
	rfe_reg.o \
	pt_ioctl.o \
	sdr1kusb.o


python:
	swig -o ./sdr1khw_wrap.cxx -python -c++ $(SOURCEDIR)/sdr1khw.i
	g++ $(CXXFLAGS) -c sdr1khw_wrap.cxx
	g++ -shared $(OBJ) ./sdr1khw_wrap.o -o _sdr1khw.so\
		 -L/usr/lib/python2.4/config -lpython2.4 -lusb

clean:
	/bin/rm -f *.so *.o
