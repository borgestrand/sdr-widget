/** 
* @file spectrum.c
* @brief Spectrum functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
*/

/* Copyright (C) 
* 2009 - John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/

#include <gtk/gtk.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "xvtr.h"
#include "band.h"
#include "bandstack.h"
#include "main.h"
#include "filter.h"
#include "frequency.h"
#include "ozy.h"
#include "property.h"
#include "soundcard.h"
#include "spectrum.h"
#include "spectrum_update.h"
#include "vfo.h"
#include "subrx.h"
#include "iphone.h"

GtkWidget* spectrum;

int spectrumMAX=-100;
int spectrumMIN=-180;
int spectrumSTEP=10;

int spectrumLow=-24000;
int spectrumHigh=+24000;

int spectrumMode;


GdkPixmap *spectrumPixmap=NULL;
GdkPoint* spectrumPoints;
GdkPixbuf *waterfallPixbuf;
guchar* waterfallPixels;
gint rowStride,
     nChannels;

float waterfallHighThreshold=-100.0f;
float waterfallLowThreshold=-150.0f;

int colorLowR=0; // black
int colorLowG=0;
int colorLowB=0;

int colorMidR=255; // red
int colorMidG=0;
int colorMidB=0;

int colorHighR=255; // yellow
int colorHighG=255;
int colorHighB=0;

float* waterfall;

int adcOverflow=0;

gboolean spectrumAverage=1;
int initAverage=1;
float averageSpectrum[SPECTRUM_BUFFER_SIZE];
float spectrumAverageSmoothing=0.4f;
 
gboolean spectrum_configure_event(GtkWidget* widget,GdkEventConfigure* event);
gboolean spectrum_expose_event(GtkWidget* widget,GdkEventExpose* event);
gboolean spectrum_button_press_event(GtkWidget* widget,GdkEventButton* event);
gboolean spectrum_button_release_event(GtkWidget* widget,GdkEventButton* event);
gboolean spectrum_motion_notify_event(GtkWidget* widget,GdkEventMotion* event);
gboolean spectrum_scroll_event(GtkWidget* widget,GdkEventScroll* event);

void plotScope(float* samples);
void drawScope();
void plotSpectrum(float* samples,int height);
void drawSpectrum(int height);
void plotPhase(float* samples);
void drawPhase();
void plotPhase2(float* samples);
void drawPhase2();
void drawWaterfall(int y,int height);

void spectrumUpdateOff();
void setSpectrumMode(int mode);


/* --------------------------------------------------------------------------*/
/** 
* @brief New spectrum display
* 
* @return 
*/
GtkWidget* newSpectrumDisplay() {
    
    spectrumLow=-sampleRate/2;
    spectrumHigh=+sampleRate/2;

    // allocate space for the plotting spectrumPoints
    spectrumPoints=calloc(spectrumWIDTH,sizeof(GdkPoint));
    int i;
    for(i=0;i<spectrumWIDTH;i++) {
        spectrumPoints[i].x=i;
        spectrumPoints[i].y=-1;
    }

    // allocate space for the waterfall
    waterfall=calloc(spectrumWIDTH,sizeof(float));
    waterfallPixbuf=gdk_pixbuf_new(GDK_COLORSPACE_RGB,FALSE,8,spectrumWIDTH,spectrumHEIGHT);
    waterfallPixels=gdk_pixbuf_get_pixels(waterfallPixbuf);
    nChannels=gdk_pixbuf_get_n_channels(waterfallPixbuf);
    rowStride=gdk_pixbuf_get_rowstride(waterfallPixbuf);

    // build the UI
    spectrum=gtk_drawing_area_new();
    gtk_widget_set_size_request(GTK_WIDGET(spectrum),spectrumWIDTH,spectrumHEIGHT);
    gtk_widget_show(spectrum);

    g_signal_connect(G_OBJECT (spectrum),"configure_event",G_CALLBACK(spectrum_configure_event),NULL);
    g_signal_connect(G_OBJECT (spectrum),"expose_event",G_CALLBACK(spectrum_expose_event),NULL);

    g_signal_connect(G_OBJECT(spectrum),"motion_notify_event",G_CALLBACK(spectrum_motion_notify_event),NULL);
    g_signal_connect(G_OBJECT(spectrum),"button_press_event",G_CALLBACK(spectrum_button_press_event),NULL);
    g_signal_connect(G_OBJECT(spectrum),"button_release_event",G_CALLBACK(spectrum_button_release_event),NULL);
    g_signal_connect(G_OBJECT(spectrum),"scroll_event",G_CALLBACK(spectrum_scroll_event),NULL);

    gtk_widget_set_events(spectrum, GDK_EXPOSURE_MASK
                         | GDK_LEAVE_NOTIFY_MASK
                         | GDK_BUTTON_PRESS_MASK
                         | GDK_BUTTON_RELEASE_MASK
                         | GDK_SCROLL_MASK
                         | GDK_POINTER_MOTION_MASK);

    return spectrum;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set spectrum span
* 
* @param span
*/
void setSpectrumSpan(int span) {
    spectrumLow=-span;
    spectrumHigh=span;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Spectrum configure event
* 
* @param widget
* @param event
* 
* @return 
*/
gboolean spectrum_configure_event(GtkWidget* widget,GdkEventConfigure* event) {
    if(spectrumPixmap) g_object_unref(spectrumPixmap);

    spectrumPixmap=gdk_pixmap_new(widget->window,widget->allocation.width,widget->allocation.height,-1);

    gdk_draw_rectangle(spectrumPixmap,
		       widget->style->black_gc,
		       TRUE,
		       0,0,
		       widget->allocation.width,
		       widget->allocation.height);
    return TRUE;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Spectrum expose event 
* 
* @param widget
* @param event
* 
* @return 
*/
gboolean spectrum_expose_event(GtkWidget* widget,GdkEventExpose* event) {
    gdk_draw_drawable(widget->window,
		    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
		    spectrumPixmap,
		    event->area.x, event->area.y,
		    event->area.x, event->area.y,
		    event->area.width, event->area.height);
    return FALSE;
}

gboolean hasMoved;
int firstX;
int lastX;

/* --------------------------------------------------------------------------*/
/** 
* @brief Spectrum button press event
* 
* @param widget
* @param event
* 
* @return 
*/
gboolean spectrum_button_press_event(GtkWidget* widget,GdkEventButton* event) {
    switch(event->button) {
        case 1:
            // left button - start dragging frequency
            firstX=event->x;
            lastX=event->x;
            hasMoved=FALSE;
            break;
        case 2:
            // middle button
            break;
        case 3:
            // right button - click to frequency (to cursor) 
            {
            long increment;
            if(subrx) {
                int subCursor=((subrxFrequency-frequencyA)-spectrumLow)*spectrumWIDTH/(spectrumHigh-spectrumLow);
                increment=-(int)(subCursor+((float)event->x*((float)spectrumHigh-(float)spectrumLow)/(float)spectrumWIDTH));
            } else {
                increment=(int)((float)spectrumLow+((float)event->x*((float)spectrumHigh-(float)spectrumLow)/(float)spectrumWIDTH));
            }
            vfoIncrementFrequency(increment);
            }
            break;
    }
    return TRUE;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Spectrum button released event
* 
* @param widget
* @param event
* 
* @return 
*/
gboolean spectrum_button_release_event(GtkWidget* widget,GdkEventButton* event) {
    switch(event->button) {
        case 1:
            // left button  - click to frequency (centered in filter) if not dragged
            if(!hasMoved) {
                if(!subrx) {
                    long increment;
                    increment=(int)((float)spectrumLow+((float)event->x*((float)spectrumHigh-(float)spectrumLow)/(float)spectrumWIDTH)-((float)filterLow+((float)filterHigh-(float)filterLow)/2.0));
                    vfoIncrementFrequency(increment);
                }
            }
            break;
        case 2:
            // middle button
            break;
        case 3:
            // right button - click to frequency (center mid filter)
            break;
    }
    return TRUE;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Spectrum motion notify event
* 
* @param widget
* @param event
* 
* @return 
*/
gboolean spectrum_motion_notify_event(GtkWidget* widget,GdkEventMotion* event) {
    if(event->state & GDK_BUTTON1_MASK) {
        long f;
        int moved=lastX-event->x;
        f=(long)((float)moved*((float)spectrumHigh-(float)spectrumLow)/(float)spectrumWIDTH);
        if(subrx) f=-f;
        vfoIncrementFrequency(f);
        lastX=event->x;
        hasMoved=TRUE;
    }
    return TRUE;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Spectrum scroll event
* 
* @param widget
* @param event
* 
* @return 
*/
gboolean spectrum_scroll_event(GtkWidget* widget,GdkEventScroll* event) {
    long f;
    if(event->direction==GDK_SCROLL_UP) {
        f=frequencyIncrement;
    } else {
        f=-frequencyIncrement;
    }
    vfoIncrementFrequency(f);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Spectrum update
* 
* @param samples
*/
void updateSpectrum(float* samples) {

    iphone_set_samples(samples);

    spectrumLow=-sampleRate/2;
    spectrumHigh=+sampleRate/2;
    switch(spectrumMode) {
        case spectrumSPECTRUM:
            plotSpectrum(samples,spectrumHEIGHT);
            drawSpectrum(spectrumHEIGHT);
            break;
        case spectrumPANADAPTER:
            plotSpectrum(samples,spectrumHEIGHT);
            drawSpectrum(spectrumHEIGHT);
            break;
        case spectrumSCOPE:
            plotScope(samples);
            drawScope();
            break;
        case spectrumPHASE:
            plotPhase(samples);
            drawPhase();
            break;
        case spectrumPHASE2:
            plotPhase2(samples);
            drawPhase2();
            break;
        case spectrumPANWATER:
            plotSpectrum(samples,spectrumHEIGHT/2);
            drawWaterfall(spectrumHEIGHT/2,spectrumHEIGHT/2);
            drawSpectrum(spectrumHEIGHT/2);
            break;
        case spectrumHISTOGRAM:
            spectrumUpdateOff();
            break;
        case spectrumNONE:
            spectrumUpdateOff();
            break;
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Plot scope
* 
* @param samples
*/
void plotScope(float* samples) {
        int y=0;
        int pixels=0;
        int i;
        for(i=0;i<spectrumWIDTH;i++) {
            pixels=(int)((float)(spectrumHEIGHT/2)*samples[i*2]);
            y = spectrumHEIGHT/2-pixels;
            spectrumPoints[i].x = i;
            spectrumPoints[i].y = y;
        }
}


/* --------------------------------------------------------------------------*/
/** 
* @brief Draw scope
*/
void drawScope() {

    // get the spectrum context - just copy the window GC and modify
    GdkGC* gc;
    if(spectrum->window) {

        gc=gdk_gc_new(spectrum->window);
        gdk_gc_copy(gc,spectrum->style->black_gc);

        gdk_draw_rectangle(spectrumPixmap,gc,TRUE,0,0,spectrumWIDTH,spectrumHEIGHT);

        // draw the grid
        gdk_gc_set_rgb_fg_color(gc,&verticalColor);
        gdk_draw_line(spectrumPixmap,gc,0,spectrumHEIGHT/2,spectrumWIDTH,spectrumHEIGHT/2);
        gdk_gc_set_rgb_fg_color(gc,&horizontalColor);
        gdk_draw_line(spectrumPixmap,gc,spectrumWIDTH/2,0,spectrumWIDTH/2,spectrumHEIGHT);

        // plot the data
        gdk_gc_set_rgb_fg_color(gc,&plotColor);
        gdk_draw_lines(spectrumPixmap,gc,spectrumPoints,spectrumWIDTH);

        // update the spectrum
        gtk_widget_queue_draw(spectrum);

        g_object_unref(gc);
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Draw spectrum
* 
* @param height
*/
void drawSpectrum(int height) {

    // get the spectrum context - just copy the window GC and modify
    GdkGC* gc;
    PangoContext *context;
    PangoLayout *layout;
    char label[80];

    int i;
    long long f;
    long long minDisplay;
    long long maxDisplay;
    long long minFrequency;
    BAND_LIMITS* bandLimits;

    float hzPerPixel;

    int filterLeftX;
    int filterRightX;

    minDisplay=f=frequencyA-(sampleRate/2);
    maxDisplay=frequencyA+(sampleRate/2);
    
    hzPerPixel=(float)sampleRate/(float)spectrumWIDTH;

    if(spectrum->window) {

        gc=gdk_gc_new(spectrum->window);
        gdk_gc_copy(gc,spectrum->style->black_gc);
        gdk_draw_rectangle(spectrumPixmap,gc,TRUE,0,0,spectrumWIDTH,height);

        // setup for drawing text
        context = gdk_pango_context_get_for_screen (gdk_screen_get_default ());
        layout = pango_layout_new (context);
        pango_layout_set_width(layout,spectrumWIDTH*PANGO_SCALE);

        if(subrx) {
            filterLeftX=(filterLow+(subrxFrequency-frequencyA)-spectrumLow)*spectrumWIDTH/(spectrumHigh-spectrumLow);
            filterRightX=(filterHigh+(subrxFrequency-frequencyA)-spectrumLow)*spectrumWIDTH/(spectrumHigh-spectrumLow);
            if(filterLeftX==filterRightX) filterRightX++;
            gdk_gc_set_rgb_fg_color(gc,&subrxFilterColor);
            gdk_draw_rectangle(spectrumPixmap,gc,TRUE,filterLeftX,0,filterRightX-filterLeftX,height);
        }

        // draw the filter 
        filterLeftX=(filterLow-spectrumLow)*spectrumWIDTH/(spectrumHigh-spectrumLow);
        filterRightX=(filterHigh-spectrumLow)*spectrumWIDTH/(spectrumHigh-spectrumLow);
        if(filterLeftX==filterRightX) filterRightX++;
        gdk_gc_set_rgb_fg_color(gc,&filterColor);
        gdk_draw_rectangle(spectrumPixmap,gc,TRUE,filterLeftX,0,filterRightX-filterLeftX,height);

        // draw the vertical lines
        for(i=0;i<spectrumWIDTH;i++) {
            if(f>0) {
                if((f%10000)<(long long)hzPerPixel) {
                    gdk_gc_set_rgb_fg_color(gc,&verticalColor);
                    gdk_draw_line(spectrumPixmap,gc,i,0,i,height);
                    gdk_gc_set_rgb_fg_color(gc,&spectrumTextColor);
                    sprintf(label,"<span font_desc='Sans Regular 8'>%5.3f</span>",(float)f/1000000.0f);
                    pango_layout_set_markup (layout, label, -1);
                    gdk_draw_layout (GDK_DRAWABLE (spectrumPixmap), gc, i-17, 0, layout);
                }
            }
            f+=(long long)hzPerPixel;
        }

        // draw band edges
        bandLimits=getBandLimits(minDisplay,maxDisplay);
        if(bandLimits!=NULL) {
            gdk_gc_set_rgb_fg_color(gc,&red);
            if((minDisplay<bandLimits->minFrequency)&&(maxDisplay>bandLimits->minFrequency)) {
                i=(bandLimits->minFrequency-minDisplay)/(long long)hzPerPixel;
                gdk_draw_line(spectrumPixmap,gc,i-1,0,i-1,height);
                gdk_draw_line(spectrumPixmap,gc,i,0,i,height);
            }
            if((minDisplay<bandLimits->maxFrequency)&&(maxDisplay>bandLimits->maxFrequency)) {
                i=(bandLimits->maxFrequency-minDisplay)/(long long)hzPerPixel;
                gdk_draw_line(spectrumPixmap,gc,i,0,i,height);
                gdk_draw_line(spectrumPixmap,gc,i+1,0,i+1,height);
            }
        }
        
        // draw the cursor
        gdk_gc_set_rgb_fg_color(gc,&red);
        gdk_draw_line(spectrumPixmap,gc,spectrumWIDTH/2,0,spectrumWIDTH/2,height);

        // draw the horizontal lines
        int v=spectrumMAX-spectrumMIN;
        int n=v/spectrumSTEP;
        int p=height/n;
        int i;

        for(i=1;i<n;i++) {
            int val=spectrumMAX-i*spectrumSTEP;
            int y=(spectrumMAX-val)*height/v;
            gdk_gc_set_rgb_fg_color(gc,&horizontalColor);
            gdk_draw_line(spectrumPixmap,gc,0,y,spectrumWIDTH,y);
            gdk_gc_set_rgb_fg_color(gc,&spectrumTextColor);
            sprintf(label,"<span font_desc='Sans Regular 8'>%d</span>",val);
            pango_layout_set_markup (layout, label, -1);
            gdk_draw_layout (GDK_DRAWABLE (spectrumPixmap), gc, 0, y, layout);
        }

        // display the frequency info
        gdk_gc_set_rgb_fg_color(gc,&spectrumTextColor);
        sprintf(label,"<span font_desc='Sans Regular 8'>%s</span>",getFrequencyInfo(frequencyA));
        pango_layout_set_markup (layout, label, -1);
        gdk_draw_layout (GDK_DRAWABLE (spectrumPixmap), gc, spectrumWIDTH/2, height-12, layout);

        // dispay ADC Overflow
        if(getADCOverflow()) {
            adcOverflow=30;
        }

        if(adcOverflow>0) {
            gdk_gc_set_rgb_fg_color(gc,&red);
            sprintf(label,"<span font_desc='Sans Regular 8'>LT2208 ADC Overflow</span>");
            pango_layout_set_markup (layout, label, -1);
            gdk_draw_layout (GDK_DRAWABLE (spectrumPixmap), gc, spectrumWIDTH/4*3, height-12, layout);
            adcOverflow--;
        }

        // draw the spectrum
        gdk_gc_set_rgb_fg_color(gc,&plotColor);
        gdk_draw_lines(spectrumPixmap,gc,spectrumPoints,spectrumWIDTH);

        // update the spectrum
        gtk_widget_queue_draw(spectrum);

        g_object_unref(context);
        g_object_unref(layout);
        g_object_unref(gc);
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Plot spectrum
* 
* @param samples
* @param height
*/
void plotSpectrum(float* samples,int height) {

    // plot the spectrum
    int num_samples;
    int start_sample_index;
    float slope;
    int yRange=spectrumMAX-spectrumMIN;
    long long f;
    float hzPerPixel;

    f=frequencyA-(sampleRate/2);
    hzPerPixel=(float)sampleRate/(float)spectrumWIDTH;

    start_sample_index=(SPECTRUM_BUFFER_SIZE>>1)+((spectrumLow*SPECTRUM_BUFFER_SIZE)/sampleRate);
    num_samples=(int)((spectrumHigh-spectrumLow)*SPECTRUM_BUFFER_SIZE/sampleRate);
    if (start_sample_index < 0) start_sample_index = 0;
    if ((num_samples - start_sample_index) > (SPECTRUM_BUFFER_SIZE+1))
        num_samples = SPECTRUM_BUFFER_SIZE-start_sample_index;
    slope = (float)num_samples/(float)spectrumWIDTH;

    int spectrum_max_x=0;
    float spectrum_max_y = -1000000.0F;
    int i;

    if(spectrumAverage) {
        if(initAverage) {
            for(i=0;i<SPECTRUM_BUFFER_SIZE;i++) {
                averageSpectrum[i]=samples[i];
            }
            initAverage=0;
        } else {
            for(i=0;i<SPECTRUM_BUFFER_SIZE;i++) {
                averageSpectrum[i] = (samples[i] * (1 - spectrumAverageSmoothing)) + (averageSpectrum[i] * spectrumAverageSmoothing);
            }
        }
    } else {
        for(i=0;i<SPECTRUM_BUFFER_SIZE;i++) {
            averageSpectrum[i]=samples[i];
        }
    }

    for(i=0; i<spectrumWIDTH; i++) {
        float max = -1000000.0F;
        float dval = i*slope + start_sample_index;
        int lindex = (int)floorf(dval);
        int rindex = (int)floorf(dval + slope);
        if (rindex > SPECTRUM_BUFFER_SIZE) rindex = SPECTRUM_BUFFER_SIZE;

        if(f>0) {
            int j;
            for(j=lindex;j<rindex;j++) {
                if (averageSpectrum[j] > max) max=averageSpectrum[j];
            }

            max = max + displayCalibrationOffset + preampOffset;
        } else {
            max=-250;
        }

        // save for waterfall
        waterfall[i]=max;

        if(max > spectrum_max_y)
        {
            spectrum_max_y = max;
            spectrum_max_x = i;
        }

        spectrumPoints[i].y = (int)(floorf(((float)spectrumMAX - (float)max)*(float)height/(float)yRange));
        spectrumPoints[i].x = i;
        f+=(long long)hzPerPixel;

    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Draw waterfall
* 
* @param y
* @param height
*/
void drawWaterfall(int y,int height) {
    // get the spectrum context - just copy the window GC and modify
    GdkGC* gc;
    int i,j;
    int R,G,B;

    if(spectrum->window) {

        gc=gdk_gc_new(spectrum->window);

        /* Pointer to current pixel */
        static guchar *pix;

        // shift down the lines of the waterfall
        for( i = height-2; i > 0; i-- ) {
            pix = waterfallPixels + rowStride * i + nChannels;
            for( j = 0; j < spectrumWIDTH; j++ ) {
                pix[0] = pix[ -rowStride];
                pix[1] = pix[1-rowStride];
                pix[2] = pix[2-rowStride];
                pix += nChannels;
            }
        }

        // plot the latest line
        pix=waterfallPixels;
        for(i=0;i<spectrumWIDTH;i++) {
            if(waterfall[i]<waterfallLowThreshold) {
                pix[0]=colorLowR;
                pix[1]=colorLowG;
                pix[2]=colorLowB;
            } else if(waterfall[i]>waterfallHighThreshold) {
                pix[0]=colorHighR;
                pix[1]=colorHighG;
                pix[2]=colorHighB;
            } else {
                float range=waterfallHighThreshold-waterfallLowThreshold;
                float offset=waterfall[i]-waterfallLowThreshold;
                float percent=offset/range;
                if(percent<(2.0f/9.0f)) {
                    float local_percent = percent / (2.0f/9.0f);
                    R = (int)((1.0f-local_percent)*colorLowR);
                    G = (int)((1.0f-local_percent)*colorLowG);
                    B = (int)(colorLowB + local_percent*(255-colorLowB));
                } else if(percent<(3.0f/9.0f)) {
                    float local_percent = (percent - 2.0f/9.0f) / (1.0f/9.0f);
                    R = 0;
                    G = (int)(local_percent*255);
                    B = 255;
                } else if(percent<(4.0f/9.0f)) {
                     float local_percent = (percent - 3.0f/9.0f) / (1.0f/9.0f);
                     R = 0;
                     G = 255;
                     B = (int)((1.0f-local_percent)*255);
                } else if(percent<(5.0f/9.0f)) {
                     float local_percent = (percent - 4.0f/9.0f) / (1.0f/9.0f);
                     R = (int)(local_percent*255);
                     G = 255;
                     B = 0;
                } else if(percent<(7.0f/9.0f)) {
                     float local_percent = (percent - 5.0f/9.0f) / (2.0f/9.0f);
                     R = 255;
                     G = (int)((1.0f-local_percent)*255);
                     B = 0;
                } else if(percent<(8.0f/9.0f)) {
                     float local_percent = (percent - 7.0f/9.0f) / (1.0f/9.0f);
                     R = 255;
                     G = 0;
                     B = (int)(local_percent*255);
                } else {
                     float local_percent = (percent - 8.0f/9.0f) / (1.0f/9.0f);
                     R = (int)((0.75f + 0.25f*(1.0f-local_percent))*255.0f);
                     G = (int)(local_percent*255.0f*0.5f);
                     B = 255;
                }
                pix[0]=R;
                pix[1]=G;
                pix[2]=B;
            }
            pix+=nChannels;
        }
        

        gdk_draw_pixbuf(spectrumPixmap,gc,waterfallPixbuf,0,0,0,spectrumHEIGHT/2,spectrumWIDTH,spectrumHEIGHT/2,GDK_RGB_DITHER_NONE,0,0);

        g_object_unref(gc);
    }
}

#define NUM_PHASE_POINTS 100

/* --------------------------------------------------------------------------*/
/** 
* @brief Plot phase graph
* 
* @param samples
*/
void plotPhase(float* samples) {
        int x=0;
        int y=0;
        int i;
        for(i=0;i<NUM_PHASE_POINTS;i++) {
            x = (int)(samples[i*2]*(float)spectrumHEIGHT/2.0F);
            y = (int)(samples[(i*2)+1]*(float)spectrumHEIGHT/2.0F);
            spectrumPoints[i].x = (spectrumWIDTH/2)+x;
            spectrumPoints[i].y = (spectrumHEIGHT/2)+y;
        }
}


/* --------------------------------------------------------------------------*/
/** 
* @brief Draw phase graph
*/
void drawPhase() {

    // get the spectrum context - just copy the window GC and modify
    GdkGC* gc;
    if(spectrum->window) {

        gc=gdk_gc_new(spectrum->window);
        gdk_gc_copy(gc,spectrum->style->black_gc);

        gdk_draw_rectangle(spectrumPixmap,gc,TRUE,0,0,spectrumWIDTH,spectrumHEIGHT);

        // draw the grid

        // plot the data
        gdk_gc_set_rgb_fg_color(gc,&plotColor);
        gdk_draw_polygon(spectrumPixmap,gc,FALSE,spectrumPoints,NUM_PHASE_POINTS);

        // update the spectrum
        gtk_widget_queue_draw(spectrum);

        g_object_unref(gc);
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Plot phase2 graph
* 
* @param samples
*/
void plotPhase2(float* samples) {
        int x=0;
        int y=0;
        int i;
        for(i=0;i<NUM_PHASE_POINTS;i++) {
            x = (int)(samples[i*2]*(float)spectrumHEIGHT*0.5*500.0);
            y = (int)(samples[i*2+1]*(float)spectrumHEIGHT*0.5*500.0);
            spectrumPoints[i].x = (spectrumWIDTH/2)+x;
            spectrumPoints[i].y = (spectrumHEIGHT/2)+y;
        }
}


/* --------------------------------------------------------------------------*/
/** 
* @brief Draw phase2 graph
*/
void drawPhase2() {

    // get the spectrum context - just copy the window GC and modify
    GdkGC* gc;
    if(spectrum->window) {

        gc=gdk_gc_new(spectrum->window);
        gdk_gc_copy(gc,spectrum->style->black_gc);

        gdk_draw_rectangle(spectrumPixmap,gc,TRUE,0,0,spectrumWIDTH,spectrumHEIGHT);

        // draw the grid

        // plot the data
        gdk_gc_set_rgb_fg_color(gc,&plotColor);
        gdk_draw_polygon(spectrumPixmap,gc,FALSE,spectrumPoints,NUM_PHASE_POINTS);

        // update the spectrum
        gtk_widget_queue_draw(spectrum);

        g_object_unref(gc);
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Trun off spectrum update
*/
void spectrumUpdateOff() {

    // get the spectrum context - just copy the window GC and modify
    GdkGC* gc;

    if(spectrum->window) {
        gc=gdk_gc_new(spectrum->window);
        gdk_gc_copy(gc,spectrum->style->black_gc);
        gdk_draw_rectangle(spectrumPixmap,gc,TRUE,0,0,spectrumWIDTH,spectrumHEIGHT);

        // update the spectrum
        gtk_widget_queue_draw(spectrum);

        g_object_unref(gc);
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set spectrum mode
* 
* @param mode
*/
void setSpectrumMode(int mode) {
    spectrumMode=mode;

    if(spectrumMode==spectrumNONE) {
        spectrumUpdateOff();
    }

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Save the spectrum state
*/
void spectrumSaveState() {
    char string[128];

    sprintf(string,"%d",spectrumAverage);
    setProperty("spectrum.average",string);

    sprintf(string,"%f",spectrumAverageSmoothing);
    setProperty("spectrum.average.smoothing",string);

    sprintf(string,"%d",spectrumMAX);
    setProperty("spectrum.max",string);

    sprintf(string,"%d",spectrumMIN);
    setProperty("spectrum.min",string);

    sprintf(string,"%d",spectrumSTEP);
    setProperty("spectrum.step",string);

    sprintf(string,"%f",waterfallHighThreshold);
    setProperty("waterfall.high",string);

    sprintf(string,"%f",waterfallLowThreshold);
    setProperty("waterfall.low",string);

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Restore the spectrum state
*/
void spectrumRestoreState() {
    char* value;

    value=getProperty("spectrum.average");
    if(value) spectrumAverage=(gboolean)atoi(value);

    value=getProperty("spectrum.average.smoothing");
    if(value) spectrumAverageSmoothing=atof(value);

    value=getProperty("spectrum.max");
    if(value) spectrumMAX=atoi(value);

    value=getProperty("spectrum.min");
    if(value) spectrumMIN=atoi(value);

    value=getProperty("spectrum.step");
    if(value) spectrumSTEP=atoi(value);

    value=getProperty("waterfall.high");
    if(value) waterfallHighThreshold=atof(value);

    value=getProperty("waterfall.low");
    if(value) waterfallLowThreshold=atof(value);

}

