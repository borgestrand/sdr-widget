#!/usr/bin/python
# -*- coding: cp1252 -*-

############################################################
#
# Widget Control
#
# adapted from MoboControl
#
# Alex Lee, 9V1AL & Loftur Jonasson, TF3LJ
#
############################################################

from PythonCard import about, dialog, graphic, model, clipboard
import ConfigParser

import pprint

import os, sys, platform
import wx

import usb
import array, time

import wx.lib.mixins.listctrl as listmix
#

#############################################################
# all the critical information regarding the USB device and
# the interface and endpoints you plan to use.
#############################################################
vendorid1    = 0x16c0
vendorid2    = 0xfffe
vendorid3    = 0x16d0
productid1   = 0x05dc
productid2   = 0x03e8
productid3   = 0x0007
productid4   = 0x0761 # vendorid3 SDR-Widget UAC1
productid5   = 0x0762 # vendorid3 SDR-Widget UAC2
productid6   = 0x0763 # vendorid3 USB9023 UAC1
productid7   = 0x0764 # vendorid3 USB9023 UAC2
productid8   = 0x0765 # vendorid3 USB5102 UAC1
productid9   = 0x0766 # vendorid3 USB5102 UAC2
productid10   = 0x0767 # vendorid3 USB8741 UAC1
productid11   = 0x0768 # vendorid3 USB8741 UAC2
productid12   = 0x075C # vendorid3 AB-1.x UAC1
productid13   = 0x075D # vendorid3 AB-1.x UAC2
productid14   = 0x075E # vendorid3 QNKTC future use UAC1
productid15   = 0x075F # vendorid3 QNKTC future use UAC2
productid16   = 0x098B # vendorid3 AMB UAC1
productid17   = 0x098C # vendorid3 AMB UAC2
confignum   = 1
interfacenum= 0
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

# Some globals
ON  = 1
OFF = 0
FeatureDisabled = 255

