/** 
* @file audio.c
* @brief Audio files for GHPSDR
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
#include "audio.h"
#include "main.h"
#include "property.h"

GtkWidget* audioFrame;
GtkWidget* audioTable;

GtkWidget* buttonNR;
GtkWidget* buttonANF;
GtkWidget* buttonNB;
GtkWidget* buttonNB2;
GtkWidget* buttonMUTE;
GtkWidget* buttonBIN;

gboolean NR=FALSE;
gboolean ANF=FALSE;
gboolean NB=FALSE;
gboolean NB2=FALSE;
gboolean MUTE=FALSE;
gboolean BIN=FALSE;

/* --------------------------------------------------------------------------*/
/** 
* @brief Audio button Callback 
* 
* @param widget -- pointer to the parent widget, 
* @param data -- pointer to the data.
*/
void audioButtonCallback(GtkWidget* widget,gpointer data) {
    GtkWidget* label;
    char c[80];
    gboolean state;

    if(widget==buttonNR) {
        NR=!NR;
        state=NR;
        //writeCommand("setBlkANF 1");
        SetANF(0,0,1);
        //sprintf(c,"setNR %d",NR);
        //writeCommand(c);
	SetNRvals(0,0,128,50,1.0,1.0);
        SetNR(0,0,NR);
    } else if(widget==buttonANF) {
        ANF=!ANF;
        state=ANF;
        //writeCommand("setBlkANF 1");
	SetANFvals(0,0,128,50,0.1,0.0);
        SetANF(0,0,1);
        //sprintf(c,"setANF %d",ANF);
        //writeCommand(c);
        SetANF(0,0,ANF);
    } else if(widget==buttonNB) {
        NB=!NB;
        state=NB;
        //sprintf(c,"setNB %d",NB);
        //writeCommand(c);
        SetNB(0,0,NB);
    } else if(widget==buttonNB2) {
        NB2=!NB2;
        state=NB2;
        //sprintf(c,"setNB2 %d",NB2);
        //writeCommand(c);
        SetNB(0,0,NB2);
    } else if(widget==buttonMUTE) {
        MUTE=!MUTE;
        state=MUTE;
    } else if(widget==buttonBIN) {
        BIN=!BIN;
        state=BIN;
        SetBIN(0,0,BIN);
    }
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
* @brief Build Audio User Interface 
* 
* @return GtkWidget pointer 
*/
GtkWidget* buildAudioUI() {

    GtkWidget* label;

    audioFrame=gtk_frame_new("DSP");
    gtk_widget_modify_bg(audioFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(audioFrame)),GTK_STATE_NORMAL,&white);

    audioTable=gtk_table_new(2,4,TRUE);

    // audio settings
    buttonNR = gtk_button_new_with_label ("NR");
    gtk_widget_modify_bg(buttonNR, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonNR);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonNR),50,25);
    g_signal_connect(G_OBJECT(buttonNR),"clicked",G_CALLBACK(audioButtonCallback),NULL);
    gtk_widget_show(buttonNR);
    gtk_table_attach_defaults(GTK_TABLE(audioTable),buttonNR,0,1,0,1);

    buttonANF = gtk_button_new_with_label ("ANF");
    gtk_widget_modify_bg(buttonANF, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonANF);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonANF),50,25);
    g_signal_connect(G_OBJECT(buttonANF),"clicked",G_CALLBACK(audioButtonCallback),NULL);
    gtk_widget_show(buttonANF);
    gtk_table_attach_defaults(GTK_TABLE(audioTable),buttonANF,1,2,0,1);

    buttonNB = gtk_button_new_with_label ("NB");
    gtk_widget_modify_bg(buttonNB, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonNB);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonNB),50,25);
    g_signal_connect(G_OBJECT(buttonNB),"clicked",G_CALLBACK(audioButtonCallback),NULL);
    gtk_widget_show(buttonNB);
    gtk_table_attach_defaults(GTK_TABLE(audioTable),buttonNB,2,3,0,1);

    buttonNB2 = gtk_button_new_with_label ("NB2");
    gtk_widget_modify_bg(buttonNB2, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonNB2);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonNB2),50,25);
    g_signal_connect(G_OBJECT(buttonNB2),"clicked",G_CALLBACK(audioButtonCallback),NULL);
    gtk_widget_show(buttonNB2);
    gtk_table_attach_defaults(GTK_TABLE(audioTable),buttonNB2,3,4,0,1);

    buttonMUTE = gtk_button_new_with_label ("MUTE");
    gtk_widget_modify_bg(buttonMUTE, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonMUTE);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonMUTE),50,25);
    g_signal_connect(G_OBJECT(buttonMUTE),"clicked",G_CALLBACK(audioButtonCallback),NULL);
    gtk_widget_show(buttonMUTE);
    gtk_table_attach_defaults(GTK_TABLE(audioTable),buttonMUTE,0,1,1,2);

    buttonBIN = gtk_button_new_with_label ("BIN");
    gtk_widget_modify_bg(buttonBIN, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBIN);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBIN),50,25);
    g_signal_connect(G_OBJECT(buttonBIN),"clicked",G_CALLBACK(audioButtonCallback),NULL);
    gtk_widget_show(buttonBIN);
    gtk_table_attach_defaults(GTK_TABLE(audioTable),buttonBIN,1,2,1,2);

    gtk_container_add(GTK_CONTAINER(audioFrame),audioTable);
    gtk_widget_show(audioTable);
    gtk_widget_show(audioFrame);

    return audioFrame;

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Save the Audio state. 
*/
void audioSaveState() {
    char string[128];

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Restore the Audio state.
*/
void audioRestoreState() {
    char* value;
}

