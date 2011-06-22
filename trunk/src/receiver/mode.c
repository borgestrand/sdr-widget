/** 
* @file mode.c
* @brief Mode functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
*/
// mode.c

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

#include "bandstack.h"
#include "command.h"
#include "cw.h"
#include "filter.h"
#include "main.h"
#include "mode.h"
#include "property.h"
#include "soundcard.h"

GtkWidget* modeFrame;
GtkWidget* modeTable;

int mode;
char stringMode[8];

GtkWidget* buttonLSB;
GtkWidget* buttonUSB;
GtkWidget* buttonDSB;
GtkWidget* buttonCWL;
GtkWidget* buttonCWU;
GtkWidget* buttonAM;
GtkWidget* buttonSAM;
GtkWidget* buttonFMN;
GtkWidget* buttonDIGU;
GtkWidget* buttonSPEC;
GtkWidget* buttonDIGL;
GtkWidget* buttonDRM;

GtkWidget* currentModeButton;

int setMode(int *mode);

/* --------------------------------------------------------------------------*/
/** 
* @brief  Select the mode - sends a setMode and setFilter to DSP
* 
* @param widget
*/
void selectMode(GtkWidget* widget) {
    GtkWidget* label;
    char temp[80];

    if(currentModeButton) {
        label=gtk_bin_get_child((GtkBin*)currentModeButton);
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &black);
    }
    if(widget) {
        label=gtk_bin_get_child((GtkBin*)widget);
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &buttonSelected);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &buttonSelected);
        currentModeButton=widget;
    }

    if(widget==buttonLSB) {
        mode=modeLSB;
    } else if(widget==buttonUSB) {
        mode=modeUSB;
    } else if(widget==buttonDSB) {
        mode=modeDSB;
    } else if(widget==buttonCWL) {
        mode=modeCWL;
    } else if(widget==buttonCWU) {
        mode=modeCWU;
    } else if(widget==buttonAM) {
        mode=modeAM;
    } else if(widget==buttonSAM) {
        mode=modeSAM;
    } else if(widget==buttonFMN) {
        mode=modeFMN;
    } else if(widget==buttonDIGL) {
        mode=modeDIGL;
    } else if(widget==buttonSPEC) {
        mode=modeSPEC;
    } else if(widget==buttonDIGU) {
        mode=modeDIGU;
    } else if(widget==buttonDRM) {
        mode=modeDRM;
    }


    // set RX mode
    //sprintf(temp,"setMode %d",mode);
    //writeCommand(temp);
    SetMode(0,0,mode);
    SetMode(0,1,mode);
    SetMode(1,0,mode);

    setFilterValues(mode);
    setFilter(filter);

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the mode of the mode function
* 
* @param mode
*/
int setMode(int *mode) {
    GtkWidget* widget;
    switch(*mode) {
        case modeLSB:
            widget=buttonLSB;
            break;
        case modeUSB:
            widget=buttonUSB;
            break;
        case modeDSB:
            widget=buttonDSB;
            break;
        case modeCWL:
            widget=buttonCWL;
            break;
        case modeCWU:
            widget=buttonCWU;
            break;
        case modeAM:
            widget=buttonAM;
            break;
        case modeSAM:
            widget=buttonSAM;
            break;
        case modeFMN:
            widget=buttonFMN;
            break;
        case modeDIGL:
            widget=buttonDIGL;
            break;
        case modeSPEC:
            widget=buttonSPEC;
            break;
        case modeDIGU:
            widget=buttonDIGU;
            break;
        case modeDRM:
            widget=buttonDRM;
            break;
    }
    selectMode(widget);
    free(mode);
    return FALSE;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief  Callback when a mode button is pressed
* 
* @param widget
* @param data
*/
void modeCallback(GtkWidget* widget,gpointer data) {
    selectMode(widget);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief  Build the GUI
* 
* @return 
*/
GtkWidget* buildModeUI() {
    GtkWidget* label;


    modeFrame=gtk_frame_new("Mode");
    gtk_widget_modify_bg(modeFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(modeFrame)),GTK_STATE_NORMAL,&white);

    modeTable=gtk_table_new(3,4,TRUE);

    // mode buttons
    buttonLSB = gtk_button_new_with_label ("LSB");
    gtk_widget_modify_bg(buttonLSB, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonLSB);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonLSB),50,25);
    g_signal_connect(G_OBJECT(buttonLSB),"clicked",G_CALLBACK(modeCallback),NULL);
    gtk_widget_show(buttonLSB);
    gtk_table_attach_defaults(GTK_TABLE(modeTable),buttonLSB,0,1,0,1);

    buttonUSB = gtk_button_new_with_label ("USB");
    gtk_widget_modify_bg(buttonUSB, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonUSB);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonUSB),50,25);
    g_signal_connect(G_OBJECT(buttonUSB),"clicked",G_CALLBACK(modeCallback),NULL);
    gtk_widget_show(buttonUSB);
    gtk_table_attach_defaults(GTK_TABLE(modeTable),buttonUSB,0,1,1,2);

    buttonCWL = gtk_button_new_with_label ("CW/L");
    gtk_widget_modify_bg(buttonCWL, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonCWL);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonCWL),50,25);
    g_signal_connect(G_OBJECT(buttonCWL),"clicked",G_CALLBACK(modeCallback),NULL);
    gtk_widget_show(buttonCWL);
    gtk_table_attach_defaults(GTK_TABLE(modeTable),buttonCWL,1,2,0,1);

    buttonCWU = gtk_button_new_with_label ("CW/U");
    gtk_widget_modify_bg(buttonCWU, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonCWU);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonCWU),50,25);
    g_signal_connect(G_OBJECT(buttonCWU),"clicked",G_CALLBACK(modeCallback),NULL);
    gtk_widget_show(buttonCWU);
    gtk_table_attach_defaults(GTK_TABLE(modeTable),buttonCWU,1,2,1,2);

    buttonDSB = gtk_button_new_with_label ("DSB");
    gtk_widget_modify_bg(buttonDSB, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonDSB);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonDSB),50,25);
    g_signal_connect(G_OBJECT(buttonDSB),"clicked",G_CALLBACK(modeCallback),NULL);
    gtk_widget_show(buttonDSB);
    gtk_table_attach_defaults(GTK_TABLE(modeTable),buttonDSB,0,1,2,3);


    buttonFMN = gtk_button_new_with_label ("FM/N");
    gtk_widget_modify_bg(buttonFMN, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonFMN);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonFMN),50,25);
    g_signal_connect(G_OBJECT(buttonFMN),"clicked",G_CALLBACK(modeCallback),NULL);
    gtk_widget_show(buttonFMN);
    gtk_table_attach_defaults(GTK_TABLE(modeTable),buttonFMN,3,4,2,3);

    buttonAM = gtk_button_new_with_label ("AM");
    gtk_widget_modify_bg(buttonAM, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonAM);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonAM),50,25);
    g_signal_connect(G_OBJECT(buttonAM),"clicked",G_CALLBACK(modeCallback),NULL);
    gtk_widget_show(buttonAM);
    gtk_table_attach_defaults(GTK_TABLE(modeTable),buttonAM,3,4,0,1);

    buttonSAM = gtk_button_new_with_label ("SAM");
    gtk_widget_modify_bg(buttonSAM, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonSAM);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonSAM),50,25);
    g_signal_connect(G_OBJECT(buttonSAM),"clicked",G_CALLBACK(modeCallback),NULL);
    gtk_widget_show(buttonSAM);
    gtk_table_attach_defaults(GTK_TABLE(modeTable),buttonSAM,3,4,1,2);

    buttonDIGL = gtk_button_new_with_label ("DIGL");
    gtk_widget_modify_bg(buttonDIGL, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonDIGL);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonDIGL),50,25);
    g_signal_connect(G_OBJECT(buttonDIGL),"clicked",G_CALLBACK(modeCallback),NULL);
    gtk_widget_show(buttonDIGL);
    gtk_table_attach_defaults(GTK_TABLE(modeTable),buttonDIGL,2,3,0,1);

    buttonDIGU = gtk_button_new_with_label ("DIGU");
    gtk_widget_modify_bg(buttonDIGU, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonDIGU);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonDIGU),50,25);
    g_signal_connect(G_OBJECT(buttonDIGU),"clicked",G_CALLBACK(modeCallback),NULL);
    gtk_widget_show(buttonDIGU);
    gtk_table_attach_defaults(GTK_TABLE(modeTable),buttonDIGU,2,3,1,2);

    buttonSPEC = gtk_button_new_with_label ("SPEC");
    gtk_widget_modify_bg(buttonSPEC, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonSPEC);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonSPEC),50,25);
    g_signal_connect(G_OBJECT(buttonSPEC),"clicked",G_CALLBACK(modeCallback),NULL);
    gtk_widget_show(buttonSPEC);
    gtk_table_attach_defaults(GTK_TABLE(modeTable),buttonSPEC,1,2,2,3);

    buttonDRM = gtk_button_new_with_label ("DRM");
    gtk_widget_modify_bg(buttonDRM, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonDRM);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonDRM),50,25);
    g_signal_connect(G_OBJECT(buttonDRM),"clicked",G_CALLBACK(modeCallback),NULL);
    gtk_widget_show(buttonDRM);
    gtk_table_attach_defaults(GTK_TABLE(modeTable),buttonDRM,2,3,2,3);

  
    gtk_container_add(GTK_CONTAINER(modeFrame),modeTable);
    gtk_widget_show(modeTable);
    gtk_widget_show(modeFrame);

    return modeFrame;

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Save the mode state
*/
void modeSaveState() {
    char string[128];
    sprintf(string,"%d",mode);
    setProperty("mode",string);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Restore the mode state
*/
void modeRestoreState() {
    char* value;
    value=getProperty("mode");
    if(value) mode=atoi(value);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief get the mode as a string
*/
char* modeToString() {
    switch(mode) {
        case modeLSB:
            strcpy(stringMode,"LSB");
            break;
        case modeUSB:
            strcpy(stringMode,"USB");
            break;
        case modeDSB:
            strcpy(stringMode,"DSB");
            break;
        case modeCWL:
            strcpy(stringMode,"CWL");
            break;
        case modeCWU:
            strcpy(stringMode,"CWU");
            break;
        case modeAM:
            strcpy(stringMode,"AM");
            break;
        case modeSAM:
            strcpy(stringMode,"SAM");
            break;
        case modeFMN:
            strcpy(stringMode,"FMN");
            break;
        case modeDIGL:
            strcpy(stringMode,"DIGL");
            break;
        case modeSPEC:
            strcpy(stringMode,"SPEC");
            break;
        case modeDIGU:
            strcpy(stringMode,"DIGU");
            break;
        case modeDRM:
            strcpy(stringMode,"DRM");
            break;
    }

    return stringMode;
}
