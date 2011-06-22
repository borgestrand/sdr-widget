/** 
* @file preamp.c
* @brief Preamp files for GHPSDR
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
#include "bandstack.h"
#include "command.h"
#include "preamp.h"
#include "main.h"
#include "property.h"
#include "ozy.h"

GtkWidget* preampFrame;
GtkWidget* preampTable;

GtkWidget* buttonOn;

/* --------------------------------------------------------------------------*/
/** 
* @brief Preamp button Callback 
* 
* @param widget -- pointer to the parent widget, 
* @param data -- pointer to the data.
*/
void preampButtonCallback(GtkWidget* widget,gpointer data) {
    GtkWidget* label;
    char c[80];
    gboolean state;

    if(preamp) {
        state=0;
    } else {
        state=1;
    }    
//    setPreamp(state);

    label=gtk_bin_get_child((GtkBin*)widget);
    if(state) {
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &buttonSelected);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &buttonSelected);
    } else {
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &black);
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Build Preamp User Interface 
* 
* @return GtkWidget pointer 
*/
GtkWidget* buildPreampUI() {

    GtkWidget* label;


    preampFrame=gtk_frame_new("Preamp");
    gtk_widget_modify_bg(preampFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(preampFrame)),GTK_STATE_NORMAL,&white);

    preampTable=gtk_table_new(1,4,TRUE);

    // preamp settings
    buttonOn = gtk_button_new_with_label ("On");
    gtk_widget_modify_bg(buttonOn, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonOn);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonOn),50,25);
    g_signal_connect(G_OBJECT(buttonOn),"clicked",G_CALLBACK(preampButtonCallback),NULL);
    gtk_widget_show(buttonOn);
    gtk_table_attach_defaults(GTK_TABLE(preampTable),buttonOn,0,1,0,1);

    gtk_container_add(GTK_CONTAINER(preampFrame),preampTable);
    gtk_widget_show(preampTable);
    gtk_widget_show(preampFrame);

    return preampFrame;

}

void forcePreamp(int state) {
    GtkWidget* label;
    label=gtk_bin_get_child((GtkBin*)buttonOn);
    if(state) {
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &buttonSelected);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &buttonSelected);
    } else {
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &black);
    }
}
