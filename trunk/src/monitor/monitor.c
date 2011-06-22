/**
* @file monitor.c
* @brief simple spectrum display to monitor a receiver
* @author John Melton, G0ORX/N6LYT
* @version 0.1
* @date 2009-10-13
*/

/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
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


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <gtk/gtk.h>

#include <fftw3.h>

#include "monitor.h"
#include "command.h"
#include "configure.h"
#include "iqthread.h"
#include "property.h"

#define WIDTH 480
#define HEIGHT 100

static int receiver=-1;
static long frequency;
static char server[80];

static char command[80];

static fftwf_complex* timebuf;
static fftwf_complex* freqbuf;
static fftwf_plan plan;
static float* result;
static float* filter;

static GtkWidget* main_window;
static GtkWidget* spectrum;
static GdkPixmap* spectrum_pixmap=NULL;
static GdkPoint* iq_points;
static GdkColor plot_color;

static GdkPixbuf *waterfall_pixbuf;
static guchar* waterfall_pixels;
static int row_stride;
static int n_channels;

static float* waterfall;
static float waterfall_high=-60.0;
static float waterfall_low=-130.0;

static int colorLowR=128; // black
static int colorLowG=128;
static int colorLowB=128;

static int colorMidR=255; // red
static int colorMidG=0;
static int colorMidB=0;

static int colorHighR=255; // yellow
static int colorHighG=255;
static int colorHighB=0;

static float display_calibration_offset=-82.62103f;

static gint timer_id;

static int fps=10;
static float max_display=0.0;
static float min_display=-60.0;

static int sample_rate;

static char property_path[80];

static int window_x;
static int window_y;

static struct option long_options[] = {
    {"receiver",required_argument, 0, 0},
    {"frequency",required_argument, 0, 1},
    {"server",required_argument, 0, 2},
    {"update",required_argument, 0, 3},
    {"high",required_argument, 0, 4},
    {"low",required_argument, 0, 5},
    {0,0,0,0}
};
static char* short_options="";
static int option_index;

void process_args(int argc,char* argv[]);
void init_monitor();
void build_ui();
int quit();
void process_iq_samples(float* buffer);
void process_samples();
void plot_samples();
void draw_samples();
void draw_waterfall();
float *blackman_harris_filter(int n);

int main(int argc,char* argv[]) {

    char* response;
    char* token;

    gtk_init(&argc,&argv);

    process_args(argc,argv);

    while(receiver==-1) {
        GtkDialog* dialog=(GtkDialog*)configure(TRUE);
        gtk_dialog_run(dialog);
        frequency=configure_get_frequency();
        configure_destroy();
    }

/*
    sprintf(property_path,".ghpsdr.rx%d.properties",receiver);
    properties_load(property_path);

    token=property_get("window.x");
    if(token) window_x=atoi(token);
    token=property_get("window.y");
    if(token) window_y=atoi(token);
    token=property_get("fps");
    if(token) fps=atoi(token);
    token=property_get("frequency");
    if(token) frequency=atol(token);
    token=property_get("waterfall.high");
    if(token) waterfall_high=atof(token);
    token=property_get("waterfall.low");
    if(token) waterfall_low=atof(token);
*/

    init_command(server);

    init_monitor();

    sprintf(command,"attach %d",receiver);
    response=send_command(command);
    token=strtok(response," ");
    if(token==NULL) {
        fprintf(stderr,"null response to attach command!\n");
        exit(1);
    }
    if(strcmp(token,"OK")==0) {
        token=strtok(NULL," ");
        if(token==NULL) {
            fprintf(stderr,"expected sample rate in response to attach command!\n");
            exit(1);
        } else {
            sample_rate=atoi(token);
        }
    } else {
        fprintf(stderr,"Error: %s!\n",response);
        exit(1);
    }

    sprintf(command,"frequency %ld",frequency);
    send_command(command);

    build_ui();

    init_iq_thread(receiver);
    start_iq_thread((void*)process_iq_samples);
    sprintf(command,"start iq %d",get_iq_port());
    send_command(command);
    
    timer_id=gtk_timeout_add(1000/fps,process_samples,NULL);

    gtk_main();

    close_command();

    return 0;
}

void process_args(int argc,char* argv[]) {
    int i;

    // set defaults
    receiver=-1;
    strcpy(server,"127.0.0.1");
    frequency=7056000L;
    
    while((i=getopt_long(argc,argv,short_options,long_options,&option_index))!=EOF) {
        switch(option_index) {
            case 0: // receiver
                receiver=atoi(optarg);
                break;
            case 1: // frequency
                frequency=atol(optarg);
                break;
            case 2: // server
                strcpy(server,optarg);
                break;
            case 3: // update fps
                fps=atoi(optarg);
                break;
            case 4: // waterfall_high
                waterfall_high=(float)atoi(optarg);
                break;
            case 5: // waterfall_low
                waterfall_low=(float)atoi(optarg);
                break;
	    default:
                fprintf(stderr,"Usage: \n");
                fprintf(stderr,"  monitor --receivers N (default 1)\n");
                fprintf(stderr,"          --server 0.0.0.0 (default 127.0.0.1)\n");
                fprintf(stderr,"          --frequency 0 (default 705600)\n");
                fprintf(stderr,"          --update 15 (default 15) \n");
                fprintf(stderr,"          --high -60 (default -60) \n");
                fprintf(stderr,"          --update -130 (default -130) \n");
                exit(1);

        }
    }
}

