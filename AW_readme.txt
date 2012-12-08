Audio Widget README
===================

Read this file if you are curious about the Audio Widget project. Most of it
also applies to the SDR Widget project. This document focuses on using and 
modifying the Audio Widget. It does not (yet) focus on percieved audio quality.

Please note that implementations of the Audio Widget and its drivers come under
many names. Depending on your firmware version it may appear under names
containing "QNKTC", "Yoyodyne", "Audio Widget", "SDR Widget", "ASIO UAC2", 
"DG8SAQ". 

Version 20120807 BSB initial
        20120808 BSB old HW additions
        20120809 BSB Windows listening
        20120810 BSB Various minor edits
        20120812 BSB Atmel batchisp/Flip debugging
        20120815 Alex's .mpdconf file copied in
        20120825 Alex's feedback text copied in, UAC2 on Windows edits
        20120917 Christian's input on Linux dfu programming
        20121004 Added Nikolay's recipe on firmware builds
		20121208 Updated Flip version

You should read this file from the top without skipping too much. Depending on 
your ambition level you may finish it sooner or later. More and more complex
topics will be introduced as you read on. If you wish to modify the hardware 
and not the software you may skip the sections on WidgetControl and firmware.

Early revisions are quite AB-1.2 and Windows centric. That is not the purpose of
this file! Please suggest additions or changes and send them to me.

Some sections are split into Linux and Windows. The Windows text is developed on
a Windows 7 system. Compatibility with Windows XP may be there, but is not
prioritized. 

The increasingly more complex topics of this text are:
- Project introduction and history
- Listening to audio UAC1
- Listening to audio UAC2
- Installing drivers 
- Installing players
- WidgetControl software
- Installing new firmware
- Compiling new firmware
- Modding the hardware
- Modding Windows drivers
- Appendix 1 - git primer
- Appendix 2 - UAC2 Feedback Mechanism



Project introduction and history
================================

The project started out as the SDR Widget, an open source system of firmware,
hardware and programs for HAMs / Radio Amateurs. Late 2010 work was initiated
to extract and reuse parts of the project for Hi-Fi / Hi-End audio purposes. 
This project was named Audio Widget (AW). 

The projects implement Asyncronous USB Audio in a generic MCU, the AT32UC3A3
from Atmel. 

You are welcome to participate on our mailing list and forums. Even though some
forums threads may seem long and advanced, new users are very welcome to join:
  http://www.diyaudio.com/forums/digital-source/185761-open-source-usb-interface-audio-widget-170.html#post3109786
  http://www.hifisentralen.no/forumet/diy-og-utvikling-ha-yttalere-forsterkere-etc/61961-usb-i2s-module-analog-board-version-1-1-a-3.html
  http://www.audiofaidate.org/forum/viewtopic.php?f=22&t=7813

Audio Widget mailing list. How to join?...
  https://groups.google.com/forum/?fromgroups#!forum/audio-widget

SDR Widget homepage, wiki and downloads include lots of important information:
  http://code.google.com/p/sdr-widget/
  http://code.google.com/p/sdr-widget/w/list
  http://code.google.com/p/sdr-widget/downloads/list

The two projects use the same firmware code base. (See Compiling new firmware
below.) The hardware implementations are quite different. Three families of
hardware exist:

- SDR Widget hardware by George Boudreau / Yoyodyne consulting
  ...

- Single-board Audio Widget hardware by George Boudreau / Yoyodyne consulting
  ...

- Modular Audio Widget by Børge Strand-Bergesen / QNKTC
  The USB-I2S module sits on a separate Analog Board. The module may be moved
  onto other analog boards. The Analog Board is made to be modded if one wants
  to. These implementations have been named AB-1, AB-1.1, AB-1.12, AB-1.13 and
  AB-1.2. More info about the most recent, AB-1.2:
    http://www.qnktc.com
  Old and sold out QNKTC hardware:
    http://www.qnktc.com/ab_11/AB_11.html
    http://www.qnktc.com/mod_ab1_old/

The project is open source. That applies to firmware and driver source code and
to hardware schematics. 



Listening to audio UAC1
=======================

USB Audio Class 1 is the default operating mode of the AB-1.2. It is plug-and-
play on Windows, Linux and Mac. UAC1 supports sample rates up to 24/48. One 
might believe 24/96 is the UAC1 limit, but that is not the case with the Atmel
AVR32 MCU the project is built on. Their chip has endpoint buffer size limited
to 512 bytes. 1kbyte would allow 24/96 on UAC1. UAC1 uses USB 1.1 (Full Speed).

UAC1 uses asynchronous USB audio with the DAC as the timing reference.



Listening to audio UAC1 - Linux
===============================

Which distribution is recommended, which player is good, how to set up player
sample rates according to music and not fixed....

How to select playback hardware....

This is a recommended .mpdconf file for mpd. Note last line for no automatic
resampling. 

