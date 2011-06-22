/** 
* @file subrx.c
* @brief Receiver 2 files for GHPSDR
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-11
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
#include <stdio.h>
#include <stdlib.h>
#include "bandstack.h"
#include "command.h"
#include "subrx.h"
#include "main.h"
#include "property.h"
#include "ozy.h"
#include "dttsp.h"
#include "vfo.h"

GtkWidget* subrxFrame;
GtkWidget* subrxTable;

GtkWidget* subrxEnabled;
GtkWidget* subrxGainFrame;
GtkWidget* subrxGainScale;
GtkWidget* subrxPanFrame;
GtkWidget* subrxPanScale;
GtkWidget* subrxFrequencyDisplay;
GdkPixmap* subrxPixmap;

float subrxGain=10.0;
float subrxPan=50.0;

long long subrxFrequency;
long long subrxFrequencyLO;
long long subrxFrequencyDsp;
long long subrxFrequencyDds;

gboolean subrx=FALSE;

void setSubrxFrequency(long long f); 
void subrxIncrementFrequency(long increment);

/* --------------------------------------------------------------------------*/
/**
* @brief Callback when subrxFrequencyDisplay is created
*
* @param widget
* @param event
*
* @return
*/
gboolean subrxFrequencyDisplayConfigure(GtkWidget* widget,GdkEventConfigure* event) {
    GdkGC* gc;
    PangoContext *context;
    PangoLayout *layout;
    char temp[128];

    if(subrxPixmap) g_object_unref(subrxPixmap);

    subrxPixmap=gdk_pixmap_new(widget->window,widget->allocation.width,widget->allocation.height,-1);

    gc=gdk_gc_new(widget->window);
    gdk_gc_set_rgb_fg_color(gc,&background);
    gdk_draw_rectangle(subrxPixmap,
                       gc,
                       TRUE,
                       0,0,
                       widget->allocation.width,
                       widget->allocation.height);

    context = gdk_pango_context_get_for_screen(gdk_screen_get_default ());
    layout = pango_layout_new(context);
    pango_layout_set_width(layout,widget->allocation.width*PANGO_SCALE);
    pango_layout_set_alignment(layout,PANGO_ALIGN_RIGHT);
    sprintf(temp,"<span foreground='%s' background='#2C2C2C' font_desc='Sans Bold 24'>% 7lld.%03lld.%03lld </span>",subrx?"#00FF00":"#C0C0C0",subrxFrequency/1000000LL,(subrxFrequency%1000000LL)/1000LL,subrxFrequency%1000LL);
    pango_layout_set_markup(layout,temp,-1);
    gdk_draw_layout(GDK_DRAWABLE(subrxPixmap),gc,0,0,layout);

    gdk_gc_set_rgb_fg_color(gc,&grey);
    gdk_draw_rectangle(subrxPixmap,
                       gc,
                       FALSE,
                       0,0,
                       widget->allocation.width-1,
                       widget->allocation.height-1);

    g_object_unref(context);
    g_object_unref(layout);
    g_object_unref(gc);

    return TRUE;
}

/* --------------------------------------------------------------------------*/
/**
* @brief Callback when subrx is exposed - need to paint it from the pixmap
*
* @param widget
* @param event
*
* @return
*/
gboolean subrxFrequencyDisplayExpose(GtkWidget* widget,GdkEventExpose* event) {
    gdk_draw_drawable(widget->window,
                    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                    subrxPixmap,
                    event->area.x, event->area.y,
                    event->area.x, event->area.y,
                    event->area.width, event->area.height);
    return FALSE;
}

/* --------------------------------------------------------------------------*/
/**
* @brief update vfo a display
*
* @param widget
* @param event
*
* @return
*/
void updateSubrxDisplay() {
    GdkGC* gc;
    PangoContext *context;
    PangoLayout *layout;
    char temp[128];

    if(subrxFrequencyDisplay->window) {
        gc=gdk_gc_new(subrxFrequencyDisplay->window);
        gdk_gc_set_rgb_fg_color(gc,&background);
        gdk_draw_rectangle(subrxPixmap,
                           gc,
                           TRUE,
                           0,0,
                           subrxFrequencyDisplay->allocation.width,
                           subrxFrequencyDisplay->allocation.height);

        context = gdk_pango_context_get_for_screen (gdk_screen_get_default ());
        layout = pango_layout_new (context);
        pango_layout_set_width(layout,subrxFrequencyDisplay->allocation.width*PANGO_SCALE);
        pango_layout_set_alignment(layout,PANGO_ALIGN_RIGHT);
        sprintf(temp,"<span foreground='%s' background='#2C2C2C' font_desc='Sans Bold 24'>% 7lld.%03lld.%03lld </span>",subrx?"#00FF00":"#C0C0C0",subrxFrequency/1000000LL,(subrxFrequency%1000000LL)/1000LL,subrxFrequency%1000LL);
        pango_layout_set_markup(layout,temp,-1);
        gdk_draw_layout(GDK_DRAWABLE(subrxPixmap),gc,0,0,layout);

        gdk_gc_set_rgb_fg_color(gc,&grey);
        gdk_draw_rectangle(subrxPixmap,
                           gc,
                           FALSE,
                           0,0,
                           subrxFrequencyDisplay->allocation.width-1,
                           subrxFrequencyDisplay->allocation.height-1);

        g_object_unref(context);
        g_object_unref(layout);
        g_object_unref(gc);

        gtk_widget_queue_draw(subrxFrequencyDisplay);
    }
}

