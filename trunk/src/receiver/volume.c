/** 
* @file volume.c
* @brief Mode functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
*/
// volume.c

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
#include "property.h"

#include "command.h"
#include "dttsp.h"
#include "main.h"

GtkWidget* volumeFrame;

double volume;

GtkWidget* volumeScale;

/* --------------------------------------------------------------------------*/
/** 
* @brief  Select the volume
* 
* @param widget
*/
void selectVolume(GtkWidget* widget) {
    GtkWidget* label;
    char temp[80];
}

/* --------------------------------------------------------------------------*/
/** 
* @brief  Callback when volume values changes
* 
* @param widget
* @param data
*/
void volumeChanged(GtkWidget* widget,gpointer data) {
    volume=gtk_range_get_value((GtkRange*)volumeScale);
    SetRXOutputGain(0,0,volume/100.0);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief  Build the GUI
* 
* @return 
*/
GtkWidget* buildVolumeUI() {
    GtkWidget* label;

    volumeFrame=gtk_frame_new("AF Gain");
    gtk_widget_modify_bg(volumeFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(volumeFrame)),GTK_STATE_NORMAL,&white);

    volumeScale=gtk_hscale_new_with_range(0.0,100.0,10.0);
    g_signal_connect(G_OBJECT(volumeScale),"value-changed",G_CALLBACK(volumeChanged),NULL);
    gtk_range_set_value((GtkRange*)volumeScale,volume);
    gtk_widget_set_size_request(GTK_WIDGET(volumeScale),150,25);
    gtk_widget_show(volumeScale);
    gtk_container_add(GTK_CONTAINER(volumeFrame),volumeScale);

    gtk_widget_set_size_request(volumeFrame,200,55);
    gtk_widget_show(volumeFrame);

    SetRXOutputGain(0,0,volume/100.0);

    return volumeFrame;
  
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Save the volume state
*/
void volumeSaveState() {
    char string[128];
    sprintf(string,"%f",volume);
    setProperty("volume",string);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Restore the volume state
*/
void volumeRestoreState() {
    char* value;
    value=getProperty("volume");
    if(value) volume=atof(value); else volume=3.0f;
}