music_directory "~/Music"
playlist_directory "~/.mpd/playlists" 
db_file "~/.mpd/mpd.db"
log_file "~/.mpd/mpd.log" 
audio_output {
    type "alsa"
    name "My ALSA Device"
    device "hw:1,0"    # your device may be hw:0,0 or hw:1,0 or hw:2,0 etc
            # check with
            # $ aplay -l
    auto_resample "no"
}



Listening to audio UAC1 - Windows
=================================

The Audio Widget should show up in Device Manager under "Sound, video and game
controllers". 

Usually, your newly plugged in audio device will become the new default 
device. But that is not necessarily always the case. To make sure the Audio
Widget is the default playback (and / or communication) device, do as follows:

- Right-click the little speaker icon in the bottom-right corner
- Left-click Playback devices
- Right-click DG8SAQ-I2C / Audio Widget / QNKTC... / Yoyodyne...
- Left-click Set as Default Device
- To use with Skype etc. left-click Set as Default Communication Device 

On Windows 7 make sure you're sampling at 44.1 (or 48ksps) depending on your 
music. 44.1kHz is the sampling frequency of CDs and the most likely sampling 
rate used. That way you bypass the OS's built-in sample rate converter which 
would add artefacts to the sound. Some players may take exclusive control, but
don't count on it.

- Left-click the little speaker icon in the bottom-right corner
- Left-click the icon on top of the volume control
- The Output Properties window should appear.
- Click Advanced
- Choose Default Format = 44100 from the pull-down menu
- Tick all Allow exclusive options
- Click Apply
- Click Enhancements
- Disable all sound effects
- Click Apply and close the window



Listening to audio UAC2
=======================

Please refer to the below sections on installing drivers and players. Simple
step-by-step for Windows users can be found at:
  http://www.qnktc.com/Win_cleanup.pdf
  http://www.qnktc.com/Win_install.pdf

USB Audio Class 2 supports sample rates up to 32/192. This is limited by the 
serial interfaces of the Atmel MCU, not USB. UAC2 is supported by the operating
system on Linux (kernel > 2.6.38) and Mac. UAC2 uses USB 2.0 (High speed).

On Windows you will need a separate driver. Commercial drivers for all Windows
programs have been tested but are not included in the project. For certain (but
not all) Windows programs the ASIO protocol can be used with UAC2. An open 
source UAC2 ASIO driver is part of the project. (See Installing drivers below.)

NB: On Widows, the Audio Widget wil NOT show up in Device Manager under "Sound,
video and game controllers". This is the case when you first plug it in, and it
is the case when the open source UAC2 ASIO driver is used. (Commercial drivers
will make it appear in Device Manager, but as stated above and in the next
section, they are not part of the project.)

If you only plan to use Linux, Mac and ASIO enabled players on Windows, use 
UAC2. If you want to use generic Windows programs, use UAC1. 

To change between UAC1 and UAC2, do as follows: Press the Prog button until the
front LED changes color and then goes dark. Then release the Prog button. After
that click and release the Reset button. A red front LED indicates UAC2. A green
front LED indicates UAC1. Do not confuse the front LED with the LEDs on the 
USB-I2S module. 



Installing drivers - Windows
============================

Driver installation is only needed on Windows. 

You will need to download and install AWSetup.zip from:
  https://sites.google.com/site/nikkov/home/microcontrollers/audio-widget/

This package contains Windows ASIO drivers, drivers and Windows implementation
of WidgetControl. 

See Modding Windows Drivers below for more details. The source code is found 
here:
  https://github.com/nikkov/Win-Widget

It is possible to use the Audio Widget with commercial UAC2 drivers. However, 
nobody is currently offering these drivers for sale with a convenient single-
user licence. A group buy may happen. If you are interested, please contact the
audio-widget mailing list.

If you have never used an UAC2 or Audio Widget / SDR Widget driver, you may 
probably skip the rest of this section. If you are upgrading to new drivers it
is highly recommended that you clean up any old drivers which may have been in
use by the OS. The below link has a good recipe. The text below contains a copy
of it:
  http://www.tech-recipes.com/rx/504/how-to-uninstall-hidden-devices-drivers-and-services/

0 -  Unplug Audio Widget from your computer.

1 -  Uninstall AudioWidget in Start menu or C:\Program Files (x86)\Audio-Widget

2 -  Open the Start menu and choose Run. Type in "cmd". On the icon on top, 
     right-click and choose "Run as Administrator". Click "OK" in the User 
     Account Control window.

3 -  At the command prompt, type in "set devmgr_show_nonpresent_devices=1" 
     and press Enter. (Note that nothing seems to happen. This is expected. 
     You are actually setting an environment variable which is going to help you
     to see hidden devices.)

4 -  On the next command prompt line, type devmgmt.msc and press Enter. This 
     will launch the Windows Device Manager Console.
     
5 -  In the Device Manager Console, from the View menu, select Show Hidden 
     Devices.

