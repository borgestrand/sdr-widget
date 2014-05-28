##
## make both the widget
## and the widget-control that matches
## the features in the widget
##
## usage: "make audio-widget" or "make sdr-widget"
## IMPORTANT: run "make clean" if you change compilation defaults or 
## targets!

## Hardware variety selects USB VID/PID and signature fields only
## Available defines:
## -DCOMPILING_FOR_DRIVER_DEVELOPMENT uses "internal lab use only" VID/PID
## -DFEATURE_PRODUCT_SDR_WIDGET uses AUDIO_PRODUCT_ID_1 and _2
## -DFEATURE_PRODUCT_USB9023 uses AUDIO_PRODUCT_ID_3 and _4
## -DFEATURE_PRODUCT_USB5102 uses AUDIO_PRODUCT_ID_5 and _6
## -DFEATURE_PRODUCT_USB8741 uses AUDIO_PRODUCT_ID_7 and _8
## -DFEATURE_PRODUCT_AB1x uses AUDIO_PRODUCT_ID_9 and _10
## -DFEATURE_PRODUCT_AMB uses AUDIO_PRODUCT_ID_13 and _14
##
## Other defines:
## -DUSB_STATE_MACHINE_DEBUG activate audio feedback state machine
##                           debugging on GPIO and UART

## See featurs.h #define FEATURE_VALUE_NAMES for available defaults. 
SDR_WIDGET_DEFAULTS=-DFEATURE_BOARD_DEFAULT=feature_board_widget \
	-DFEATURE_IMAGE_DEFAULT=feature_image_uac2_dg8saq \
	-DFEATURE_IN_DEFAULT=feature_in_normal \
	-DFEATURE_OUT_DEFAULT=feature_out_normal \
	-DFEATURE_ADC_DEFAULT=feature_adc_ak5394a \
	-DFEATURE_DAC_DEFAULT=feature_dac_cs4344 \
	-DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 \
	-DFEATURE_LOG_DEFAULT=feature_log_500ms \
	-DFEATURE_FILTER_DEFAULT=feature_filter_fir \
	-DFEATURE_QUIRK_DEFAULT=feature_quirk_none \
	-DFEATURE_PRODUCT_SDR_WIDGET 

# These defaults are compiled into code, not necessarily forced
# into flash. To force them into flash, reboot with 
# feature_quirk_ptest set in flash, which will lead to flash being
# overwritten with defaults
AUDIO_WIDGET_DEFAULTS=-DFEATURE_BOARD_DEFAULT=feature_board_usbi2s \
	-DFEATURE_IMAGE_DEFAULT=feature_image_uac1_audio \
	-DFEATURE_IN_DEFAULT=feature_in_normal \
	-DFEATURE_OUT_DEFAULT=feature_out_normal \
	-DFEATURE_ADC_DEFAULT=feature_adc_none \
	-DFEATURE_DAC_DEFAULT=feature_dac_generic \
	-DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 \
	-DFEATURE_LOG_DEFAULT=feature_log_500ms \
	-DFEATURE_FILTER_DEFAULT=feature_filter_fir \
	-DFEATURE_QUIRK_DEFAULT=feature_quirk_none \
	-DFEATURE_PRODUCT_AMB

## Boot up with this code, reboot with feature_quirk_ptest set
## in flash (for good measure). That will execute the production 
## test with these defaults. Then reboot with feature_quirk_none
## and flash will not be overwritten with compiled-in defaults
## at next reboot.
PROD_TEST_DEFAULTS=-DFEATURE_BOARD_DEFAULT=feature_board_usbi2s \
	-DFEATURE_IMAGE_DEFAULT=feature_image_uac1_audio \
	-DFEATURE_IN_DEFAULT=feature_in_normal \
	-DFEATURE_OUT_DEFAULT=feature_out_normal \
	-DFEATURE_ADC_DEFAULT=feature_adc_none \
	-DFEATURE_DAC_DEFAULT=feature_dac_generic \
	-DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 \
	-DFEATURE_LOG_DEFAULT=feature_log_500ms \
	-DFEATURE_FILTER_DEFAULT=feature_filter_fir \
	-DFEATURE_QUIRK_DEFAULT=feature_quirk_ptest \
	-DFEATURE_PRODUCT_AMB

all:: Release/widget.elf widget-control

Release/widget.elf::
	rm -f Release/widget.elf Release/src/features.o
	./make-widget

prod-test::
	rm -f Release/widget.elf Release/src/features.o
	CFLAGS="$(PROD_TEST_DEFAULTS)" ./make-widget

audio-widget::
	rm -f Release/widget.elf Release/src/features.o
	CFLAGS="$(AUDIO_WIDGET_DEFAULTS)" ./make-widget

sdr-widget::
	rm -f Release/widget.elf Release/src/features.o
	CFLAGS="$(SDR_WIDGET_DEFAULTS)" ./make-widget

widget-control: widget-control.c src/features.h
	gcc $(AUDIO_WIDGET_DEFAULTS) -o widget-control widget-control.c -lusb-1.0

clean::
	rm -f widget-control widget-control.exe
	cd Release && make clean
	rm -f widget-control
