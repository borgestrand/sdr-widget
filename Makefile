
# These defaults are compiled into code, not necessarily forced
# into flash. To force them into flash, reboot with 
# feature_quirk_ptest set in flash, which will lead to flash being
# overwritten with defaults
#
# -DFEATURE_PRODUCT_AB1x		Henry Audio USB DAC 128 and QNKTC series of DACs
# -DFEATURE_PRODUCT_HA256		Henry Audio experimental product
# -DFEATURE_PRODUCT_BOEC1		Boenicke experimental product
# -DFEATURE_PRODUCT_WFADC		Whisperfloor data collection device
#
# -DHW_GEN_AB1X					Pure USB DAC - all marketed Henry Audio / QNKTC hardware versions
# -DHW_GEN_WFADC				Whisperfloor ADC data collection based on old USB module
# -DHW_GEN_SPRX					Latest revision of SPDIF receiver
# -DHW_GEN_SPRX_PATCH_01		Hardware development, simplification of RXmod_t1_A nr. 1 and 2. Possibly applicable to RXmod_T1_C boards as well
# -DHW_GEN_SPRX_PATCH_02		Hardware development, RXmod_T1_C boards
#
# -DFEATURE_HID					USB HID functions AND debug development system - FIX: split them in two... 
# -DFEATURE_VOLUME_CTRL			Software volume control in DAC
# -DFEATURE_CLOCK_SELECTOR		Clock selector, don't enable unless you wish to experiment with different clocks for ADC and DAC
# -DFEATURE_ALT2_16BIT			UAC2 has both ALT1 (24 bit) and ALT2 (16 bit). Applies to both ADC and DAC code in UAC2. Does NOT! apply to UAC1
#
# -DFEATURE_ADC_EXPERIMENTAL	Experimental ADC support
# -DFEATURE_SPDIF_CMD			Experimental disable state machine in SPDIF receiver code - only makes sense in SPRX context
#
# -DUSB_REDUCED_DEBUG			Reduce debug and use UART only for CPU communication
# -DFEATURE_UNINVERT_LRCK		Reverse the effect of an inverted LRCK signal on the I2S output
# -DBOOT_SIGNAL_LED				Use front LED to indicate reboot
# -DLOOSE_SIGNAL_LED			Use front LED to indicate SPDIF signal loss
# -DI2S_METADATA				Send 8 bits of metadata below each 24-bit audio data sample
# -DI2S_POLARITY_CHECK			Output SDATA in phase with LRCK for debugging. It will override -DI2S_METADATA for paused content

# -DSECONDARY_DAC				Experimenting with two DAC chips on one DAC board
# -DINDICATE_EXTREME_SI			Indication of extreme skip/insert events

# PARTNAME=-mpart=uc3a3256
# Use "prog128.bat". The command "prog.bat" has been updated to tell you it is obsolete
# NB: the use of "prog256.bat" or the initial versions of "prog.bat" will brick an AT32UC3A3128 processor!!

PARTNAME=-mpart=uc3a3128
# This HW target seems to work on uc3a3256 hardware as well
# Use "prog128.bat"
# NB: the use of "prog256.bat" or "prog.bat" will positively brick a '128 processor!!


# 20260918: build date/time and git commit, injected as bare (unquoted-safe) tokens - see
# BUILD_STR()/BUILD_INFO_STRING in device_mouse_hid_task.c for how these become a boot-time
# string. Deliberately NOT passed as a quoted string here: $(CFLAGS) is spliced unquoted into
# the compile recipe (Release/src/subdir.mk), so an embedded "..." would get stripped by that
# second shell pass rather than reaching the compiler - bare alphanumeric tokens have no such
# problem and the C preprocessor's stringification operator builds the quoted literal instead.
BUILD_DATE := $(shell date '+%Y%m%d')
BUILD_TIME := $(shell date '+%H%M')
BUILD_TZ := $(shell date '+%Z')
BUILD_COMMIT := $(shell git rev-parse --short HEAD 2>/dev/null || echo unknown)
BUILD_DIRTY := $(shell git diff --quiet 2>/dev/null && git diff --cached --quiet 2>/dev/null && echo clean || echo dirty)

BUILD_INFO_DEFS = -DBUILD_DATE=$(BUILD_DATE) -DBUILD_TIME=$(BUILD_TIME) -DBUILD_TZ=$(BUILD_TZ) -DBUILD_COMMIT=$(BUILD_COMMIT) -DBUILD_DIRTY=$(BUILD_DIRTY)