6 -  Search under tabs for 
     "libusbK USB Devices"
     "Sound, video..."
     "Audio-Widget..."
     There, delete and uninstall anything which rings of:
     "Audio-Widget"
     "SDR-Widget"
     "DG8SAQ"
     "QNKTC"
     "Yoyodyne"

7 -  You should now be ready to install the new fresh version of the drivers!     

Windows is also notorious for trying to "help" you locate the driver it 
believes you need. Here is a link to a text on modifying this "help":
  http://www.addictivetips.com/windows-tips/how-to-disable-automatic-driver-installation-in-windows-vista/

For the curious, here is an introduction about how the ASIO driver works. You 
may skip the rest of this section if you are not curious. Normally, Windows 
drivers run in Kernel mode, and are recognized as audio devices in the Control
Panel. See http://msdn.microsoft.com/en-us/library/windows/desktop/dd316780%28v=vs.85%29.aspx

The ASIO driver, on the other hand, represents a protocol which runs entirely 
in User mode. An ASIO driver is compiled into a .dll file. When the ASIO driver 
is istalled, it identifies itself and its .dll file at certains places in the 
Windows registry. ASIO enabled players will search these places and find the 
installed ASIO drivers, let the user choose one of then, and then start calling
functions in its .dll file. In the case of the Audio Widget, the driver is 
compiled into asiouac2.dll. This file is _typically_ installed into 
C:\Program Files\Audio-Widget or C:\Program Files (x86)\Audio-Widget. 

If you want to try out an experimental asiouac2.dll, just stop all ASIO 
players, make a backup of asiouac2.dll and copy the new one into the location
mentioned above. Then start playing again.

This file contains compiled UAC2 logic. The USB interface itself is handled by
libusbK. The Audio Widget hardware and firmware in UAC2mode are supported 
through a generic driver in Kernel mode, not a specialized audio driver. This
generic driver is in libusbK.sys. Its functions are accessed through User mode
libraries in libusbK.dll and libusb0.dll. These files exist in different 
versions for 32 and 64 bit Windows versions. asiouac2.dll uses these User mode
libraries and the generic Kernel mode driver in order to access the actual USB
hardware.

When the Audio Widget hardware is installed with a driver, the driver's .inf
file is a modified generic libusbK file. That way it is accessed through the 
three libusbK files described above. It is only through asiouac2.dll that it 
plays back audio.



Installing players - Linux
==========================

Which distribution is recommended, which player is good, how to set up player
sample rates according to music and not fixed....



Installing players - Windows
============================

Any Windows audio software should work with the Audio Widget in UAC1 mode. In 
UAC2 two players are recommened; foobar2000 and J-River Media Center. Only the 
former comes free of charge. For foobar2000 you need to install the ASIO 
protocol separately. Links:
  http://www.foobar2000.org/
  http://www.foobar2000.org/components - search for ASIO
  http://www.jriver.com/

For both players you will have to set up ASIO as the output device. For 
foobar2000 you will need to look up and follow instructions to add ASIO as a
component.

We are currently (August 2012) experiencing noise in foobar2000 some times when 
the time bar is pulled back and forth. This is being debugged. We are not 
experiencing noise in J-River Media Center. If you experience any kind of noise,
do not hesitate to contact the Audio Widget mailing list. UPDATE: As of October,
2012 firmware updates seem to have reduced noise. Still, if you hear unexpected
signals, let the developers know!



WidgetControl software - Linux
==============================

WidgetControl is a program which lets you select features in the firmware. As
a casual listener you will probably not have to use this program. It lets you
select UAC1/2 and several other parameters. 

For the knowledge of firmware developers, features of the firmware are selected
in the makefile. Defaults written to MCU flash memory can be modified with 
WidgetControl. 

A word of notice: "Save" only writes the new parameter to MCU flash. You will 
need to reset the Widget as well. Clicking "Reset" in a WidgetControl
implementation will take 5-8 seconds to take effect. Clicking the actual Reset
button will be faster.

Linux implementations of WidgetControl are:
.py implementation and prerequisites... which will also work on Widows...
.c implementation...

For the AudioWidget you should use the following settings:
- Board Type = usbi2s
- ADC Type = none
- Filter Type = fir (This setting controls a single GPIO pin for PCM5102)
- Image Type = uac1_audio (or uac2_audio, see Listening to audio UAC2 above)
- DAC Type = generic
- Quirks = quirk_none for Windows and Mac, quirk_linux for Linux
- IN Type = normal
- LCD Type = hd44780
- OUT Type = normal
- Log Type = none



Widgetcontrol software - Windows
================================

Please see above for the introduction on WidgetControl and its use. 

WidgetControl for windows (and it required drivers) are part of the AWSetup.zip
package. (See Installing drivers - Windows above.)

It is also possible to run the Python implementation of WidgetControl on 
Windows. It has several prerequisites, and is not really needed on Windows
anymore...



Installing new firmware - Linux
===============================