/* --------------------------------------------------------------------------*/
/**
* @brief Frequency scroll wheel
*
* @param widget
* @param event
*
* @return
*/
gboolean subrx_frequency_scroll_event(GtkWidget* widget,GdkEventScroll* event) {
    long increment;
    if(event->direction==GDK_SCROLL_UP) {
        increment=frequencyIncrement;
    } else {
        increment=-frequencyIncrement;
    }
    subrxIncrementFrequency(increment);
}


/* --------------------------------------------------------------------------*/
/** 
* @brief Enabled button Callback 
* 
* @param widget -- pointer to the parent widget, 
* @param data -- pointer to the data.
*/
void subrxEnabledButtonCallback(GtkWidget* widget,gpointer data) {
    GtkWidget* label;
    char c[80];
    gboolean state;

    if(subrx) {
        state=0;
    } else {
        state=1;
    }    

    label=gtk_bin_get_child((GtkBin*)widget);
    if(state) {
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &buttonSelected);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &buttonSelected);
    } else {
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &black);
    }

    subrx=state;

    if(subrx) {
        subrxFrequencyLO=frequencyALO;
        long long diff=frequencyA-subrxFrequency;
        if(diff<0) diff=-diff;
        if(diff>sampleRate/2) {
            setSubrxFrequency(frequencyA);
        } else {
            setSubrxFrequency(subrxFrequency);
        }
        SetSubRXSt(0,1,TRUE);
    } else {
        SetSubRXSt(0,1,FALSE);
    }
    updateSubrxDisplay();
}

/* --------------------------------------------------------------------------*/
/**
* @brief  Callback when subrx gain values changes
*
* @param widget
* @param data
*/
void subrxGainChanged(GtkWidget* widget,gpointer data) {
    char command[80];
    subrxGain=gtk_range_get_value((GtkRange*)subrxGainScale);
    SetRXOutputGain(0,1,subrxGain/100.0);
}

/* --------------------------------------------------------------------------*/
/**
* @brief  Callback when subrx pan values changes
*
* @param widget
* @param data
*/
void subrxPanChanged(GtkWidget* widget,gpointer data) {
    char command[80];
    subrxPan=gtk_range_get_value((GtkRange*)subrxPanScale);
    SetRXPan(0,1,subrxPan);

}