AUDIO_WIDGET_DEFAULTS=$(PARTNAME)\
	-DFEATURE_BOARD_DEFAULT=feature_board_usbi2s \
	-DFEATURE_IMAGE_DEFAULT=feature_image_uac2_audio \
	-DFEATURE_IN_DEFAULT=feature_in_normal \
	-DFEATURE_OUT_DEFAULT=feature_out_normal \
	-DFEATURE_ADC_DEFAULT=feature_adc_none \
	-DFEATURE_DAC_DEFAULT=feature_dac_generic \
	-DFEATURE_LCD_DEFAULT=feature_lcd_none \
	-DFEATURE_LOG_DEFAULT=feature_log_none \
	-DFEATURE_FILTER_DEFAULT=feature_filter_fir \
	-DFEATURE_QUIRK_DEFAULT=feature_quirk_none \
	-DUSB_STATE_MACHINE_GPIO \
	-DFEATURE_HID \
	$(BUILD_INFO_DEFS) \
	\
	-DFEATURE_PRODUCT_HA256 \
	-DFEATURE_ALT2_16BIT \
	-DFEATURE_SPDIF_CMD \
	-DHW_GEN_SPRX
	

# Floormotion after "\"
#	-DFEATURE_PRODUCT_FMADC \
#	-DFEATURE_ADC_EXPERIMENTAL \
#	-DHW_GEN_FMADC

# Henry Audio USB DAC 128 / QNKTC after "\" 
#	-DFEATURE_PRODUCT_AB1x \
#	-DFEATURE_ALT2_16BIT \
#	-DHW_GEN_AB1X \
#	-DFEATURE_VOLUME_CTRL
# 20261010: VDD_SENSE (PA19 digital read -> self/bus-powered descriptor) is no longer defined anywhere.
# AB-1.x is bus-powered only. PA19 isn't connected on early HW_GEN_SPRX boards and is the BOM_VER
# ADC input (AD[5]) on newer ones. SPRX sets bus/self-powered per USB plug, see mobo_usb_select()

# Henry Audio USB DA 256 (SPRX) after "\" 
#	-DFEATURE_PRODUCT_HA256 \
#	-DFEATURE_ALT2_16BIT \
#	-DHW_GEN_SPRX

# SPRX debug after "\" 
#	-DFEATURE_PRODUCT_HA256 \
#	-DI2S_POLARITY_CHECK \
# or
#	-DI2S_METADATA \
#	-DFEATURE_ALT2_16BIT \
#	-DHW_GEN_SPRX \
#	-DFEATURE_VOLUME_CTRL \
#	-DBOOT_SIGNAL_LED \
#	-DFEATURE_SPDIF_CMD

# Boenicke (SPRX) after "\" NB: check register 0x08 / R8 of WM8804 to enable regenerated MCLK when that is needed 
#	-DFEATURE_PRODUCT_BOEC1 \
#	-DI2S_POLARITY_CHECK \
# or
#	-DI2S_METADATA \
#	-DUSB_REDUCED_DEBUG \
#	-DFEATURE_ALT2_16BIT \
#	-DHW_GEN_SPRX
	

audio-widget::
	rm -f Release/widget.elf Release/src/features.o
	CFLAGS="$(AUDIO_WIDGET_DEFAULTS)" ASFLAGS="$(PARTNAME)" ./make-widget

# 20260915: 84MHz CPU clock bring-up for 384kHz UAC2 work. Same defaults as audio-widget,
# plus FEATURE_84MHz. Source-verified, NOT yet bench-verified - see checklist.
audio-widget-84mhz::
	rm -f Release/widget.elf Release/src/features.o
	CFLAGS="$(AUDIO_WIDGET_DEFAULTS) -DFEATURE_84MHz" ASFLAGS="$(PARTNAME)" ./make-widget

clean::
	rm -f widget-control widget-control.exe
	cd Release && make clean
	rm -f widget-control


# Things you will find yourself searching for:
# Speedx_hs - sample rate definitions
# mobo_srd - sample rate detector
# AK5394A_pdca_rx_enable - turn on DMA for incoming I2S
# Serial number hard-coded on line 454 of usb_descriptors.h
# 
# Tasks
# - Turn on I2S RX DMA without LR swap or 1-sample delays - both PCM1863 and WM8804 based sources
# - Always start WM8804 playback mid-buffer, avoid use of regenerated clock
# - Improve skip/insert code, see reMarkable notes