void init_monitor() {
    int i;

    // prepare the fft (time domain to frequency domain)
    timebuf=(fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*BUFFER_SIZE);
    freqbuf=(fftwf_complex*)fftwf_malloc(sizeof(fftwf_complex)*BUFFER_SIZE);
    plan=fftwf_plan_dft_1d(BUFFER_SIZE,timebuf,freqbuf,FFTW_FORWARD,FFTW_ESTIMATE);
    result=malloc(BUFFER_SIZE*sizeof(float));
    filter=blackman_harris_filter(BUFFER_SIZE);

    waterfall=malloc(WIDTH*sizeof(float));

    iq_points=calloc(WIDTH,sizeof(GdkPoint));
    for(i=0;i<WIDTH;i++) {
        iq_points[i].x=i;
        iq_points[i].y=50;
    }

    plot_color.red=65535;
    plot_color.green=65535;
    plot_color.blue=65535;
}

gboolean spectrum_configure_event(GtkWidget* widget,GdkEventConfigure* event) {
    if(spectrum_pixmap) g_object_unref(spectrum_pixmap);
    spectrum_pixmap=gdk_pixmap_new(widget->window,widget->allocation.width,widget->allocation.height,-1);
    gdk_draw_rectangle(spectrum_pixmap,
                       widget->style->black_gc,
                       TRUE,
                       0,0,
                       widget->allocation.width,
                       widget->allocation.height);
    return TRUE;
}

gboolean spectrum_expose_event(GtkWidget* widget,GdkEventExpose* event) {
    gdk_draw_drawable(widget->window,
                    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                    spectrum_pixmap,
                    event->area.x, event->area.y,
                    event->area.x, event->area.y,
                    event->area.width, event->area.height);
    return FALSE;
}

void set_title() {
    char title[80];
    long low;
    long high;

    low=frequency-(sample_rate/2);
    high=frequency+(sample_rate/2);
    sprintf(title,"HPSDR: RX%d %ld.%03ld to %ld.%03ld MHz",receiver,low/1000000,low%1000000/1000,high/1000000,high%1000000/1000);
    gtk_window_set_title((GtkWindow*)main_window,title);
}

gboolean spectrum_button_press_event(GtkWidget* widget,GdkEventButton* event) {
    GtkDialog* dialog;
    switch(event->button) {
        case 1: // left button
        case 2: // middle button
        case 3: // right button
            dialog=(GtkDialog*)configure(FALSE);
            gtk_dialog_run(dialog);
            frequency=configure_get_frequency();
            configure_destroy();
            sprintf(command,"frequency %ld",frequency);
            send_command(command);
            set_title();
            break;
    }
}

void build_ui() {

    waterfall_pixbuf=gdk_pixbuf_new(GDK_COLORSPACE_RGB,FALSE,8,WIDTH,HEIGHT);
    waterfall_pixels=gdk_pixbuf_get_pixels(waterfall_pixbuf);
    n_channels=gdk_pixbuf_get_n_channels(waterfall_pixbuf);
    row_stride=gdk_pixbuf_get_rowstride(waterfall_pixbuf);


    main_window=gtk_window_new(GTK_WINDOW_TOPLEVEL);

    set_title();

    g_signal_connect(G_OBJECT(main_window),"delete_event",G_CALLBACK(quit),NULL);

    spectrum=gtk_drawing_area_new();
    gtk_widget_set_size_request(spectrum,WIDTH,HEIGHT);
    
    g_signal_connect(G_OBJECT(spectrum),"configure_event",G_CALLBACK(spectrum_configure_event),NULL);
    g_signal_connect(G_OBJECT(spectrum),"expose_event",G_CALLBACK(spectrum_expose_event),NULL);
    g_signal_connect(G_OBJECT(spectrum),"button_press_event",G_CALLBACK(spectrum_button_press_event),NULL);

    gtk_widget_set_events(spectrum, GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK);

    gtk_container_add(GTK_CONTAINER(main_window),spectrum);

    gtk_widget_show(spectrum);

    gtk_window_move((GtkWindow*)main_window,window_x,window_y);

    gtk_widget_show(main_window);
}

int quit() {
    char temp[128];

    gtk_window_get_position((GtkWindow*)main_window,&window_x,&window_y);
    sprintf(temp,"%d",window_x);
    property_put("window.x",temp);
    sprintf(temp,"%d",window_y);
    property_put("window.y",temp);
    sprintf(temp,"%d",fps);
    property_put("fps",temp);
    sprintf(temp,"%ld",frequency);
    property_put("frequency",temp);
    sprintf(temp,"%f",waterfall_high);
    property_put("waterfall.high",temp);
    sprintf(temp,"%f",waterfall_low);
    property_put("waterfall.low",temp);

    properties_save(property_path);

    gtk_main_quit();

    return FALSE;
}