#################################################################
# Our one and only Class
#################################################################
class Launcher(model.Background):

    #############################################################
    # Initialise the works
    #############################################################
    def on_initialize(self, event):
        c = self.components
       
        # Init some Firmware identification variables
        self.firmw_I2C_PE0FKO = OFF
        self.firmw_I2C_Mobo = OFF
        self.firmw_I2C_feature = OFF

        # Default values for a number of of items, some of which may not be supported,
        # depending on the firmware.
 
        self.imageSelection = 10
        self.inSelection = 14
        self.outSelection = 17
        self.adcSelection = 20
        self.dacSelection = 24
        
        # Enumerate the USB
        self.OnUSB(-1)
        time.sleep(0.2)             # Give firmware some breathing space

        # Get number of feature values
        self.handle.claimInterface(interfacenum) # Open the USB device for traffic
        output = self.devicetohost(0x71, 4, 1)
        self.handle.releaseInterface()           # Release the USB device
        max_feature_value_index = output[0]

        try:
            # Get feature values and put in dict
            self.handle.claimInterface(interfacenum) # Open the USB device for traffic

            self.feature_value_dict = {}
            self.feature_value_lookup_dict = {}
	    feature_index = 0
            feature_value_index = 0
            while feature_value_index < max_feature_value_index:
                output = self.devicetohost(0x71, 8, feature_value_index)
                if output[0] == 63:                 # '?'
                    break
                else:
                    output_str = ''.join(map(chr, reversed(output)))

                    if output_str == 'end':
                        feature_index = feature_index + 1
                    else:
                        self.feature_value_dict[feature_value_index] = output_str
                        self.feature_value_lookup_dict[str(feature_index)+output_str] = feature_value_index
                    feature_value_index = feature_value_index + 1
                    if feature_value_index > 100:                  # Just in case '?' not returned
                        break

            self.handle.releaseInterface()           # Release the USB device

        except:
            pass


        # Poll all values for the first time
        try:
            self.on_Refresh_command(-1)
            time.sleep(0.2)             # Give firmware some breathing space

        except:
            pass

    #############################################################
    # StartUSB Button (is also run automatically on startup)
    # Find and open USB port for access
    #############################################################
    def on_USB_command(self, event):
        self.on_initialize(-1)      # Open USB and poll all values
        
    def OnUSB(self, event):
        # loop over all busses and all devices and find the one with the correct vendor and product IDs
        # Note that if you have several of the same devices connected, it will select the last
        #print "STARTED DeviceAccessPy
        busses = usb.busses()
        for bus in busses:
            devices = bus.devices
            for dev in devices:
                if (dev.idVendor == vendorid1) & (dev.idProduct == productid1):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid1) & (dev.idProduct == productid2):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid2) & (dev.idProduct == productid3):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid3) & (dev.idProduct == productid4):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid3) & (dev.idProduct == productid5):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid3) & (dev.idProduct == productid6):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid3) & (dev.idProduct == productid7):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid3) & (dev.idProduct == productid8):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid3) & (dev.idProduct == productid9):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid3) & (dev.idProduct == productid10):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid3) & (dev.idProduct == productid11):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid3) & (dev.idProduct == productid12):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid3) & (dev.idProduct == productid13):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid3) & (dev.idProduct == productid14):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid3) & (dev.idProduct == productid15):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid3) & (dev.idProduct == productid16):
                    founddev = dev
                    foundbus = bus
                    break
                if (dev.idVendor == vendorid3) & (dev.idProduct == productid17):
                    founddev = dev
                    foundbus = bus
                    break
        try:
        # the device has been found, otherwise exit with error
          bus = foundbus
          dev = founddev
        except:
          dlg = wx.MessageDialog(self, "DG8SAQ/MOBO device not found", "USB", wx.OK | wx.ICON_INFORMATION)
          dlg.ShowModal()
          #dlg.Destroy()
          #self.OnExit(-1)
        else:
          # open the device for communication
          self.handle = dev.open()
          # choose which of the device's configurations to use, mustn't be execute on Linux
          if platform.system() == 'Windows': self.handle.setConfiguration(confignum)

          # Get Firmware serial number
          firmw_serial = self.handle.getString( dev.iSerialNumber, 30)

          # Poll firmware for version information
          self.handle.claimInterface(interfacenum)    # Open the USB device for traffic
          output = self.devicetohost(0x00, 0xa55a, 0) # Poll for version (echo value to capture DG8SAQ firmware)
          self.handle.releaseInterface()
          
          if output == (0x5a, 0xa5):
              self.firmw_I2C_PE0FKO = ON              # Certain features cannot not be supported
              firmw_version = 'DG8SAQ 1.4 or 2.0'
          else:
              firmw_version = str(output[1])+'.'+str(output[0])
              if output[1] == 15:
                  self.firmw_I2C_PE0FKO = ON          # Certain features cannot be supported
                  firmw_version = 'PE0FKO '+firmw_version
                  if output[0] > 11:
                      self.firmw_I2C_feature = ON     # I2C poll feature supported in newer PE0FKO firmware
                  else:
                      self.firmw_I2C_feature = OFF
              elif (output[1] == 16) and (output[0]>99):
                  firmw_version = 'SDRWidget '+firmw_version
                  self.firmw_I2C_PE0FKO = OFF
                  self.firmw_I2C_Mobo = ON
                  self.firmw_I2C_feature = ON
              elif (output[1] == 16) and (output[0]>9):
                  self.firmw_I2C_PE0FKO = OFF
                  firmw_version = 'Mobo '+firmw_version
                  self.firmw_I2C_Mobo = ON
                  self.firmw_I2C_feature = ON
              else:
                  self.firmw_I2C_PE0FKO = OFF
                  self.firmw_I2C_feature = ON
                  firmw_version = 'UBW/ATMega '+firmw_version
                  
              self.components.FirmwareSdisplay.text = (firmw_serial)
              self.components.FirmwareVdisplay.text = (firmw_version)


    #############################################################
    # USB Device to Host communications (all comms except for 0x30 - 0x36)
    #############################################################
    def devicetohost(self, req=0, val=0, ind=0, maxlen = 8):
        bmRTmask = (REQDIR_DEVICETOHOST | REQTYPE_VENDOR | REQTYPE_STANDARD)
        buffer = self.handle.controlMsg(requestType = bmRTmask, request = req, value = val,
        index = ind, buffer = 256, timeout = timeout)
        return buffer

    #############################################################
    # USB Host to Device communications (used by Commands 0x30 - 0x36)
    #############################################################
    def hosttodevice(self, req=0, val=0, ind=0, buffer=[]):
        bmRTmask = (REQDIR_HOSTTODEVICE | REQTYPE_VENDOR | REQTYPE_STANDARD)
        try:
            self.handle.controlMsg(requestType = bmRTmask, request = req, value = val,
            index = ind, buffer = buffer, timeout = timeout)
        except:
            time.sleep(0.2) # Wait for a timeout... to fix issues with hosttodevice call
        return

    #############################################################
    # Change Features
    #############################################################
    def on_ComboBoxBoard_textUpdate(self, event):

       self.boardSelection = self.feature_value_lookup_dict['0'+event.target.stringSelection]
       self.handle.claimInterface(interfacenum) # Open the USB device for traffic
       output = self.devicetohost(0x71, 3, (2 + (self.boardSelection * 256)))
       self.handle.releaseInterface()           # Release the USB device

       self.on_Refresh_command(-1)		# Refresh

    def on_ComboBoxImage_textUpdate(self, event):
       self.imageSelection = self.feature_value_lookup_dict['1'+event.target.stringSelection]
       self.handle.claimInterface(interfacenum) # Open the USB device for traffic
       output = self.devicetohost(0x71, 3, (3 + (self.imageSelection * 256)))
       self.handle.releaseInterface()           # Release the USB device

       self.on_Refresh_command(-1)		# Refresh

    def on_ComboBoxIn_textUpdate(self, event):
       self.inSelection = self.feature_value_lookup_dict['2'+event.target.stringSelection]
       self.handle.claimInterface(interfacenum) # Open the USB device for traffic
       output = self.devicetohost(0x71, 3, (4 + (self.inSelection * 256)))
       self.handle.releaseInterface()           # Release the USB device

       self.on_Refresh_command(-1)		# Refresh

    def on_ComboBoxOut_textUpdate(self, event):
       self.outSelection = self.feature_value_lookup_dict['3'+event.target.stringSelection]
       self.handle.claimInterface(interfacenum) # Open the USB device for traffic
       output = self.devicetohost(0x71, 3, (5 + (self.outSelection * 256)))
       self.handle.releaseInterface()           # Release the USB device

       self.on_Refresh_command(-1)		# Refresh

    def on_ComboBoxAdc_textUpdate(self, event):
       self.adcSelection = self.feature_value_lookup_dict['4'+event.target.stringSelection]
       self.handle.claimInterface(interfacenum) # Open the USB device for traffic
       output = self.devicetohost(0x71, 3, (6 + (self.adcSelection * 256)))
       self.handle.releaseInterface()           # Release the USB device

       self.on_Refresh_command(-1)		# Refresh

    def on_ComboBoxDac_textUpdate(self, event):
       self.dacSelection = self.feature_value_lookup_dict['5'+event.target.stringSelection]
       self.handle.claimInterface(interfacenum) # Open the USB device for traffic
       output = self.devicetohost(0x71, 3, (7 + (self.dacSelection * 256)))
       self.handle.releaseInterface()           # Release the USB device

       self.on_Refresh_command(-1)		# Refresh

    def on_ComboBoxLCD_textUpdate(self, event):
       self.LCDSelection = self.feature_value_lookup_dict['6'+event.target.stringSelection]
       self.handle.claimInterface(interfacenum) # Open the USB device for traffic
       output = self.devicetohost(0x71, 3, (8 + (self.LCDSelection * 256)))
       self.handle.releaseInterface()           # Release the USB device

       self.on_Refresh_command(-1)		# Refresh

    def on_ComboBoxLog_textUpdate(self, event):
       self.LogSelection = self.feature_value_lookup_dict['7'+event.target.stringSelection]
       self.handle.claimInterface(interfacenum) # Open the USB device for traffic
       output = self.devicetohost(0x71, 3, (9 + (self.LogSelection * 256)))
       self.handle.releaseInterface()           # Release the USB device

       self.on_Refresh_command(-1)		# Refresh


    #####################################
    #  Read firmware features
    #####################################
    def on_Refresh_command(self, event):
        try:
            # Get IN Type
            self.handle.claimInterface(interfacenum)# Open the USB device for traffic
            output = self.devicetohost(0x71, 4, 4)
            self.handle.releaseInterface()          # Release the USB device
            InType = self.feature_value_dict[output[0]]
            self.components.InType.text = InType
        except:
            pass

        try:
            # Get OUT Type
            self.handle.claimInterface(interfacenum)# Open the USB device for traffic
            output = self.devicetohost(0x71, 4, 5)
            self.handle.releaseInterface()          # Release the USB devic
            OutType = self.feature_value_dict[output[0]]
            self.components.OutType.text = OutType
        except:
            pass

        try:
            # Get ADC Type
            self.handle.claimInterface(interfacenum)# Open the USB device for traffic
            output = self.devicetohost(0x71, 4, 6)
            self.handle.releaseInterface()          # Release the USB device
            AdcType = self.feature_value_dict[output[0]]
            self.components.AdcType.text = AdcType
        except:
            pass

        try:
            # Get DAC Type
            self.handle.claimInterface(interfacenum)# Open the USB device for traffic
            output = self.devicetohost(0x71, 4, 7)
            self.handle.releaseInterface()          # Release the USB device
            DacType = self.feature_value_dict[output[0]]
            self.components.DacType.text = DacType
        except:
            pass

        try:
            # Get LCD Type
            self.handle.claimInterface(interfacenum)# Open the USB device for traffic
            output = self.devicetohost(0x71, 4, 8)
            self.handle.releaseInterface()          # Release the USB device
            LCDType = self.feature_value_dict[output[0]]
            self.components.LCDType.text = LCDType
        except:
            pass

        try:
            # Get Log Type
            self.handle.claimInterface(interfacenum)# Open the USB device for traffic
            output = self.devicetohost(0x71, 4, 9)
            self.handle.releaseInterface()          # Release the USB device
            LogType = self.feature_value_dict[output[0]]
            self.components.LogType.text = LogType
        except:
            pass


        try:
           # Get Board Type
            self.handle.claimInterface(interfacenum) # Open the USB device for traffic
            output = self.devicetohost(0x71, 4, 2)
            self.handle.releaseInterface()           # Release the USB device
            
            boardType = self.feature_value_dict[output[0]]
            self.components.BoardType.text = boardType

        except:
            pass

        try:
            # Get Image Type
            self.handle.claimInterface(interfacenum)# Open the USB device for traffic
            output = self.devicetohost(0x71, 4, 3)
            self.handle.releaseInterface()          # Release the USB device

            imageType = self.feature_value_dict[output[0]]
 
            self.components.ImageType.text = imageType
        except:
            pass

    ################################################################
    # Reset the Widget
    ################################################################
    def on_btnReset_command(self, event):
        # Firmware Reset
        try:
            self.handle.claimInterface(interfacenum) # Open the USB device for traffic
            self.devicetohost(0x0f, 0, 0)            # Reset
            self.handle.releaseInterface()
        except:
            time.sleep(5)   # Wait for Mobo uC to reset itself
            self.OnUSB(-1) # Read settings back again
            self.on_Refresh_command(-1)
   
    ################################################################
    # Factory Reset the Mobo
    ################################################################
    def on_btnFactoryReset_command(self, event):
        try:
            self.handle.claimInterface(interfacenum) # Open the USB device for traffic
            self.devicetohost(0x41, 0xff, 0) # Reset to Default
            self.handle.releaseInterface()
        except:
            time.sleep(5)   # Wait for Mobo uC to reset itself
            self.OnUSB(-1) # Read settings back again
            self.on_Refresh_command(-1)
       

    #############################################################
    # Exit Button
    #############################################################
    def on_Exit_command(self, event):
        self.Close()


#############################################################
# Here be Main
#############################################################
if __name__ == '__main__':
    app = model.Application(Launcher)

    # Main window resize on Windows platform
    if platform.system() == 'Windows': 
        bg = app.getCurrentBackground()
        bg.size = (922, 620)

    app.MainLoop()