These unstructions are for programming in Linux Ubuntu 10.04 and later.

NOTE: the stock dfu-programmer currently does NOT support "User Page" 
programming. User Page is used in the sdr-widget firmware to store radio 
control parameters such as filter crossover points and calibration data (like 
EEPROM).

1 -  I have a hacked version of dfu-programmer which skips the "User Page" 
     flashing without exiting in Address Error.  This hacked dfu-programmer 
     has been tested to be able to flash all the existing firmware that I have
     tested. You install this hacked version instead of the one from Ubuntu. It
     can be downloaded from:
       http://code.google.com/p/sdr-widget/downloads/list

     Note: (advanced) If you need to compile the dfu-programmer source code to 
     your machine (e.g. if you need 32bit), you may have issues linking against 
     libusb. The bug is in the build script and you can fix this by running the 
     linking command yourself and move -lusb to the end of the command.

2 -  Next, install the ELF to HEX conversion utility:
     sudo apt-get install binutils-avr

3 -  You create a script called, say, program-widget:

#!/bin/bash
##
## program the sdr-widget under linux as of Sun May 1, 2011
## accepts either a widget.elf file or a widget.hex file
##
echo program-widget with $1
[[ -z $1 ]] && echo "no file specified to program-widget" && exit 1
[[ ! -f $1 ]] && echo "no such file '$1'" && exit 1
case $1 in
  *.elf)
  hex=/tmp/`basename $1 .elf`.hex
  objcopy -O ihex $1 $hex && \
    dfu-programmer at32uc3a3256 erase --debug 6  && \
    dfu-programmer at32uc3a3256 flash --suppress-bootloader-mem $hex --debug 6 && \
    dfu-programmer at32uc3a3256 reset --debug 4 && \
    rm $hex && \
    exit 0
  ;;
  *.hex)
  dfu-programmer at32uc3a3256 erase --debug 6  && \
    dfu-programmer at32uc3a3256 flash --suppress-bootloader-mem $1 --debug 6 && \
    dfu-programmer at32uc3a3256 reset --debug 4 && \
    exit 0
  ;;
  *)
  echo unrecognized file format for programming: $1
  exit 1
esac

5 -  Create (or modify an existing) 99-avrtools.rules in /ect/udev/rules.d

# ICE50:
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb", ATTRS{idVendor}=="03eb",
ATTRS{idProduct}=="2101", MODE:="0666"

# JTAGICE mkII:
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb", ATTRS{idVendor}=="03eb",
ATTRS{idProduct}=="2103", MODE:="0666"

# AVRISP mkII:
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb", ATTRS{idVendor}=="03eb",
ATTRS{idProduct}=="2104", MODE:="0666"

# AVRONE:
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb", ATTRS{idVendor}=="03eb",
ATTRS{idProduct}=="2105", MODE:="0666"

# STK600:
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb", ATTRS{idVendor}=="03eb",
ATTRS{idProduct}=="2106", MODE:="0666"

# AVR Dragon:
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb", ATTRS{idVendor}=="03eb",
ATTRS{idProduct}=="2107", MODE:="0666"

# RzUsbStick:
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb", ATTRS{idVendor}=="03eb",
ATTRS{idProduct}=="210a", MODE:="0666"

# QT600:
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb", ATTRS{idVendor}=="03eb",
ATTRS{idProduct}=="2114", MODE:="0666"

# QT600P:
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb", ATTRS{idVendor}=="03eb",
ATTRS{idProduct}=="2116", MODE:="0666"

# Add support AT32UC3A0128 AT32UC3A0256 AT32UC3A0512 
# AT32UC3A1128 AT32UC3A1256 AT32UC3A1512
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb", ATTRS{idVendor}=="03eb",
ATTRS{idProduct}=="2ff8", MODE:="0666"

# Add support AT32UC3A3256
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb", ATTRS{idVendor}=="03eb",
ATTRS{idProduct}=="2ff1", MODE:="0666"

# Add support  AT32UC3B0128 AT32UC3B0256 AT32UC3B064 
# AT32UC3B1128 AT32UC3B1256 AT32UC3B164
SUBSYSTEM=="usb", ENV{DEVTYPE}=="usb", ATTRS{idVendor}=="03eb",
ATTRS{idProduct}=="2ff6", MODE:="0666"

6 -  Reboot for this udev rule to take effect.

7 -  To program:
       Hold Prog, click and release Rest, release Prog
       >  ./program-widget widget.elf

Note: Steps 5-6 ensure that all users get the correct permissions to program 
the device, but udev rules do not work correctly, you can try executing the 
command as root (sudo). 



Installing new firmware - Windows
=================================

New firmware (.elf file) is installed by means of the batchisp program from 
Atmel. It is part of their Flip package. For batchisp debugging see the end of 
this section.

First of all, Atmel Flip is NOT easy to install and use. After the lengthy
recipe below there is a section on debugging if things don't work. Don't give
up if things are tricky the first time!

