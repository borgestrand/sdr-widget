/** 
* @file display.c
* @brief Display functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-11
*/
// display.c

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


//
// GTK+ 2.0 implementation of Beppe's Display and Decode panel
// see http://www.radioamatore.it/sdr1000/mypowersdr.html fo the original

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "command.h"
#include "filter.h"
#include "main.h"
#include "property.h"
#include "spectrum.h"
#include "spectrum_update.h"

//GtkWidget* display;
GtkWidget* displayFixed;

GtkWidget* spectrum;

GtkWidget* buttonNone;
GtkWidget* buttonSpectrum;
GtkWidget* buttonPanadapter;
GtkWidget* buttonPanWater;
GtkWidget* buttonScope;
GtkWidget* buttonPhase;
GtkWidget* buttonPhase2;

GtkWidget* currentSpectrumButton;

/* --------------------------------------------------------------------------*/
/** 
* @brief  Callback when a spectrum button is pressed
* 
* @param widget
* @param data
*/
void spectrumCallback(GtkWidget* widget,gpointer data) {
    GtkWidget* label;
    if(currentSpectrumButton) {
        label=gtk_bin_get_child((GtkBin*)currentSpectrumButton);
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &black);
    }
    label=gtk_bin_get_child((GtkBin*)widget);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &buttonSelected);
    gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &buttonSelected);
    currentSpectrumButton=widget;

    if(widget==buttonNone) {
        setSpectrumMode(spectrumNONE);
    } else if(widget==buttonSpectrum) {
        setSpectrumMode(spectrumSPECTRUM);
    } else if(widget==buttonPanadapter) {
        setSpectrumMode(spectrumPANADAPTER);
    } else if(widget==buttonPanWater) {
        setSpectrumMode(spectrumPANWATER);
    } else if(widget==buttonScope) {
        setSpectrumMode(spectrumSCOPE);
    } else if(widget==buttonPhase) {
        setSpectrumMode(spectrumPHASE);
    } else if(widget==buttonPhase2) {
        setSpectrumMode(spectrumPHASE2);
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Build the GUI
* 
* @return 
*/
GtkWidget* buildDisplayUI() {
    GtkWidget* label;

    //display = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    //gtk_widget_modify_bg(display,GTK_STATE_NORMAL,&background);
    //gtk_window_set_title(GTK_WINDOW(display),"Display and Decode");

    displayFixed=gtk_fixed_new();
    gtk_widget_modify_bg(displayFixed,GTK_STATE_NORMAL,&background);

    spectrum=newSpectrumDisplay(spectrumWIDTH);

    gtk_fixed_put((GtkFixed*)displayFixed,spectrum,0,0);

    buttonNone=gtk_button_new_with_label("NONE");
    gtk_widget_modify_bg(buttonNone, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonNone);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonNone),100,25);
    g_signal_connect(G_OBJECT(buttonNone),"clicked",G_CALLBACK(spectrumCallback),NULL);
    gtk_widget_show(buttonNone);
    gtk_fixed_put((GtkFixed*)displayFixed,buttonNone,0,spectrumHEIGHT);

    buttonSpectrum=gtk_button_new_with_label("SPECTRUM");
    gtk_widget_modify_bg(buttonSpectrum, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonSpectrum);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonSpectrum),100,25);
    g_signal_connect(G_OBJECT(buttonSpectrum),"clicked",G_CALLBACK(spectrumCallback),NULL);
    gtk_widget_show(buttonSpectrum);
    gtk_fixed_put((GtkFixed*)displayFixed,buttonSpectrum,100,spectrumHEIGHT);

    buttonPanadapter=gtk_button_new_with_label("PANADAPTER");
    gtk_widget_modify_bg(buttonPanadapter, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonPanadapter);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonPanadapter),100,25);
    g_signal_connect(G_OBJECT(buttonPanadapter),"clicked",G_CALLBACK(spectrumCallback),NULL);
    gtk_widget_show(buttonPanadapter);
    gtk_fixed_put((GtkFixed*)displayFixed,buttonPanadapter,200,spectrumHEIGHT);

    buttonPanWater=gtk_button_new_with_label("PAN/WATER");
    gtk_widget_modify_bg(buttonPanWater, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonPanWater);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonPanWater),100,25);
    g_signal_connect(G_OBJECT(buttonPanWater),"clicked",G_CALLBACK(spectrumCallback),NULL);
    gtk_widget_show(buttonPanWater);
    gtk_fixed_put((GtkFixed*)displayFixed,buttonPanWater,300,spectrumHEIGHT);

    buttonScope=gtk_button_new_with_label("SCOPE");
    gtk_widget_modify_bg(buttonScope, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonScope);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonScope),100,25);
    g_signal_connect(G_OBJECT(buttonScope),"clicked",G_CALLBACK(spectrumCallback),NULL);
    gtk_widget_show(buttonScope);
    gtk_fixed_put((GtkFixed*)displayFixed,buttonScope,400,spectrumHEIGHT);

    buttonPhase=gtk_button_new_with_label("PHASE");
    gtk_widget_modify_bg(buttonPhase, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonPhase);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonPhase),100,25);
    g_signal_connect(G_OBJECT(buttonPhase),"clicked",G_CALLBACK(spectrumCallback),NULL);
    gtk_widget_show(buttonPhase);
    gtk_fixed_put((GtkFixed*)displayFixed,buttonPhase,500,spectrumHEIGHT);

    buttonPhase2=gtk_button_new_with_label("PHASE2");
    gtk_widget_modify_bg(buttonPhase2, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonPhase2);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonPhase2),100,25);
    g_signal_connect(G_OBJECT(buttonPhase2),"clicked",G_CALLBACK(spectrumCallback),NULL);
    gtk_widget_show(buttonPhase2);
    gtk_fixed_put((GtkFixed*)displayFixed,buttonPhase2,600,spectrumHEIGHT);

    gtk_widget_set_size_request(GTK_WIDGET(displayFixed),spectrumWIDTH,spectrumHEIGHT+25);
    gtk_widget_show(displayFixed);

    //gtk_container_add(GTK_CONTAINER(display), displayFixed);
    //gtk_window_set_position((GtkWindow*)display,GTK_WIN_POS_MOUSE);

    //gtk_widget_show(display.g);

    currentSpectrumButton=NULL;

    switch(spectrumMode) {
        case spectrumNONE:
            spectrumCallback(buttonNone,NULL);
            break;
        case spectrumSPECTRUM:
            spectrumCallback(buttonSpectrum,NULL);
            break;
        case spectrumPANADAPTER:
            spectrumCallback(buttonPanadapter,NULL);
            break;
        case spectrumPANWATER:
            spectrumCallback(buttonPanWater,NULL);
            break;
        case spectrumSCOPE:
            spectrumCallback(buttonScope,NULL);
            break;
        case spectrumPHASE:
            spectrumCallback(buttonPhase,NULL);
            break;
        case spectrumPHASE2:
            spectrumCallback(buttonPhase2,NULL);
            break;
    }

    return displayFixed;
  
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Save the display state. 
*/
void displaySaveState() {
    char string[128];

    sprintf(string,"%d",spectrumUpdatesPerSecond);
    setProperty("spectrum.updates.per.second",string);

    sprintf(string,"%d",spectrumMode);
    setProperty("spectrum.mode",string);

    spectrumSaveState();

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Restore the display state.
*/
void displayRestoreState() {
    char* value;

    value=getProperty("spectrum.updates.per.second");
    spectrumUpdatesPerSecond=SPECTRUM_UPDATES_PER_SECOND;
    if(value) spectrumUpdatesPerSecond=atoi(value);

    value=getProperty("spectrum.mode");
    spectrumMode=spectrumPANWATER;
    if(value) spectrumMode=atoi(value);

    spectrumRestoreState();
}
