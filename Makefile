
# These defaults are compiled into code, not necessarily forced
# into flash. To force them into flash, reboot with 
# feature_quirk_ptest set in flash, which will lead to flash being
# overwritten with defaults
#
# -DFEATURE_PRODUCT_AB1x		Henry Audio USB DAC 128 and QNKTC series of DACs
# -DFEATURE_PRODUCT_HA256		Henry Audio experimental product
# -DFEATURE_PRODUCT_BOEC1		Boenicke experimental product
#
# -DHW_GEN_AB1X					Pure USB DAC - all hardware revisions
# -DHW_GEN_RXMOD				Latest revision of SPDIF receiver
# -DHW_GEN_RXMOD_PATCH_01		Hardware development, simplification of RXmod_t1_A nr. 1 and 2. Possibly applicable to RXmod_T1_C boards as well
# -DHW_GEN_RXMOD_PATCH_02		Hardware development, RXmod_T1_C boards
#
# -DFEATURE_HID					USB HID functions AND debug development system - FIX: split them in two... 
# -DFEATURE_VOLUME_CTRL			Software volume control in DAC
# -DFEATURE_CLOCK_SELECTOR		Clock selector, don't enable unless you wish to experiment with different clocks for ADC and DAC
# -DFEATURE_ALT2_16BIT			UAC2 has both ALT1 (24 bit) and ALT2 (16 bit). Applies to both ADC and DAC code in UAC2. Does NOT! apply to UAC1
#
# -DFEATURE_ADC_EXPERIMENTAL	Experimental ADC support


# PARTNAME=-mpart=uc3a3256
# Use "prog.bat" or RATHER "prog256.bat"
# NB: the use of "prog128.bat" may brick the processor!!

PARTNAME=-mpart=uc3a3128
# This HW target seems to work on uc3a3256 hardware as well
# Use "prog128.bat"
# NB: the use of "prog256.bat" or "prog.bat" will positively brick a '128 processor!!

# Serial number hard-coded on line 454 of usb_descriptors.h

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
	-DUSB_STATE_MACHINE_DEBUG \
	-DFEATURE_HID \
	-DFEATURE_VOLUME_CTRL \
	-DFEATURE_PRODUCT_HA256 \
	-DHW_GEN_RXMOD \
	-DHW_GEN_RXMOD_PATCH_01 \
	-DHW_GEN_RXMOD_PATCH_02 \
	-DFEATURE_ADC_EXPERIMENTAL
	
#	 \
#	-DFEATURE_ALT2_16BIT
	

audio-widget::
	rm -f Release/widget.elf Release/src/features.o
	CFLAGS="$(AUDIO_WIDGET_DEFAULTS)" ASFLAGS="$(PARTNAME)" ./make-widget

clean::
	rm -f widget-control widget-control.exe
	cd Release && make clean
	rm -f widget-control