1  - May be omitted depending on your setup: Install a Java Virtual Machine from
     http://java.com/en/download/index.jsp

2  - May be omitted depending on your setup: Install C++ runtime from Microsoft:
     http://www.microsoft.com/en-us/download/details.aspx?id=5555

3  - Install Atmel Flip 3.4.7 from http://www.atmel.com/tools/FLIP.aspx and 
     choose the version which requires Java Runtime Environment to be pre-
	 installed. (You want your Java from Oracle, not from Atmel!)
     This text assumes you install to "C:\Program Files (x86)\Atmel\Flip 3.4.7"

4  - Copy files from Add_to_flip345_bin.zip from
     http://code.google.com/p/sdr-widget/downloads/detail?name=Add_to_flip345_bin.zip
     to folder 
     "C:\Program Files (x86)\Atmel\Flip 3.4.7\bin"

5  - Download the latest firmware from 
     http://code.google.com/p/sdr-widget/downloads/list
     or from the /Release folder of locally compiled firmware. Save it to 
     somewhere convenient, for example
     "C:\Program Files (x86)\Atmel\Flip 3.4.7\bin"

6  - Do the following. Ignore and approve any messages about driver not being 
     signed. Start, Run, hdwwiz.exe, Next, Install the hardware ... manually, 
     Next, Show All, Next, Have Disk, Browse to 
     "C:\Program Files (x86)\Atmel\Flip 3.4.7\usb", Choose "atmel_usb_dfu.inf",
     Open, OK, Select "AT32UC3A3", Next, Next, Finish

7  - Reboot (may or may not be necessary)

8  - Plug in Audio Widget USB Device. 

9  - Hold Prog, click and release Rest, release Prog. The first time you do this
     Windows will do a fair bit of driver housekeeping.

10 - You may or may not receive messages about lacking drivers for "DG8SAQ-I2C".
     Please ignore them if they show up.

11 - Windows should now be able to find the boot loader in the MCU. To verify:
     Start, Run, devmgmt.msc, look for Atmel USB Devices

12 - To program the Audio Widget use the Flip installation and the 
     Add_to_flip345_bin.zip package. Things may take extra time and require up
	 to five attempts the first time: Start, Run, cmd.exe, 
     cd "C:\Program Files (x86)\Atmel\Flip 3.4.7\bin", "prog widget.elf" 
     Substitute "widget.elf" with the compiled firmware file you wish to to use
     in your Audio Widget. This is the file from step 5. (If you saved the .elf
     file to C:\foo\widget.elf, use "prog C:\foo\widget.elf" instead.)

13 - Expect an output like this in your cmd window:

--------------------------------------------------------------------------------
C:\Program Files (x86)\Atmel\Flip 3.4.7\bin>batchisp -device at32uc3a3256 -hardw
are usb -operation erase f memory flash blankcheck loadbuffer widget.elf program
 verify start reset 0
Running batchisp 1.2.5 on Tue Jul 31 19:44:26 2012

AT32UC3A3256 - USB - USB/DFU

Device selection....................... PASS
Hardware selection..................... PASS
Opening port........................... PASS
Reading Bootloader version............. PASS    1.0.3
Erasing................................ PASS
Selecting FLASH........................ PASS
Blank checking......................... PASS    0x00000 0x3ffff
Parsing ELF file....................... PASS    widget.elf
WARNING: The user program and the bootloader overlap!
Programming memory..................... PASS    0x00000 0x1c43f
Verifying memory....................... PASS    0x00000 0x1c43f
Starting Application................... PASS    RESET   0

Summary:  Total 11   Passed 11   Failed 0

C:\Program Files (x86)\Atmel\Flip 3.4.7\bin>
--------------------------------------------------------------------------------

Debugging Atmel Flip and batchisp. Unfortunately, batchisp isn't always 100% 
stable. If you are encountering problems with it, please try one or more of the
following methods:

0 -  Make notes and screenshots of all the steps you apply in order to install 
     the firmware. You will need this log if you need to contact other Audio 
     Widget owners or Atmel support.

1 -  Instead of prog.bat, try prog_noerase.bat from the Add_to_flip345_bin.zip
     file. prog_noerase.bat comes without the "erase f" part of the command. You
     may also try omitting other parts of the command in prog.bat. 
     
2 -  Try the Linux method for firmware installation. See Installing new firmware 
     - Linux above.
     
