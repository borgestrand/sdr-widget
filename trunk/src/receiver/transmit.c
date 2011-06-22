/** 
* @file transmit.c
* @brief Tranmsit files for GHPSDR
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
#include "transmit.h"
#include "main.h"
#include "property.h"
#include "ozy.h"
#include "mode.h"
#include "filter.h"
#include "vfo.h"
#include "volume.h"

GtkWidget* transmitFrame;
GtkWidget* transmitTable;

GtkWidget* buttonMOX;
GtkWidget* buttonTune;
GtkWidget* rfGainFrame;
GtkWidget* rfGainScale;
GtkWidget* micGainFrame;
GtkWidget* micGainScale;

double rfGain=0.1;
double micGain=1.0;

int tuning=0;
double tuningPhase=0;

int fullDuplex=1;

/* --------------------------------------------------------------------------*/
/** 
* @brief MOX button Callback 
* 
* @param widget -- pointer to the parent widget, 
* @param data -- pointer to the data.
*/
void moxButtonCallback(GtkWidget* widget,gpointer data) {
    GtkWidget* label;
    char c[80];
    int state;

    if(mox) {
        state=0;
    } else {
        state=1;
    }    

    //vfoTransmit(state);
    int *vfoState=malloc(sizeof(int));
    *vfoState=state;
    g_idle_add(vfoTransmit,(gpointer)vfoState);

    //setMOX(state);

    if(!fullDuplex) {
        if(mox) {
            SetThreadProcessingMode(0,0);  // MUTE
            //SetRXOutputGain(0,0,0.0);
        } else {
            SetThreadProcessingMode(0,2);  // RUN
            //SetRXOutputGain(0,0,volume/100.0);
        }
    }

    label=gtk_bin_get_child((GtkBin*)widget);
    if(state) {
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &red);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &red);
    } else {
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &black);
    }
}

/* --------------------------------------------------------------------------*/
/**
* @brief Tune button Callback
*
* @param widget -- pointer to the parent widget,
* @param data -- pointer to the data.
*/
void tuneButtonCallback(GtkWidget* widget,gpointer data) {
    GtkWidget* label;
    char c[80];
    gboolean state;

    if(mox) {
        state=0;
    } else {
        state=1;
    }

    //vfoTransmit(state);
    int *vfoState=malloc(sizeof(int));
    *vfoState=state;
    g_idle_add(vfoTransmit,(gpointer)vfoState);
    //setMOX(state);
    tuning=state;
    tuningPhase=0.0;
    if(state) {
        switch(mode) {
            case modeLSB:
            case modeCWL:
            case modeDIGL:
                SetTXFilter(1,(double)(-cwPitch-100),(double)(-cwPitch+100));
                break;
            case modeUSB:
            case modeCWU:
            case modeDIGU:
                SetTXFilter(1,(double)(cwPitch-100),(double)(cwPitch+100));
                break;
        }
    } else {
        SetTXFilter(1,(double)filterLow,(double)filterHigh);
    }

    if(!fullDuplex) {
        if(mox) {
            SetThreadProcessingMode(0,0);  // MUTE
            //SetRXOutputGain(0,0,0.0);
        } else {
            SetThreadProcessingMode(0,2);  // RUN
            //SetRXOutputGain(0,0,volume/100.0);
        }
    }

    label=gtk_bin_get_child((GtkBin*)widget);
    if(state) {
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &red);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &red);
    } else {
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &black);
    }
}

/* --------------------------------------------------------------------------*/
/**
* @brief  Callback when rf gain values changes
*
* @param widget
* @param data
*/
void rfGainChanged(GtkWidget* widget,gpointer data) {
    rfGain=gtk_range_get_value((GtkRange*)rfGainScale);
}

