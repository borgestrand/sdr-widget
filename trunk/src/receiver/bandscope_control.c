/** 
* @file bandscope_control.c
* @brief Bandscope control files 
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-11
*/
// bandscope_control.c

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
#include <gdk/gdkkeysyms.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "main.h"
#include "bandscope.h"
#include "bandscope_control.h"
#include "property.h"

GtkWidget* zoomFixed;

GtkWidget* buttonOff;
GtkWidget* buttonZoom1;
GtkWidget* buttonZoom2;
GtkWidget* buttonZoom4;
GtkWidget* buttonZoom8;
GtkWidget* buttonZoom16;

GtkWidget* currentZoomButton;

/* --------------------------------------------------------------------------*/
/** 
* @brief  Select the zoom
* 
* @param widget
*/
void selectZoom(GtkWidget* widget) {
    GtkWidget* label;
    char temp[80];

    if(currentZoomButton) {
        label=gtk_bin_get_child((GtkBin*)currentZoomButton);
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &black);
    }
    label=gtk_bin_get_child((GtkBin*)widget);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &buttonSelected);
    gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &buttonSelected);
    currentZoomButton=widget;

    if(widget==buttonOff) {
        bandscopeSetZoom(0);
    } else if(widget==buttonZoom1) {
        bandscopeSetZoom(1);
    } else if(widget==buttonZoom2) {
        bandscopeSetZoom(2);
    } else if(widget==buttonZoom4) {
        bandscopeSetZoom(4);
    } else if(widget==buttonZoom8) {
        bandscopeSetZoom(8);
    } else if(widget==buttonZoom16) {
        bandscopeSetZoom(16);
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the zoom.
* 
* @param zoom
*/
void setZoom(int zoom) {
    GtkWidget* widget;
    switch(zoom) {
        case 0:
            widget=buttonOff;
            break;
        case 1:
            widget=buttonZoom1;
            break;
        case 2:
            widget=buttonZoom2;
            break;
        case 4:
            widget=buttonZoom4;
            break;
        case 8:
            widget=buttonZoom8;
            break;
        case 16:
            widget=buttonZoom16;
            break;
    }
    selectZoom(widget);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Callback when a zoom button is pressed.
* 
* @param widget
* @param data
*/
void zoomCallback(GtkWidget* widget,gpointer data) {
    selectZoom(widget);
}

//-------------------------------------------------------------------------------------------
//
// build the GUI
//
GtkWidget* buildBandscope_controlUI() {
    GtkWidget* label;


    zoomFixed=gtk_fixed_new();
    gtk_widget_modify_bg(zoomFixed,GTK_STATE_NORMAL,&background);

    buttonOff = gtk_button_new_with_label ("Off");
    gtk_widget_modify_bg(buttonOff, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonOff);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonOff),75,25);
    g_signal_connect(G_OBJECT(buttonOff),"clicked",G_CALLBACK(zoomCallback),NULL);
    gtk_widget_show(buttonOff);
    gtk_fixed_put((GtkFixed*)zoomFixed,buttonOff,0,0);

    // zoom buttons
    buttonZoom1 = gtk_button_new_with_label ("Zoom x1");
    gtk_widget_modify_bg(buttonZoom1, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonZoom1);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonZoom1),75,25);
    g_signal_connect(G_OBJECT(buttonZoom1),"clicked",G_CALLBACK(zoomCallback),NULL);
    gtk_widget_show(buttonZoom1);
    gtk_fixed_put((GtkFixed*)zoomFixed,buttonZoom1,75,0);

    buttonZoom2 = gtk_button_new_with_label ("Zoom x2");
    gtk_widget_modify_bg(buttonZoom2, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonZoom2);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonZoom2),75,25);
    g_signal_connect(G_OBJECT(buttonZoom2),"clicked",G_CALLBACK(zoomCallback),NULL);
    gtk_widget_show(buttonZoom2);
    gtk_fixed_put((GtkFixed*)zoomFixed,buttonZoom2,150,0);

    if(BANDSCOPE_MULTIPLIER>2) {
    buttonZoom4 = gtk_button_new_with_label ("Zoom x4");
    gtk_widget_modify_bg(buttonZoom4, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonZoom4);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonZoom4),75,25);
    g_signal_connect(G_OBJECT(buttonZoom4),"clicked",G_CALLBACK(zoomCallback),NULL);
    gtk_widget_show(buttonZoom4);
    gtk_fixed_put((GtkFixed*)zoomFixed,buttonZoom4,225,0);
    }

    if(BANDSCOPE_MULTIPLIER>4) {
    buttonZoom8 = gtk_button_new_with_label ("Zoom x8");
    gtk_widget_modify_bg(buttonZoom8, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonZoom8);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonZoom8),75,25);
    g_signal_connect(G_OBJECT(buttonZoom8),"clicked",G_CALLBACK(zoomCallback),NULL);
    gtk_widget_show(buttonZoom8);
    gtk_fixed_put((GtkFixed*)zoomFixed,buttonZoom8,300,0);
    gtk_widget_set_size_request(GTK_WIDGET(zoomFixed),375,25);
    }

    if(BANDSCOPE_MULTIPLIER>8) {
    buttonZoom16 = gtk_button_new_with_label ("Zoom x16");
    gtk_widget_modify_bg(buttonZoom16, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonZoom16);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonZoom16),75,25);
    g_signal_connect(G_OBJECT(buttonZoom16),"clicked",G_CALLBACK(zoomCallback),NULL);
    gtk_widget_show(buttonZoom16);
    gtk_fixed_put((GtkFixed*)zoomFixed,buttonZoom16,375,0);
    gtk_widget_set_size_request(GTK_WIDGET(zoomFixed),450,25);
    }

    gtk_widget_show(zoomFixed);

    setZoom(bandscopeZoom);

    return zoomFixed;
  
}

void bandscope_controlSaveState() {
    char string[128];
    sprintf(string,"%d",bandscopeZoom);
    setProperty("bandscope.zoom",string);
}

void bandscope_controlRestoreState() {
    char* value;
    value=getProperty("bandscope.zoom");
    if(value) bandscopeZoom=atoi(value);
}