3 -  Instead of pressing Prog, clicking Reset and releasing Prog to enter the
     programming bootloader (DFU), try the following: Unplug all cables leading
     to the Audio Widget. Wait. (10s-30min, waiting time isn't yet determined.)
     Press Prog while reconnecting the USB cable. Finally, release Prog and 
     execute prog.bat or prog_noerase.bat
     
4 -  If all of this fails, contact the Audio Widget mailing list and/or Atmel
     support with your detailed notes.



Compiling new firmware
======================

The firmware source code is common for SDR Widget and Audio Widget. It is all
stored in github:
  https://github.com/amontefusco/sdr-widget

See Appendix 1 - git primer below for information about using git. Its text is
taken from the SDR Widget wiki entry:
  http://code.google.com/p/sdr-widget/wiki/gitRepository

The audio-widget branch is for stable features. The audio-widget-experimental
branch is for - you guessed it - code which hasn't made it to the stable branch 
yet.

Firmware source code is written in C. Feel free to open it in your favourite 
editor. The project is not bound to a particular editing environment. It is 
built from the command line. Some people prefer Atmel's AVR32Studio tools. The
old 2.6 version can be downloaded from:
  http://www.atmel.com/tools/STUDIOARCHIVE.aspx 

To build in Windows you need to download and install Cygwin. Your Cygwin 
installation must include packages like make, gcc, git and ssh...

The source code tree includes modified parts of Atmel's AVR32 Software 
Framework. This means you should NOT install and link to that. However, you will
need the Atmel AVR32 toolchain. The AVR32 toolchain may be installed stand-alone
or as part of some kind of AVR(32) Studio package. 

Beware that Atmel's AVR32 toolchain and tools are frequently updated and 
replaced. When you download it, make sure you keep your install files. Upgrading
to a new AVR32 toolchain can be a bit risky, and you should take care when 
installing or changing it. For both Linux and Windows (Cygwin) the project root
Makefile will call the script make-widget. make-widget will look at known 
locations for the AVR32BIN directory which is part of the AVR32 toolchain. At
the time of writing,the AVR32 toolchain can be downloaded from (search for 
"toolchain"):
  http://www.atmel.com/products/microcontrollers/avr/default.aspx?tab=tools

With the default installation, the following text was appended to make-widget
search list:
  "/cygdrive/c/Program Files (x86)\Atmel\AVR Tools\AVR Toolchain\bin" \

The project root Makefile includes compile-time defaults. 

If you are new to the firmware you should spend some time reading up on it. The
code is quite complex, and you may expect a steep learning curve. Then again, it
is not impossible to get started and write usefull code!

The code is split into several images. That division is the first part you 
should look into. It is recommended to use an editor which is able to locate
function and #define definitions.



Modding the hardware
====================

Analog boards in the AB-1.* series has been designed for easy analog modding. 
The USB-I2S module has been designed with a lot of IOs in order to work without
a need for modding. The analog board work perfectly well without modding but you
are more than welcome to change it. 

Please beware that the AB-1.* series uses a 2.0mm pitch interface between 
USB-I2S module and analog board. The module uses female receptacles while the 
analog board has male pins. 

The timing reference of the AB-1.2 is provided by two high-quality oscillators
located right next to the DAC IC. In the most common configuration, the USB-I2S
module is the timing slave.

A note on clocking the AB-1.2 and its predecessors: The MCU on the USB-I2S
module takes an MCLK as input and reuses it 1:1 as bit clock in its serial
interface output (configured as I2S). The maximum frequency of this clock is
16MHz, so for audio rates we use 11.2896 and 12.288MHz. The crystal oscillators
on AB-1.2 are 22.5792 and 24.576MHz. They are first multiplexed into the DAC by
enabling only one XO at a time. The two XO outputs are shorted through 
resistors. In paralell with the DAC chip sits a 2:1 divider which generates the
MCLK for the MCU. Higher division rates are available, a 4:1 division will
support 45.1584 and 49.152MHz XOs. Please see the schematics on how to change
the division rates. It can be done quite easily by replacing 0603 resistors.

The AB-1.2 sends the same 11.2896 or 12.288MHz MCLK to two USB-I2S module input
pins. The USB-I2S module then performs another multiplexing. This way it is
compatible with analog boards which generate both of these frequencies at the 
same time. The MCU's output signal MCLK_P48_N441 is '1' when 12.288MHz is needed
and '0' when 11.2896MHz is needed. 

Another configuration exists where the USB-I2S module itself features 22.5792 
and 24.576MHz oscillators together with a 2:1 divider. This configuration will,
depending on the MCLK_P48_N441 signal, output either 22.5792 or 24.576MHz toward
the DAC, and, respectively, 11.2896 or 12.288MHz toward the MCU. This 
configuration is not used with the AB-1.2. It is only recommended when the 
USB-I2S module is used with an analog board which is not capable of providing 
the 11.2896 and 12.288MHz clocks needed by the ordinary USB-I2S module without
oscillators. The crystal oscaillator power supply on the AB-1.2 is a better
design than the space-limited oscillator power supply on the special version of
the USB-I2S module with XOs.

USB-I2S module and AB-1.2 schematics can be found at:
  http://www.qnktc.com/ab_12.php



Modding Windows drivers
=======================

Generic Windows drivers are high on the wishlist of the project. Any help to 
reach this goal is greatly appreciated. To help debug the driver do as follows:
- Rename asiouac2.dll to asiouac2release.dll
- Rename asiouac2debug.dll to asiouac2.dll
- Download and run DebugView from
  http://technet.microsoft.com/en-us/sysinternals/bb896647.aspx
- Play on any player through ASIO and view driver message on DebugView window

The Windows ASIO driver source code is found at:
  https://github.com/nikkov/Win-Widget

In order to build it you will need the 2008 vintage of Microsoft's Visual C# and
C++ Express tools. 

In order to use the ASIO parts of code you must download closed-source ASIO 
libraries from Steinberg and copy them manually into the code hierarchy. Several
project members have signed the Steinberg developer's agreement. Go to:
  http://www.steinberg.net/nc/en/company/developer/sdk_download_portal.html

The Windows ASIO drivers use libusbK. Like the ASIO files the source code only
contains a place holder for libusbK. You will have to download and install it 
yourself. Relevant libusbK sites are:
  http://sourceforge.net/projects/libusbk/
  http://libusbk.sourceforge.net/UsbK3/index.html

The Windows drivers are bundled into an install package. Here is how to compile
such a package. 

1 -  Download and install 7Zip from: 
     http://www.7-zip.org/

2 -  Add the 7Zip binary folder to the Path variable. 
     Most likely: C:\Program Files (x86)\7-Zip\
     Control Panel -> Search:"Environment Variables"

3 -  You may have to reboot for good measure

4 -  Go to DriverPackages folder and copy your updated Audio-Widget.inf here.
     Run "unix2dos re-pack-files.cmd" in Cygwin. Alternative line ends may crash
     the .cmd file. Touch InstallDriver.exe. Run "re-pack-files.cmd" in cmd. 
     This should generate InstallDriver.exe

5 -  Go to the Installer folder. Copy the following files into it:
     asiouac2.dll,
     asiouac2debug.dll (renamed "Release with Trace" version of asiouac2.dll)
     InstallDriver.exe
     WidgetControl.exe
     WidgetTest.exe
     libusbK.dll (copy of libusbK-dev-kit\bin\dll\x86\libusbK.dll)

6 -  Copy the following files from DriverPackages to Installer/usb_driver: 
     folderss amd64 and x86 with their contents
     Audio-Widget.inf

7 -  Open audiowidget.iss in Inno Setup Compiler and build it.

8 -  Add a date stamp to the generated output



Appendix 1 - git primer
=======================

Git is a version control system. It is very practial for distributed online 
projects like the SDR-Widget / Audio-Widget. But it IS a learning curve which 
shouldn't be underestimated. So read up on tutorials to get a feeling for it. 
For me the one which gave me the sort of aha feeling I needed to trust git as a
tool was this one: 
  http://ftp.newartisans.com/pub/git.from.bottom.up.pdf

This text is taken mainly from the SDR Widget wiki entry:
  http://code.google.com/p/sdr-widget/wiki/gitRepository

You will need to use git in order to download the source code for firmware and
Windows drivers. This section contains a git introduction which was up-to-date
at the time of writing. If you are new to git you MUST read up on recent 
documentation starting at 
  http://www.github.com

The git repository is located at 
  https://github.com/amontefusco/sdr-widget 
You can view branches and their files from there. 

Perhaps the most confusing thing about using git and github (the central 
repository) is all the setting up you have to do to get there. Things are fairly
well documented, but be prepared to follow this list religiously and maybe redo
the whole thing. 

First of all, get yourself a working installation of cygwin or Linux. Packages 
"git", "ssh" and "make" will be required.

Github's own tutorial is focused on a GUI. Still, it is very instructive. Read 
it at 
  http://help.github.com/win-set-up-git/ 

In a shell the setup goes something like this:

- Register with a user name and password at https://github.com/ Remember your 
  credentials as your_github_username and your_github_password

- If you need write acces, request priviledges from repository owner Andrea. 
  By this time you're part of the audio-widget mailing list and know the 
  archive and how to locate him. He'll need to know your_github_username

- You'll need to make SSH keys with
  ssh-keygen -t rsa -C "your@mail.address.com"

- Make backups of id_rsa.pub and id_rsa. Personally, I keep my local project 
  files on a memory stick which tours different computers. My user account
  on all those computers need those same id_rsa files. Their default location is
  /home/username/.ssh That's where you want copies of your backup files to end 
  up if you're using other computers. 

- Go to www.github.com -> Login -> Account Settings (top right center icon)
  -> SSH Keys -> Add SSH Key -> Paste in id_rsa.pub 

- In your shell do:
  git config --global user.name "Your Real Name"
  git config --global user.email "your@mail.address.com"
  git config --global user.password "yourpassword"
  Above line may not be required
  git config --global github.user "your_github_username"
  git config --global github.password "your_github_password"
  ssh -T git@github.com
  Are you sure .... -> yes
  Should produce 
  Hi your_github_username!....

To copy the source code you clone it:
  git clone git@github.com:amontefusco/sdr-widget.git

This will set up local copies of what is stored in the central repository. By 
now you should have a bit of a grasp about git's consept of branches. 
If you don't, go back to the tutorials and reread them. 

It is important that your work is done in your own locally stored scratchbook /
sandbox etc. That gives you the ability to change and discard things without too
large consequences. To get started, first check out the centrally stored branch
you wish to use as a starting point:
  git checkout audio-widget-experimental

Then - and only then - establish your local branch. It is very important that 
the branch name you choose is NOT the name of a centrally stored branch. You 
reach those with git checkout. "git branch" is like "ls" or "dir". "git branch 
xxx" is something you don't do without really wanting to establish the branch 
xxx. You establish the new local branch with:
  git branch local-branch

Start working on your local branch:
  git checkout local-branch 

You should now be set to start editing or browsing the code.

To get recent updates from other users execute:
  git checkout audio-widget-experimental (or other centrally stored branch)
  git pull

Then make sure you're working on your local branch with:
  git checkout local-branch

Then merge the currently checked-out (your local-branch) branch with the one 
you just pulled below (audio-widget-experimental). In this 
case the changes other users committed into the centralized branch 
audio-widget-experimental will be merged into your local-branch. 
  git merge audio-widget-experimental

At this point in time you can open up your favourite editor. Make sure you've 
done all the checkout'ing first. Otherwise the editor might get confused about 
the files which changed on your file system. 

Edit, compile, test, be marry. To compile your code you will need a toolchain 
from Atmel. See Compiling new firmware above. Building the code takes a command
like "make clean audio-widget". But to understand things, have a look at files 
"Makefile" and "make-widget" at the project root directory. 

At some point of time you may be happy with the changes and want to share them.
To do that, first make sure your local-branch is up to date. 
  git commit -am "Text describing your recent edits"

The 'a' in -am makes sure your changes are added. The 'm' part means the 
message follows. This is to prevent git from trying to open an editor to 
record the message. 

Now, close your editors because you're about to change to a branch to be used 
to write back to the repository. In this branch files may look a bit different 
before your changes are merged into it.
  git checkout audio-widget-experimental

This is your local copy of the repository-stored branch. To put your recent 
edits into this local copy, do 
  git merge local-branch

Rembember, the specified branch is merged into the current branch. The last step
remaining is to update the central repository with your changes. 
  git push

If this went well, your changes are in the central repository. And other users 
can pull them from there. For good measure, go back to your local-branch so that
any changes you do will end up there. 



Appendix 2 - UAC2 Feedback Mechanism
====================================

The audio-widget (and sister project sdr-widget) has demonstrated that the 
AT32UC3A3 can support:

1 -  Asynchronous playback with rate feedback under uac1 and uac2;

2 -  The limitation of EP's having different numbers (not allowing two EP's with
     the same EP number, one IN and one OUT) is not an issue when specifying the 
     playback explicit EP. Under uac1 you actually specify the feedback EP by 
     EP_BSYNC_ADDRESS field. Under uac2, as long as the feedback EP follows the 
     OUT EP immediately in the descriptors they will be associated (by OSX, 
     Linux, and Win uac2 drivers):

     S_usb_as_interface_descriptor    spk_as_alt1;
     S_usb_as_g_interface_descriptor_2    spk_g_as;
     S_usb_format_type_2    spk_format_type;
     S_usb_endpoint_audio_descriptor_2 ep2;  // this is the OUT EP
     S_usb_endpoint_audio_specific_2    ep2_s;
     S_usb_endpoint_audio_descriptor_2 ep3;  // this is the explicit feedback EP

