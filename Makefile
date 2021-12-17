
# These defaults are compiled into code, not necessarily forced
# into flash. To force them into flash, reboot with 
# feature_quirk_ptest set in flash, which will lead to flash being
# overwritten with defaults
#
# -DFEATURE_PRODUCT_AB1x

# PARTNAME=-mpart=uc3a3256
PARTNAME=-mpart=uc3a3128

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
	-DFEATURE_VOLUME_CTRL \
	-DHW_GEN_RXMOD \
	-DFEATURE_PRODUCT_BOEC1

audio-widget::
	rm -f Release/widget.elf Release/src/features.o
	CFLAGS="$(AUDIO_WIDGET_DEFAULTS)" ASFLAGS="$(PARTNAME)" ./make-widget

clean::
	rm -f widget-control widget-control.exe
	cd Release && make clean
	rm -f widget-control
