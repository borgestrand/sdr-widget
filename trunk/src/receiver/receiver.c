/** 
* @file receiver.c
* @brief Mode functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
*/
// receiver.c

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

GtkWidget* receiverFrame;

int receiver;

float pan;

GtkWidget* panScale;

/* --------------------------------------------------------------------------*/
/** 
* @brief  Select the receiver
* 
* @param widget
*/
void selectReceiver(GtkWidget* widget) {
    GtkWidget* label;
    char temp[80];
}

/* --------------------------------------------------------------------------*/
/** 
* @brief  Callback when pan values changes
* 
* @param widget
* @param data
*/
void panChanged(GtkWidget* widget,gpointer data) {
    pan=gtk_range_get_value((GtkRange*)panScale);
    SetRXPan(0,0,pan);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief  Build the GUI
* 
* @return 
*/
GtkWidget* buildReceiverUI() {
    GtkWidget* label;

    receiverFrame=gtk_frame_new("AF Pan");
    gtk_widget_modify_bg(receiverFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(receiverFrame)),GTK_STATE_NORMAL,&white);

    panScale=gtk_hscale_new_with_range(0.0,1.0,0.1);
    g_signal_connect(G_OBJECT(panScale),"value-changed",G_CALLBACK(panChanged),NULL);
    gtk_range_set_value((GtkRange*)panScale,pan);
    gtk_widget_set_size_request(GTK_WIDGET(panScale),150,25);
    gtk_widget_show(panScale);
    gtk_container_add(GTK_CONTAINER(receiverFrame),panScale);

    gtk_widget_set_size_request(GTK_WIDGET(receiverFrame),200,55);
    gtk_widget_show(receiverFrame);

    SetRXPan(0,0,pan);

    return receiverFrame;
  
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Save the receiver state
*/
void receiverSaveState() {
    char string[128];
    sprintf(string,"%f",pan);
    setProperty("pan",string);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Restore the receiver state
*/
void receiverRestoreState() {
    char* value;
    value=getProperty("pan");
    if(value) pan=atof(value); else pan=0.5f;
}