3 -  Simultaneous stereo capture and playback can be supported by the SSC.

I have found that it is very useful to refer to the source code of the Linux 
uac2 driver and Apple's USBAudio driver when there are doubts about how the 
uac2 specs is interpreted. There are areas where the specs are not entirely 
clear and you have to experiment with the actual drivers to find out what works
and what doesn't.

For example, under Windows uac1, you have to provide rate feedback values that
the driver expects. The delta between one feedback rate and the next feedback 
rate has to exceed a certain minimum - otherwise it will be ignored completely.
Under Linux uac2, it has automatic rate feedback format detection and the way it
changes sampling rate is different from OSX:

The difference is just that OSX does:
1 -  SetAltInterface(0)
2 -  SET_CUR freq
3 -  SetAltInterface(1)
4 -  Start streaming
and between 1 and 4, there's a time gap of ~740ms.

Linux does:
1 -  SetAltInterface(0)
2 -  SetAltInterface(1)
3 -  SET_CUR freq
4 -  Start streaming
and between 1 and 4, there's a time gap of only ~11ms.

Note that Linux sets the alt interface to 1 first before changing the freq, and
starts streaming within a very short time. This caused trouble at the firmware
as the rate feedback is still based on the OLD sampling rate before things
settle down to the new sampling rate. The OSX way of doing things is more
gentlemanly :-) However, the Linux developer thinks his way is the correct way
so we have to deal with this quirk :-)