/* --------------------------------------------------------------------------*/
/**
* @brief  Callback when mic gain values changes
*
* @param widget
* @param data
*/
void micGainChanged(GtkWidget* widget,gpointer data) {
    micGain=gtk_range_get_value((GtkRange*)micGainScale);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Build Transmit User Interface 
* 
* @return GtkWidget pointer 
*/
GtkWidget* buildTransmitUI() {

    GtkWidget* label;

    transmitFrame=gtk_frame_new("Transmit");
    gtk_widget_modify_bg(transmitFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(transmitFrame)),GTK_STATE_NORMAL,&white);

    transmitTable=gtk_table_new(1,8,TRUE);

    // transmit settings
    buttonMOX = gtk_button_new_with_label ("MOX");
    gtk_widget_modify_bg(buttonMOX, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonMOX);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonMOX),50,25);
    g_signal_connect(G_OBJECT(buttonMOX),"clicked",G_CALLBACK(moxButtonCallback),NULL);
    gtk_widget_show(buttonMOX);
    gtk_table_attach_defaults(GTK_TABLE(transmitTable),buttonMOX,0,1,0,1);

    // tune settings
    buttonTune = gtk_button_new_with_label ("Tune");
    gtk_widget_modify_bg(buttonTune, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonTune);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonTune),50,25);
    g_signal_connect(G_OBJECT(buttonTune),"clicked",G_CALLBACK(tuneButtonCallback),NULL);
    gtk_widget_show(buttonTune);
    gtk_table_attach_defaults(GTK_TABLE(transmitTable),buttonTune,1,2,0,1);

    // rf gain

    rfGainFrame=gtk_frame_new("Power");
    gtk_widget_modify_bg(rfGainFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(rfGainFrame)),GTK_STATE_NORMAL,&white);

    rfGainScale=gtk_hscale_new_with_range(0.1,0.5,0.01);

    g_signal_connect(G_OBJECT(rfGainScale),"value-changed",G_CALLBACK(rfGainChanged),NULL);
    gtk_range_set_value((GtkRange*)rfGainScale,rfGain);
    gtk_widget_set_size_request(GTK_WIDGET(rfGainScale),150,30);
    gtk_widget_show(rfGainScale);
    gtk_container_add(GTK_CONTAINER(rfGainFrame),rfGainScale);
    gtk_widget_show(rfGainFrame);
    gtk_table_attach_defaults(GTK_TABLE(transmitTable),rfGainFrame,2,5,0,1);

    // mic gain

    micGainFrame=gtk_frame_new("Mic Gain");
    gtk_widget_modify_bg(micGainFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(micGainFrame)),GTK_STATE_NORMAL,&white);

    micGainScale=gtk_hscale_new_with_range(0.01,1.0,0.01);

    g_signal_connect(G_OBJECT(micGainScale),"value-changed",G_CALLBACK(micGainChanged),NULL);
    gtk_range_set_value((GtkRange*)micGainScale,micGain);
    gtk_widget_set_size_request(GTK_WIDGET(micGainScale),150,30);
    gtk_widget_show(micGainScale);
    gtk_container_add(GTK_CONTAINER(micGainFrame),micGainScale);
    gtk_widget_show(micGainFrame);
    gtk_table_attach_defaults(GTK_TABLE(transmitTable),micGainFrame,5,8,0,1);

    gtk_container_add(GTK_CONTAINER(transmitFrame),transmitTable);
    gtk_widget_show(transmitTable);
    gtk_widget_show(transmitFrame);

    return transmitFrame;

}

/* --------------------------------------------------------------------------*/
/**
* @brief Save the transmit state
*/
void transmitSaveState() {
    char string[128];
    sprintf(string,"%f",rfGain);
    setProperty("rfGain",string);
    sprintf(string,"%f",micGain);
    setProperty("micGain",string);
    sprintf(string,"%d",fullDuplex);
    setProperty("fullDuplex",string);
}

/* --------------------------------------------------------------------------*/
/**
* @brief Restore the volume state
*/
void transmitRestoreState() {
    char* value;
    value=getProperty("rfGain");
    if(value) rfGain=atof(value); else rfGain=3.0f;
    value=getProperty("micGain");
    if(value) micGain=atof(value); else micGain=1.0f;
    value=getProperty("fullDuplex");
    if(value) fullDuplex=atoi(value); else fullDuplex=0;
}