/* --------------------------------------------------------------------------*/
/** 
* @brief Build Transmit User Interface 
* 
* @return GtkWidget pointer 
*/
GtkWidget* buildSubRxUI() {

    GtkWidget* label;

    subrxFrame=gtk_frame_new("Sub RX");
    gtk_widget_modify_bg(subrxFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(subrxFrame)),GTK_STATE_NORMAL,&white);

    subrxTable=gtk_table_new(2,8,TRUE);

    // subrx settings

    subrxFrequencyDisplay=gtk_drawing_area_new();
    //gtk_widget_set_size_request(GTK_WIDGET(subrxFrequencyDisplay),250,35);
    g_signal_connect(G_OBJECT (subrxFrequencyDisplay),"configure_event",G_CALLBACK(subrxFrequencyDisplayConfigure),NULL);
    g_signal_connect(G_OBJECT (subrxFrequencyDisplay),"expose_event",G_CALLBACK(subrxFrequencyDisplayExpose),NULL);
    g_signal_connect(G_OBJECT(subrxFrequencyDisplay),"scroll_event",G_CALLBACK(subrx_frequency_scroll_event),NULL);
    gtk_widget_set_events(subrxFrequencyDisplay,GDK_EXPOSURE_MASK|GDK_SCROLL_MASK);
    gtk_widget_show(subrxFrequencyDisplay);
    gtk_table_attach_defaults(GTK_TABLE(subrxTable),subrxFrequencyDisplay,1,8,0,1);


    subrxEnabled = gtk_button_new_with_label ("SubRX");
    gtk_widget_modify_bg(subrxEnabled, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)subrxEnabled);

    if(subrx) {
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &buttonSelected);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &buttonSelected);
    } else {
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &black);
    }

    gtk_widget_set_size_request(GTK_WIDGET(subrxEnabled),50,25);
    g_signal_connect(G_OBJECT(subrxEnabled),"clicked",G_CALLBACK(subrxEnabledButtonCallback),NULL);
    gtk_widget_show(subrxEnabled);
    gtk_table_attach_defaults(GTK_TABLE(subrxTable),subrxEnabled,0,1,0,1);

    // subrx gain
    subrxGainFrame=gtk_frame_new("AF Gain");
    gtk_widget_modify_bg(subrxGainFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(subrxGainFrame)),GTK_STATE_NORMAL,&white);

    subrxGainScale=gtk_hscale_new_with_range(0.0,100.0,10.0);
    g_signal_connect(G_OBJECT(subrxGainScale),"value-changed",G_CALLBACK(subrxGainChanged),NULL);
    gtk_range_set_value((GtkRange*)subrxGainScale,subrxGain);
    gtk_widget_set_size_request(GTK_WIDGET(subrxGainScale),150,30);
    gtk_widget_show(subrxGainScale);
    gtk_container_add(GTK_CONTAINER(subrxGainFrame),subrxGainScale);
    gtk_widget_show(subrxGainFrame);
    gtk_table_attach_defaults(GTK_TABLE(subrxTable),subrxGainFrame,0,4,1,2);

    SetRXOutputGain(0,1,subrxGain/100.0);

    // subrx pan
    subrxPanFrame=gtk_frame_new("AF Pan");
    gtk_widget_modify_bg(subrxPanFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(subrxPanFrame)),GTK_STATE_NORMAL,&white);

    subrxPanScale=gtk_hscale_new_with_range(0.0,1.0,0.1);
    g_signal_connect(G_OBJECT(subrxPanScale),"value-changed",G_CALLBACK(subrxPanChanged),NULL);
    gtk_range_set_value((GtkRange*)subrxPanScale,subrxPan);
    gtk_widget_set_size_request(GTK_WIDGET(subrxPanScale),150,30);
    gtk_widget_show(subrxPanScale);
    gtk_container_add(GTK_CONTAINER(subrxPanFrame),subrxPanScale);
    gtk_widget_show(subrxPanFrame);
    gtk_table_attach_defaults(GTK_TABLE(subrxTable),subrxPanFrame,4,8,1,2);

    SetRXPan(0,1,subrxPan);

    gtk_container_add(GTK_CONTAINER(subrxFrame),subrxTable);
    gtk_widget_show(subrxTable);
    gtk_widget_show(subrxFrame);

    return subrxFrame;

}

/* --------------------------------------------------------------------------*/
/**
* @brief Set the subrx frequency
*
* @param f
*/
void setSubrxFrequency(long long f) {
    long long diff;
    if(subrx) {
        if((f>=(frequencyA-(sampleRate/2)))&& (f<=(frequencyA+(sampleRate/2)))) {
            subrxFrequency=f;
        }
        subrxFrequencyDsp=0;
        subrxFrequencyDds=0;
        subrxFrequency=f;
        subrxFrequencyDds=f-subrxFrequencyLO;
        updateSubrxDisplay();
        diff=frequencyA-subrxFrequency;
        SetRXOsc(0,1,(double)diff-LO_OFFSET);
    }
}

/* --------------------------------------------------------------------------*/
/**
* @brief Increment the frequency
*
* @param increment
*/
void subrxIncrementFrequency(long increment) {
     int f=subrxFrequency+(long long)increment;
     if((f>=(frequencyA-(sampleRate/2)))&& (f<=(frequencyA+(sampleRate/2)))) {
         setSubrxFrequency(f);
     }
}


/* --------------------------------------------------------------------------*/
/**
* @brief Save the sub rx state
*/
void subrxSaveState() {
    char string[128];
    sprintf(string,"%d",subrx);
    setProperty("subrx",string);
    sprintf(string,"%f",subrxGain);
    setProperty("subrxGain",string);
    sprintf(string,"%f",subrxPan);
    setProperty("subrxPan",string);
    sprintf(string,"%lld",subrxFrequency);
    setProperty("subrxFrequency",string);
}

/* --------------------------------------------------------------------------*/
/**
* @brief Restore the sub rx state
*/
void subrxRestoreState() {
    char* value;
    value=getProperty("subrxGain");
    if(value) subrxGain=atof(value); else subrxGain=10.0f;
    value=getProperty("subrxPan");
    if(value) subrxPan=atof(value); else subrxPan=50.0f;
    value=getProperty("subrx");
    if(value) subrx=atoi(value); else subrx=0;
    value=getProperty("subrxFrequency");
    if(value) subrxFrequency=atol(value); else subrxFrequency=7051000;

    SetRXPan(0,1,subrxPan);
    SetRXOutputGain(0,1,subrxGain/100.0);
}

void resetSubRx() {
    if(subrx) {
        subrxEnabledButtonCallback(subrxEnabled,NULL);
    }
}
