##
## make both the widget
## and the widget-control that matches
## the features in the widget
##
## usage: "make audio-widget" or "make sdr-widget"
## IMPORTANT: run "make clean" if you change compilation defaults or 
## targets!

SDR_WIDGET_DEFAULTS=-DFEATURE_BOARD_DEFAULT=feature_board_widget \
	-DFEATURE_IMAGE_DEFAULT=feature_image_uac2_dg8saq \
	-DFEATURE_IN_DEFAULT=feature_in_normal \
	-DFEATURE_OUT_DEFAULT=feature_out_normal \
	-DFEATURE_ADC_DEFAULT=feature_adc_ak5394a \
	-DFEATURE_DAC_DEFAULT=feature_dac_cs4344 \
	-DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 \
	-DFEATURE_LOG_DEFAULT=feature_log_500ms \
	-DFEATURE_FILTER_DEFAULT=feature_filter_fir

AUDIO_WIDGET_DEFAULTS=-DFEATURE_BOARD_DEFAULT=feature_board_usbi2s \
	-DFEATURE_IMAGE_DEFAULT=feature_image_uac1_audio \
	-DFEATURE_IN_DEFAULT=feature_in_normal \
	-DFEATURE_OUT_DEFAULT=feature_out_normal \
	-DFEATURE_ADC_DEFAULT=feature_adc_none \
	-DFEATURE_DAC_DEFAULT=feature_dac_es9023 \
	-DFEATURE_LCD_DEFAULT=feature_lcd_hd44780 \
	-DFEATURE_LOG_DEFAULT=feature_log_500ms \
	-DFEATURE_FILTER_DEFAULT=feature_filter_fir

all:: Release/widget.elf widget-control

Release/widget.elf::
	rm -f Release/widget.elf Release/src/features.o
	./make-widget

audio-widget::
	rm -f Release/widget.elf Release/src/features.o
	CFLAGS="$(AUDIO_WIDGET_DEFAULTS)" ./make-widget

sdr-widget::
	rm -f Release/widget.elf Release/src/features.o
	CFLAGS="$(SDR_WIDGET_DEFAULTS)" ./make-widget

widget-control: widget-control.c src/features.h
	gcc -o widget-control widget-control.c -lusb-1.0

clean::
	cd Release && make clean
	rm -f widget-control
