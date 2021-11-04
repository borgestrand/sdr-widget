
# These defaults are compiled into code, not necessarily forced
# into flash. To force them into flash, reboot with 
# feature_quirk_ptest set in flash, which will lead to flash being
# overwritten with defaults

<<<<<<< HEAD
# The target part '256 for Henry Audio USB DAC series
# -mpart=uc3a3256
# -mpart=uc3a3128

PARTNAME=-mpart=uc3a3256

AUDIO_WIDGET_DEFAULTS=$(PARTNAME) \
=======
PARTNAME=-mpart=uc3a3256

AUDIO_WIDGET_DEFAULTS=$(PARTNAME)\
>>>>>>> before-mpart
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
	-DFEATURE_PRODUCT_AB1x

<<<<<<< HEAD
# Choose wisely:
#   -DFEATURE_PRODUCT_AMB
#   -DFEATURE_PRODUCT_MADA
#	-DFEATURE_PRODUCT_AB1x \
#	-DFEATURE_VOLUME_CTRL \
#	-DHW_GEN_AB1X \
# 	-DHW_GEN_DIN10 \
# 	-DHW_GEN_DIN20 \
#	-DHW_GEN_RXMOD \
#	-DFEATURE_CLOCK_SELECTOR \ - Build UAC2 with clock selector
#	-DUSB_STATE_MACHINE_DEBUG \ - Used for verbose RS232 debugging
#	-DUSB_STATE_MACHINE_GPIO \  - Used for 'scope debugging of state machine timing
#	-DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 \
#	-DFEATURE_CFG_INTERFACE \						Enable the configuration interface at Endpoint 0. Disabling breaks UAC1
#	-DFEATURE_HID \



audio-widget::
	rm -f Release/widget.elf Release/src/features.o
	CFLAGS="$(AUDIO_WIDGET_DEFAULTS)" ./make-widget
=======

audio-widget::
	rm -f Release/widget.elf Release/src/features.o
	CFLAGS="$(AUDIO_WIDGET_DEFAULTS)" ASFLAGS="$(PARTNAME)" ./make-widget
>>>>>>> before-mpart

clean::
	rm -f widget-control widget-control.exe
	cd Release && make clean
	rm -f widget-control
