
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
#
# -DUSB_REDUCED_DEBUG			Reduce debug and use UART only for CPU communication



# PARTNAME=-mpart=uc3a3256
# Use "prog.bat" or RATHER "prog256.bat"
# NB: the use of "prog128.bat" may brick the processor!!

PARTNAME=-mpart=uc3a3128
# This HW target seems to work on uc3a3256 hardware as well
# Use "prog128.bat"
# NB: the use of "prog256.bat" or "prog.bat" will positively brick a '128 processor!!


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
	-DVDD_SENSE \
	-DUSB_STATE_MACHINE_GPIO \
	-DFEATURE_HID \
	\
	-DFEATURE_PRODUCT_BOEC1 \
	-DUSB_REDUCED_DEBUG \
	-DFEATURE_ALT2_16BIT \
	-DHW_GEN_SPRX \
	-DHW_GEN_SPRX_PATCH_01 \
	-DHW_GEN_SPRX_PATCH_02

# Floormotion after "\"
#	-DFEATURE_PRODUCT_FMADC \
#	-DFEATURE_ADC_EXPERIMENTAL \
#	-DHW_GEN_FMADC

# Henry Audio USB DAC 128 / QNKTC after "\" 
#	-DFEATURE_PRODUCT_AB1x \
#	-DFEATURE_ALT2_16BIT \
#	-DHW_GEN_AB1X \
#	-DFEATURE_VOLUME_CTRL

# Henry Audio USB DAC 256 (SPRX) after "\" 
#	-DFEATURE_PRODUCT_HA256 \
#	-DFEATURE_ALT2_16BIT \
#	-DHW_GEN_SPRX \
#	-DHW_GEN_SPRX_PATCH_01 \
#	-DHW_GEN_SPRX_PATCH_02 \
#	-DFEATURE_VOLUME_CTRL 

# Boenicke (SPRX) after "\" NB: check register 0x08 / R8 of WM8804 to enable regenerated MCLK when that is needed
#	-DFEATURE_PRODUCT_BOEC1 \
#	-DUSB_REDUCED_DEBUG \
#	-DFEATURE_ALT2_16BIT \
#	-DHW_GEN_SPRX \
#	-DHW_GEN_SPRX_PATCH_01 \
#	-DHW_GEN_SPRX_PATCH_02
	

audio-widget::
	rm -f Release/widget.elf Release/src/features.o
	CFLAGS="$(AUDIO_WIDGET_DEFAULTS)" ASFLAGS="$(PARTNAME)" ./make-widget

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

