#    Purpose: Interact with USB device firmware
#    Created: 2008-08-15 by Opendous Inc.
#    Released under the MIT License
#
#    Last Edit: 2010-03-22 by Loftur Jonasson, TF3LJ
#
#    The below is a simple/crude example of how to
#    control a DG8SAQ/PE0FKO device or or the MOBO
#    firmware through Python
#

import usb, time

# all the critical information regarding the device and the interface and endpoints you plan to use.
#vendorid    = 0x16c0
#productid   = 0x05dc
vendorid    = 0x03eb
productid   = 0x2307
confignum   = 1
#interfacenum= 0
interfacenum= 3
timeout     = 1500


# Control Message bmRequestType Masks
REQDIR_HOSTTODEVICE = (0 << 7)
REQDIR_DEVICETOHOST = (1 << 7)
REQTYPE_STANDARD    = (0 << 5)
REQTYPE_CLASS       = (1 << 5)
REQTYPE_VENDOR      = (2 << 5)
REQREC_DEVICE       = (0 << 0)
REQREC_INTERFACE    = (1 << 0)
REQREC_ENDPOINT     = (2 << 0)
REQREC_OTHER        = (3 << 0)



# loop over all busses and all devices and find the one with the correct vendor and product IDs
# Note that if you have several of the same devices connected, it will select the last
print "STARTED DeviceAccessPy"
busses = usb.busses()
for bus in busses:
    devices = bus.devices
    for dev in devices:
        if (dev.idVendor == vendorid) & (dev.idProduct == productid):
            founddev = dev
            foundbus = bus

# the device has been found, otherwise exit with error
bus = foundbus
dev = founddev

# open the device for communication
handle = dev.open()

# choose which of the device's configurations to use
handle.setConfiguration(confignum)

# choose which interface to interact with
#handle.claimInterface(interfacenum)


#TODO: this while() loop is where you should place your device interaction code
buffer = () # initialize the read/write buffer
##########


maxlen = 32 # size of buffer

#buffer = (10,11,12,13,14,15,16,17)
#buffer = (10,11,12) #,13,14,15,16,17)
#bmRTmask = (REQDIR_HOSTTODEVICE | REQTYPE_VENDOR | REQTYPE_STANDARD)
#handle.claimInterface(interfacenum)
#handle.controlMsg(requestType = bmRTmask, request = 0x37, value = 02,
#      index = 01, buffer = buffer, timeout = timeout)
#handle.releaseInterface()
#print buffer

# Read frequency from Si570 device
bmRTmask = (REQDIR_DEVICETOHOST | REQTYPE_VENDOR | REQTYPE_STANDARD)
handle.claimInterface(interfacenum)
buffer = handle.controlMsg(requestType = bmRTmask, request = 0x3a, value = 00,
    index = 00, buffer = maxlen, timeout = timeout)
handle.releaseInterface()
print "Si570 frequency by value reads:", buffer
freq =  float (buffer[0] + 256 * buffer[1] + 65536*buffer[2] + 256*65536*buffer[3])/2**23
print '%2.06f MHz' %freq

# Set frequency by value
buffer = (0x5c,0x8f,0x82,0x03) # normal
#buffer = (0x03,0x82,0x8f,0x5c) # reverse
bmRTmask = (REQDIR_HOSTTODEVICE | REQTYPE_VENDOR | REQTYPE_STANDARD)
handle.claimInterface(interfacenum)
handle.controlMsg(requestType = bmRTmask, request = 0x32, value = 0,
      index = 0, buffer = buffer, timeout = timeout)
handle.releaseInterface()
print "Si570 32bit frequency value set to:", buffer

# Watchdog reset
#bmRTmask = (REQDIR_DEVICETOHOST | REQTYPE_VENDOR | REQTYPE_STANDARD)
#handle.claimInterface(interfacenum)
#buffer = handle.controlMsg(requestType = bmRTmask, request = 0x41, value = 0xff,
#    index = 0x00, buffer = maxlen, timeout = timeout)
#handle.releaseInterface()
#print buffer

#RX Filter
bmRTmask = (REQDIR_DEVICETOHOST | REQTYPE_VENDOR | REQTYPE_STANDARD)
handle.claimInterface(interfacenum)
try:
    handle.controlMsg(requestType = bmRTmask, request = 0x17, value = 0x00,
        index = 0xff, buffer = maxlen, timeout = timeout)
except:
    time.sleep(0.5) # Wait for a timeout... to fix issues with hosttodevice call
handle.releaseInterface()
print "RX Filters", buffer

#TX Filter
bmRTmask = (REQDIR_DEVICETOHOST | REQTYPE_VENDOR | REQTYPE_STANDARD)
handle.claimInterface(interfacenum)
try:
    handle.controlMsg(requestType = bmRTmask, request = 0x17, value = 0x00,
        index = 0x1ff, buffer = maxlen, timeout = timeout)
except:
    time.sleep(0.5) # Wait for a timeout... to fix issues with hosttodevice call
handle.releaseInterface()
print "TX Filters", buffer

#Transmit on/off
bmRTmask = (REQDIR_DEVICETOHOST | REQTYPE_VENDOR | REQTYPE_STANDARD)
handle.claimInterface(interfacenum)
buffer = handle.controlMsg(requestType = bmRTmask, request = 0x50, value = 0x01,
    index = 0x00, buffer = maxlen, timeout = timeout)
handle.releaseInterface()
print "TXRX",buffer