void process_iq_samples(float* buffer) {
    int i;

    for(i=0;i<BUFFER_SIZE;i++) {
        timebuf[i][0]=buffer[i]*filter[i];
        timebuf[i][1]=buffer[i+BUFFER_SIZE]*filter[i];
    }

    fftwf_execute(plan);

    // compute power levels
    for(i=0;i<BUFFER_SIZE;i++) {
        result[i]=10.0f*log10(sqrt((freqbuf[i][0]*freqbuf[i][0]) +
                  (freqbuf[i][1]*freqbuf[i][1]))+1e-60);
    }
}

void process_samples() {
    plot_samples();
    //draw_samples();
    draw_waterfall();
}

void plot_samples() {
    int i,j;
    float range=max_display-min_display;
    float slope=(float)BUFFER_SIZE/(float)WIDTH;

    for(i=0;i<WIDTH;i++) {
        float max=-1000000.0F;
        float dval=(float)i*slope;
        int lindex=(int)floorf(dval);
        int rindex=(int)floorf(dval+slope);
        if(rindex>BUFFER_SIZE) rindex=BUFFER_SIZE;
        for(j=lindex;j<rindex;j++) {
            if(result[j]>max) {
                max=result[j];
            }
        }
        waterfall[i]=max+display_calibration_offset;
//fprintf(stderr,"[%d] %f\n",i,max);
    }
/*
    for(i=0;i<BUFFER_SIZE;i++) {
        iq_points[i].x=i;
        iq_points[i].y=(int)(floor((max_display-result[(i+(BUFFER_SIZE/2))%BUFFER_SIZE])*(float)HEIGHT/range));
    }
*/
}

void draw_samples() {
    GdkGC* gc;

    if(spectrum->window) {
        gc=gdk_gc_new(spectrum->window);
        gdk_gc_copy(gc,spectrum->style->black_gc);
        gdk_draw_rectangle(spectrum_pixmap,gc,TRUE,0,0,WIDTH,HEIGHT);
        gdk_gc_set_rgb_fg_color(gc,&plot_color);
        gdk_draw_lines(spectrum_pixmap,gc,iq_points,WIDTH);
        gtk_widget_queue_draw(spectrum);
        g_object_unref(gc);
    }
}

void draw_waterfall() {
    GdkGC* gc;
    int i,j;
    int R,G,B;
    guchar *pix;
    float sample;

    if(spectrum->window) {
        gc=gdk_gc_new(spectrum->window);

        // shift down the lines of the waterfall
        for( i = HEIGHT; i > 0; i-- ) {
            pix = waterfall_pixels + row_stride * i + n_channels;
            for( j = 0; j < WIDTH; j++ ) {
                pix[0] = pix[ -row_stride];
                pix[1] = pix[1-row_stride];
                pix[2] = pix[2-row_stride];
                pix += n_channels;
            }
        }

// plot the latest line
        pix=waterfall_pixels+row_stride-n_channels;
        for(i=0;i<WIDTH;i++) {
            sample=waterfall[(i+(WIDTH/2))%WIDTH];
            if(sample<waterfall_low) {
                pix[0]=colorLowR;
                pix[1]=colorLowG;
                pix[2]=colorLowB;
            } else if(sample>waterfall_high) {
                pix[0]=colorHighR;
                pix[1]=colorHighG;
                pix[2]=colorHighB;
            } else {
                float range=waterfall_high-waterfall_low;
                float offset=sample-waterfall_low;
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
            pix-=n_channels;
        }

        gdk_draw_pixbuf(spectrum_pixmap,gc,waterfall_pixbuf,0,0,0,0,WIDTH,HEIGHT,GDK_RGB_DITHER_NONE,0,0);
        gtk_widget_queue_draw(spectrum);

        g_object_unref(gc);
    }
}

float *blackman_harris_filter(int n) {
    float* bhf;
    float a0=0.35875F,
          a1=0.48829F,
          a2=0.14128F,
          a3=0.01168F;
    float twopi=M_PI*2.0F;
    float fourpi=M_PI*4.0F;
    float sixpi=M_PI*6.0F;
    int i;

    bhf=malloc(n*sizeof(float));
    for(i = 0;i<n;i++) {
        bhf[i]=a0
                  - a1 * cos(twopi  * ((float)i + 0.5) / (float)(n-1))
                  + a2 * cos(fourpi * ((float)i + 0.5) / (float)(n-1))
                  - a3 * cos(sixpi * ((float)i + 0.5) / (float)(n-1));
    }
    return bhf;
}

int get_receiver() {
    return receiver;
}

void set_receiver(int r) {
    receiver=r;
}

float get_waterfall_high() {
    return waterfall_high;
}

void set_waterfall_high(float high) {
    waterfall_high=high;
}

float get_waterfall_low() {
    return waterfall_low;
}

void set_waterfall_low(float low) {
    waterfall_low=low;
}

int get_update_fps() {
    return fps;
}

void set_update_fps(int update) {
    fps=update;
}


long get_frequency() {
    return frequency;
}

void set_frequency(long f) {
    frequency=f;
}
