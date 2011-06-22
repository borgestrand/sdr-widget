/** 
* @file band.c
* @brief Files to define the Amateur Radio Bands.
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-11
*/
// band.c

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

#include "xvtr.h"
#include "band.h"
#include "bandstack.h"
#include "command.h"
#include "cw.h"
#include "filter.h"
#include "main.h"
#include "preamp.h"
#include "property.h"
#include "soundcard.h"
#include "mode.h"
#include "ozy.h"
#include "vfo.h"
#include "spectrum.h"
#include "subrx.h"

GtkWidget* bandFrame;
GtkWidget* bandTable;

int band=band40;
int xvtr_band=band160;

GtkWidget* buttonBand1;
GtkWidget* buttonBand2;
GtkWidget* buttonBand3;
GtkWidget* buttonBand4;
GtkWidget* buttonBand5;
GtkWidget* buttonBand6;
GtkWidget* buttonBand7;
GtkWidget* buttonBand8;
GtkWidget* buttonBand9;
GtkWidget* buttonBand10;
GtkWidget* buttonBand11;
GtkWidget* buttonBand12;
GtkWidget* buttonBand13;
GtkWidget* buttonBand14;

GtkWidget* currentBandButton;

GtkWidget* currentHFButton;
GtkWidget* currentXVTRButton;

gboolean displayHF=TRUE;

/* --------------------------------------------------------------------------*/
/** 
* @brief bandstack
*/
/* ----------------------------------------------------------------------------*/
BANDSTACK_ENTRY bandstack160[] =
    {{1810000LL,modeCWL,filterF4,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {1835000LL,modeCWU,filterF0,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {1845000LL,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125}};

BANDSTACK_ENTRY bandstack80[] =
    {{3501000LL,modeCWL,filterF0,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {3751000LL,modeLSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {3850000LL,modeLSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125}};

BANDSTACK_ENTRY bandstack60[] =
    {{5330500LL,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {5346500LL,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {5366500LL,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {5371500LL,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {5403500LL,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125}};

BANDSTACK_ENTRY bandstack40[] =
    {{7001000LL,modeCWL,filterF0,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {7152000LL,modeLSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {7255000LL,modeLSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125}};

BANDSTACK_ENTRY bandstack30[] =
    {{10120000LL,modeCWU,filterF0,200,2800,200,2800,100,0,-40,-160,10,-40,-125},
     {10130000LL,modeCWU,filterF0,200,2800,200,2800,100,0,-40,-160,10,-40,-125},
     {10140000LL,modeCWU,filterF4,200,2800,200,2800,100,0,-40,-160,10,-40,-125}};

BANDSTACK_ENTRY bandstack20[] =
    {{14010000LL,modeCWU,filterF0,200,2800,200,2800,100,0,-40,-160,10,-40,-125},
     {14230000LL,modeUSB,filterF5,200,2800,200,2800,100,0,-40,-160,10,-100,-125},
     {14336000LL,modeUSB,filterF5,200,2800,200,2800,100,0,-40,-160,10,-40,-125}};

BANDSTACK_ENTRY bandstack18[] =
    {{18068600LL,modeCWU,filterF0,200,2800,200,2800,100,0,-40,-160,10,-40,-125},
     {18125000LL,modeUSB,filterF5,200,2800,200,2800,100,0,-40,-160,10,-40,-125},
     {18140000LL,modeUSB,filterF5,200,2800,200,2800,100,0,-40,-160,10,-40,-125}};

BANDSTACK_ENTRY bandstack15[] =
    {{21001000LL,modeCWU,filterF0,200,2800,200,2800,100,0,-40,-160,10,-40,-125},
     {21255000LL,modeUSB,filterF5,200,2800,200,2800,100,0,-40,-160,10,-40,-125},
     {21300000LL,modeUSB,filterF5,200,2800,200,2800,100,0,-40,-160,10,-40,-125}};

BANDSTACK_ENTRY bandstack12[] =
    {{24895000LL,modeCWU,filterF0,200,2800,200,2800,100,0,-40,-160,10,-40,-125},
     {24900000LL,modeUSB,filterF5,200,2800,200,2800,100,0,-40,-160,10,-40,-125},
     {24910000LL,modeUSB,filterF5,200,2800,200,2800,100,0,-40,-160,10,-40,-125}};

BANDSTACK_ENTRY bandstack10[] =
    {{28010000LL,modeCWU,filterF0,200,2800,200,2800,100,0,-40,-160,10,-40,-125},
     {28300000LL,modeUSB,filterF5,200,2800,200,2800,100,0,-40,-160,10,-40,-125},
     {28400000LL,modeUSB,filterF5,200,2800,200,2800,100,0,-40,-160,10,-40,-125}};

BANDSTACK_ENTRY bandstack50[] =
    {{50010000LL,modeCWU,filterF0,200,2800,200,2800,100,0,-40,-160,10,-40,-125},
     {50125000LL,modeUSB,filterF5,200,2800,200,2800,100,0,-40,-160,10,-40,-150},
     {50200000LL,modeUSB,filterF5,200,2800,200,2800,100,0,-40,-160,10,-40,-125}};

BANDSTACK_ENTRY bandstackGEN[] =
    {{909000LL,modeAM,filterF6,-6000,6000,-6000,6000,1000,0,-40,-160,10,-40,-125},
     {5975000LL,modeAM,filterF6,-6000,6000,-6000,6000,1000,0,-40,-160,10,-40,-125},
     {13845000LL,modeAM,filterF6,-6000,6000,-6000,6000,1000,0,-40,-160,10,-40,-125}};

BANDSTACK_ENTRY bandstackWWV[] =
    {{2500000LL,modeSAM,filterF6,200,2800,200,2800,1000,0,-40,-160,10,-40,-125},
     {5000000LL,modeSAM,filterF6,200,2800,200,2800,1000,0,-40,-160,10,-40,-125},
     {10000000LL,modeSAM,filterF6,200,2800,200,2800,1000,0,-40,-160,10,-40,-125},
     {15000000LL,modeSAM,filterF6,200,2800,200,2800,1000,0,-40,-160,10,-40,-125},
     {20000000LL,modeSAM,filterF6,200,2800,200,2800,1000,0,-40,-160,10,-40,-125}};

BANDSTACK bandstack[BANDS];

#define NUM_BAND_LIMITS 22

BAND_LIMITS bandLimits[NUM_BAND_LIMITS] = {
    {1800000LL,2000000LL},
    {3500000LL,4000000LL},
    {5330500LL,5403500LL},
    {7000000LL,7300000LL},
    {10100000LL,10150000LL},
    {14000000LL,14350000LL},
    {18068000LL,18168000LL},
    {21000000LL,21450000LL},
    {24890000LL,24990000LL},
    {28000000LL,29700000LL},
    {50000000LL,54000000LL},
    {144000000LL,148000000LL},
    {222000000LL,224980000LL},
    {420000000LL,450000000LL},
    {902000000LL,928000000LL},
    {1240000000LL,1300000000LL},
    {2300000000LL,2450000000LL},
    {3456000000LL,3456400000LL},
    {5760000000LL,5760400000LL},
    {10368000000LL,10368400000LL},
    {24192000000LL,24192400000LL},
    {47088000000LL,47088400000LL}
};

/* --------------------------------------------------------------------------*/
/** 
* @brief xvtr
*/
/* ----------------------------------------------------------------------------*/
XVTR_ENTRY xvtr[12]=
    {{"",0LL,0LL,0LL,0LL,0LL,0LL,0LL,0LL,0,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {"",0LL,0LL,0LL,0LL,0LL,0LL,0LL,0LL,0,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {"",0LL,0LL,0LL,0LL,0LL,0LL,0LL,0LL,0,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {"",0LL,0LL,0LL,0LL,0LL,0LL,0LL,0LL,0,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {"",0LL,0LL,0LL,0LL,0LL,0LL,0LL,0LL,0,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {"",0LL,0LL,0LL,0LL,0LL,0LL,0LL,0LL,0,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {"",0LL,0LL,0LL,0LL,0LL,0LL,0LL,0LL,0,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {"",0LL,0LL,0LL,0LL,0LL,0LL,0LL,0LL,0,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {"",0LL,0LL,0LL,0LL,0LL,0LL,0LL,0LL,0,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {"",0LL,0LL,0LL,0LL,0LL,0LL,0LL,0LL,0,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {"",0LL,0LL,0LL,0LL,0LL,0LL,0LL,0LL,0,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125},
     {"",0LL,0LL,0LL,0LL,0LL,0LL,0LL,0LL,0,modeUSB,filterF5,-2800,-200,-2800,-200,100,0,-40,-160,10,-40,-125}};

void setTuningMode(int mode);
void setBand(int band);
void setIncrement(int increment);

void setHFTitles() {
    gtk_button_set_label((GtkButton*)buttonBand1,"160");
    gtk_widget_set_sensitive(buttonBand1,TRUE);
    gtk_button_set_label((GtkButton*)buttonBand2,"80");
    gtk_widget_set_sensitive(buttonBand2,TRUE);
    gtk_button_set_label((GtkButton*)buttonBand3,"60");
    gtk_widget_set_sensitive(buttonBand3,TRUE);
    gtk_button_set_label((GtkButton*)buttonBand4,"40");
    gtk_widget_set_sensitive(buttonBand4,TRUE);
    gtk_button_set_label((GtkButton*)buttonBand5,"30");
    gtk_widget_set_sensitive(buttonBand5,TRUE);
    gtk_button_set_label((GtkButton*)buttonBand6,"20");
    gtk_widget_set_sensitive(buttonBand6,TRUE);
    gtk_button_set_label((GtkButton*)buttonBand7,"17");
    gtk_widget_set_sensitive(buttonBand7,TRUE);
    gtk_button_set_label((GtkButton*)buttonBand8,"15");
    gtk_widget_set_sensitive(buttonBand8,TRUE);
    gtk_button_set_label((GtkButton*)buttonBand9,"12");
    gtk_widget_set_sensitive(buttonBand9,TRUE);
    gtk_button_set_label((GtkButton*)buttonBand10,"10");
    gtk_widget_set_sensitive(buttonBand10,TRUE);
    gtk_button_set_label((GtkButton*)buttonBand11,"6");
    gtk_widget_set_sensitive(buttonBand11,TRUE);
    gtk_button_set_label((GtkButton*)buttonBand12,"GEN");
    gtk_widget_set_sensitive(buttonBand12,TRUE);
    gtk_button_set_label((GtkButton*)buttonBand13,"WWV");
    gtk_widget_set_sensitive(buttonBand13,TRUE);
    gtk_button_set_label((GtkButton*)buttonBand14,"XVTR");
}

void setXVTRTitles() {
    gtk_button_set_label((GtkButton*)buttonBand1,xvtr[0].name);
    if(strcmp(xvtr[0].name,"")==0) {
        gtk_widget_set_sensitive(buttonBand1,FALSE);
    } else {
        gtk_widget_set_sensitive(buttonBand1,TRUE);
    }
    gtk_button_set_label((GtkButton*)buttonBand2,xvtr[1].name);
    if(strcmp(xvtr[1].name,"")==0) {
        gtk_widget_set_sensitive(buttonBand2,FALSE);
    } else {
        gtk_widget_set_sensitive(buttonBand2,TRUE);
    }
    gtk_button_set_label((GtkButton*)buttonBand3,xvtr[2].name);
    if(strcmp(xvtr[2].name,"")==0) {
        gtk_widget_set_sensitive(buttonBand3,FALSE);
    } else {
        gtk_widget_set_sensitive(buttonBand3,TRUE);
    }
    gtk_button_set_label((GtkButton*)buttonBand4,xvtr[3].name);
    if(strcmp(xvtr[3].name,"")==0) {
        gtk_widget_set_sensitive(buttonBand4,FALSE);
    } else {
        gtk_widget_set_sensitive(buttonBand4,TRUE);
    }
    gtk_button_set_label((GtkButton*)buttonBand5,xvtr[4].name);
    if(strcmp(xvtr[4].name,"")==0) {
        gtk_widget_set_sensitive(buttonBand5,FALSE);
    } else {
        gtk_widget_set_sensitive(buttonBand5,TRUE);
    }
    gtk_button_set_label((GtkButton*)buttonBand6,xvtr[5].name);
    if(strcmp(xvtr[5].name,"")==0) {
        gtk_widget_set_sensitive(buttonBand6,FALSE);
    } else {
        gtk_widget_set_sensitive(buttonBand6,TRUE);
    }
    gtk_button_set_label((GtkButton*)buttonBand7,xvtr[6].name);
    if(strcmp(xvtr[6].name,"")==0) {
        gtk_widget_set_sensitive(buttonBand7,FALSE);
    } else {
        gtk_widget_set_sensitive(buttonBand7,TRUE);
    }
    gtk_button_set_label((GtkButton*)buttonBand8,xvtr[7].name);
    if(strcmp(xvtr[7].name,"")==0) {
        gtk_widget_set_sensitive(buttonBand8,FALSE);
    } else {
        gtk_widget_set_sensitive(buttonBand8,TRUE);
    }
    gtk_button_set_label((GtkButton*)buttonBand9,xvtr[8].name);
    if(strcmp(xvtr[8].name,"")==0) {
        gtk_widget_set_sensitive(buttonBand9,FALSE);
    } else {
        gtk_widget_set_sensitive(buttonBand9,TRUE);
    }
    gtk_button_set_label((GtkButton*)buttonBand10,xvtr[9].name);
    if(strcmp(xvtr[9].name,"")==0) {
        gtk_widget_set_sensitive(buttonBand10,FALSE);
    } else {
        gtk_widget_set_sensitive(buttonBand10,TRUE);
    }
    gtk_button_set_label((GtkButton*)buttonBand11,xvtr[10].name);
    if(strcmp(xvtr[10].name,"")==0) {
        gtk_widget_set_sensitive(buttonBand11,FALSE);
    } else {
        gtk_widget_set_sensitive(buttonBand11,TRUE);
    }
    gtk_button_set_label((GtkButton*)buttonBand12,xvtr[11].name);
    if(strcmp(xvtr[11].name,"")==0) {
        gtk_widget_set_sensitive(buttonBand12,FALSE);
    } else {
        gtk_widget_set_sensitive(buttonBand12,TRUE);
    }
    gtk_button_set_label((GtkButton*)buttonBand13,"");
    gtk_widget_set_sensitive(buttonBand13,FALSE);
    gtk_button_set_label((GtkButton*)buttonBand14,"HF");
}



/* --------------------------------------------------------------------------*/
/** 
* @brief Select the band.
* 
* @param widget
*/
/* ----------------------------------------------------------------------------*/
void selectBand(GtkWidget* widget) {
    GtkWidget* label;
    BANDSTACK_ENTRY* entry;
    XVTR_ENTRY* xvtr_entry;
    int current;

    resetSubRx();

    if(currentBandButton) {
        label=gtk_bin_get_child((GtkBin*)currentBandButton);
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &white);
    }
    label=gtk_bin_get_child((GtkBin*)widget);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &buttonSelected);
    gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &buttonSelected);

    //save current
    if(currentBandButton) {
        if(displayHF) {
            currentHFButton=currentBandButton;
            current=bandstack[band].current_entry;
            entry=&bandstack[band].entry[current];
            entry->frequencyA=frequencyA;
            entry->mode=mode;
            entry->filter=filter;
            entry->var1Low=filterVar1Low;
            entry->var1High=filterVar1High;
            entry->var2Low=filterVar2Low;
            entry->var2High=filterVar2High;
            entry->step=frequencyIncrement;
            entry->preamp=preamp;
            entry->spectrumHigh=spectrumMAX;
            entry->spectrumLow=spectrumMIN;
            entry->spectrumStep=spectrumSTEP;
            entry->waterfallHigh=waterfallHighThreshold;
            entry->waterfallLow=waterfallLowThreshold;
        } else {
            currentXVTRButton=currentBandButton;
            xvtr_entry=&xvtr[xvtr_band];
            xvtr_entry->rxFrequency=frequencyA;
            xvtr_entry->txFrequency=frequencyB;
            xvtr_entry->mode=mode;
            xvtr_entry->filter=filter;
            xvtr_entry->var1Low=filterVar1Low;
            xvtr_entry->var1High=filterVar1High;
            xvtr_entry->var2Low=filterVar2Low;
            xvtr_entry->var2High=filterVar2High;
            xvtr_entry->step=frequencyIncrement;
            xvtr_entry->preamp=preamp;
            xvtr_entry->spectrumHigh=spectrumMAX;
            xvtr_entry->spectrumLow=spectrumMIN;
            xvtr_entry->spectrumStep=spectrumSTEP;
            xvtr_entry->waterfallHigh=waterfallHighThreshold;
            xvtr_entry->waterfallLow=waterfallLowThreshold;
        }
    }

    if(widget==buttonBand14) {
        // XVTR / HF
        displayHF=!displayHF;
        if(displayHF) {
            currentBandButton=NULL;
            setHFTitles();
            setBand(band);
            //selectBand(currentHFButton);
        } else {
            currentBandButton=NULL;
            setXVTRTitles();
            setBand(xvtr_band);
            //selectBand(currentXVTRButton);
        }
    } else {
        if(displayHF) {
            setLOFrequency(0LL);
            if(currentBandButton==widget) {
                bandstack[band].current_entry++;
                if(bandstack[band].current_entry>=bandstack[band].entries) {
                    bandstack[band].current_entry=0;
                }
            } else {
                currentBandButton=widget;
                if(widget==buttonBand1) {
                    band=band160;
                } else if(widget==buttonBand2) {
                    band=band80;
                } else if(widget==buttonBand3) {
                    band=band60;
                } else if(widget==buttonBand4) {
                    band=band40;
                } else if(widget==buttonBand5) {
                    band=band30;
                } else if(widget==buttonBand6) {
                    band=band20;
                } else if(widget==buttonBand7) {
                    band=band17;
                } else if(widget==buttonBand8) {
                    band=band15;
                } else if(widget==buttonBand9) {
                    band=band12;
                } else if(widget==buttonBand10) {
                    band=band10;
                } else if(widget==buttonBand11) {
                    band=band6;
                } else if(widget==buttonBand12) {
                    band=bandGen;
                } else if(widget==buttonBand13) {
                    band=bandWWV;
                } else if(widget==buttonBand14) {
                    band=bandXVTR;
                }
            }

            current=bandstack[band].current_entry;
            entry=&bandstack[band].entry[current];

            {
                int *m=malloc(sizeof(int));
                *m=entry->mode;
                setMode(m);
            }
            filterVar1Low=entry->var1Low;
            filterVar1High=entry->var1High;
            filterVar2Low=entry->var2Low;
            filterVar2High=entry->var2High;
            setFilter(entry->filter);
            {
                long long *f=malloc(sizeof(long long));
                *f=entry->frequencyA;
                setAFrequency(f);
            }
            setIncrement(entry->step);

            //setPreamp(entry->preamp);
            forcePreamp(entry->preamp);

            spectrumMAX=entry->spectrumHigh;
            spectrumMIN=entry->spectrumLow;
            spectrumSTEP=entry->spectrumStep;
            waterfallHighThreshold=entry->waterfallHigh;
            waterfallLowThreshold=entry->waterfallLow;
        } else {
            currentBandButton=widget;

            if(widget==buttonBand1) {
                xvtr_band=band160;
            } else if(widget==buttonBand2) {
                xvtr_band=band80;
            } else if(widget==buttonBand3) {
                xvtr_band=band60;
            } else if(widget==buttonBand4) {
                xvtr_band=band40;
            } else if(widget==buttonBand5) {
                xvtr_band=band30;
            } else if(widget==buttonBand6) {
                xvtr_band=band20;
            } else if(widget==buttonBand7) {
                xvtr_band=band17;
            } else if(widget==buttonBand8) {
                xvtr_band=band15;
            } else if(widget==buttonBand9) {
                xvtr_band=band12;
            } else if(widget==buttonBand10) {
                xvtr_band=band10;
            } else if(widget==buttonBand11) {
                xvtr_band=band6;
            } else if(widget==buttonBand12) {
                xvtr_band=bandGen;
            }

            xvtr_entry=&xvtr[xvtr_band];
            {
                int *m=malloc(sizeof(int));
                *m=xvtr_entry->mode;
                setMode(m);
            }
            filterVar1Low=xvtr_entry->var1Low;
            filterVar1High=xvtr_entry->var1High;
            filterVar2Low=xvtr_entry->var2Low;
            filterVar2High=xvtr_entry->var2High;
            setFilter(xvtr_entry->filter);
            setLOFrequency(xvtr_entry->rxFrequencyLO);
            {
                long long *f=malloc(sizeof(long long));
                *f=xvtr_entry->rxFrequency;
                setAFrequency(f);
            }
            setIncrement(xvtr_entry->step);

            //setPreamp(xvtr_entry->preamp);
            forcePreamp(xvtr_entry->preamp);

            spectrumMAX=xvtr_entry->spectrumHigh;
            spectrumMIN=xvtr_entry->spectrumLow;
            spectrumSTEP=xvtr_entry->spectrumStep;
            waterfallHighThreshold=xvtr_entry->waterfallHigh;
            waterfallLowThreshold=xvtr_entry->waterfallLow;

        }
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the band.
* 
* @param band -- band value 
*/
/* ----------------------------------------------------------------------------*/
void setBand(int band) {
    GtkWidget* widget;
    switch(band) {
        case band160:
            widget=buttonBand1;
            break;
        case band80:
            widget=buttonBand2;
            break;
        case band60:
            widget=buttonBand3;
            break;
        case band40:
            widget=buttonBand4;
            break;
        case band30:
            widget=buttonBand5;
            break;
        case band20:
            widget=buttonBand6;
            break;
        case band17:
            widget=buttonBand7;
            break;
        case band15:
            widget=buttonBand8;
            break;
        case band12:
            widget=buttonBand9;
            break;
        case band10:
            widget=buttonBand10;
            break;
        case band6:
            widget=buttonBand11;
            break;
        case bandGen:
            widget=buttonBand12;
            break;
        case bandWWV:
            widget=buttonBand13;
            break;
        case bandXVTR:
            widget=buttonBand14;
            break;
    }
    selectBand(widget);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the band using g_idle_add
* 
* @param data -- pointer to the band 
*/
/* ----------------------------------------------------------------------------*/
int remoteSetBand(gpointer *data) {
    int band=*(int*)data;
    setBand(band);
    free(data);
    return 0;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Force the band.
* 
* @param b -- band value.
*/
/* ----------------------------------------------------------------------------*/
void forceBand(int b) {
    GtkWidget* label;
    GtkWidget* widget;
    switch(b) {
        case band160:
            widget=buttonBand1;
            break;
        case band80:
            widget=buttonBand2;
            break;
        case band60:
            widget=buttonBand3;
            break;
        case band40:
            widget=buttonBand4;
            break;
        case band30:
            widget=buttonBand5;
            break;
        case band20:
            widget=buttonBand6;
            break;
        case band17:
            widget=buttonBand7;
            break;
        case band15:
            widget=buttonBand8;
            break;
        case band12:
            widget=buttonBand9;
            break;
        case band10:
            widget=buttonBand10;
            break;
        case band6:
            widget=buttonBand11;
            break;
        case bandGen:
            widget=buttonBand12;
            break;
        case bandWWV:
            widget=buttonBand13;
            break;
        case bandXVTR:
            widget=buttonBand14;
            break;
    }
    if(currentBandButton) {
        label=gtk_bin_get_child((GtkBin*)currentBandButton);
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    }
    label=gtk_bin_get_child((GtkBin*)widget);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &buttonSelected);

    currentBandButton=widget;

    if(displayHF) {
        band=b;
    } else {
        xvtr_band=b;
    }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Callback when a band button is pressed.
* 
* @param widget
* @param data
*/
/* ----------------------------------------------------------------------------*/
void bandCallback(GtkWidget* widget,gpointer data) {
    selectBand(widget);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Build the GUI
* 
* @return 
*/
/* ----------------------------------------------------------------------------*/
GtkWidget* buildBandUI() {
    GtkWidget* label;

    bandFrame=gtk_frame_new("Band");
    gtk_widget_modify_bg(bandFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(bandFrame)),GTK_STATE_NORMAL,&white);

    bandTable=gtk_table_new(4,4,TRUE);

    // band selection
    buttonBand1 = gtk_button_new_with_label ("160");
    gtk_widget_modify_bg(buttonBand1, GTK_STATE_NORMAL, &bandButtonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBand1);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBand1),50,25);
    g_signal_connect(G_OBJECT(buttonBand1),"clicked",G_CALLBACK(bandCallback),NULL);
    gtk_widget_show(buttonBand1);
    gtk_table_attach_defaults(GTK_TABLE(bandTable),buttonBand1,0,1,0,1);

    buttonBand2 = gtk_button_new_with_label ("80");
    gtk_widget_modify_bg(buttonBand2, GTK_STATE_NORMAL, &bandButtonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBand2);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBand2),50,25);
    g_signal_connect(G_OBJECT(buttonBand2),"clicked",G_CALLBACK(bandCallback),NULL);
    gtk_widget_show(buttonBand2);
    gtk_table_attach_defaults(GTK_TABLE(bandTable),buttonBand2,1,2,0,1);

    buttonBand3 = gtk_button_new_with_label ("60");
    gtk_widget_modify_bg(buttonBand3, GTK_STATE_NORMAL, &bandButtonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBand3);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBand3),50,25);
    g_signal_connect(G_OBJECT(buttonBand3),"clicked",G_CALLBACK(bandCallback),NULL);
    gtk_widget_show(buttonBand3);
    gtk_table_attach_defaults(GTK_TABLE(bandTable),buttonBand3,2,3,0,1);

    buttonBand4 = gtk_button_new_with_label ("40");
    gtk_widget_modify_bg(buttonBand4, GTK_STATE_NORMAL, &bandButtonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBand4);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBand4),50,25);
    g_signal_connect(G_OBJECT(buttonBand4),"clicked",G_CALLBACK(bandCallback),NULL);
    gtk_widget_show(buttonBand4);
    gtk_table_attach_defaults(GTK_TABLE(bandTable),buttonBand4,3,4,0,1);

    buttonBand5 = gtk_button_new_with_label ("30");
    gtk_widget_modify_bg(buttonBand5, GTK_STATE_NORMAL, &bandButtonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBand5);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBand5),50,25);
    g_signal_connect(G_OBJECT(buttonBand5),"clicked",G_CALLBACK(bandCallback),NULL);
    gtk_widget_show(buttonBand5);
    gtk_table_attach_defaults(GTK_TABLE(bandTable),buttonBand5,0,1,1,2);

    buttonBand6 = gtk_button_new_with_label ("20");
    gtk_widget_modify_bg(buttonBand6, GTK_STATE_NORMAL, &bandButtonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBand6);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBand6),50,25);
    g_signal_connect(G_OBJECT(buttonBand6),"clicked",G_CALLBACK(bandCallback),NULL);
    gtk_widget_show(buttonBand6);
    gtk_table_attach_defaults(GTK_TABLE(bandTable),buttonBand6,1,2,1,2);

    buttonBand7 = gtk_button_new_with_label ("17");
    gtk_widget_modify_bg(buttonBand7, GTK_STATE_NORMAL, &bandButtonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBand7);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBand7),50,25);
    g_signal_connect(G_OBJECT(buttonBand7),"clicked",G_CALLBACK(bandCallback),NULL);
    gtk_widget_show(buttonBand7);
    gtk_table_attach_defaults(GTK_TABLE(bandTable),buttonBand7,2,3,1,2);

    buttonBand8 = gtk_button_new_with_label ("15");
    gtk_widget_modify_bg(buttonBand8, GTK_STATE_NORMAL, &bandButtonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBand8);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBand8),50,25);
    g_signal_connect(G_OBJECT(buttonBand8),"clicked",G_CALLBACK(bandCallback),NULL);
    gtk_widget_show(buttonBand8);
    gtk_table_attach_defaults(GTK_TABLE(bandTable),buttonBand8,3,4,1,2);

    buttonBand9 = gtk_button_new_with_label ("12");
    gtk_widget_modify_bg(buttonBand9, GTK_STATE_NORMAL, &bandButtonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBand9);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBand9),50,25);
    g_signal_connect(G_OBJECT(buttonBand9),"clicked",G_CALLBACK(bandCallback),NULL);
    gtk_widget_show(buttonBand9);
    gtk_table_attach_defaults(GTK_TABLE(bandTable),buttonBand9,0,1,2,3);

    buttonBand10 = gtk_button_new_with_label ("10");
    gtk_widget_modify_bg(buttonBand10, GTK_STATE_NORMAL, &bandButtonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBand10);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBand10),50,25);
    g_signal_connect(G_OBJECT(buttonBand10),"clicked",G_CALLBACK(bandCallback),NULL);
    gtk_widget_show(buttonBand10);
    gtk_table_attach_defaults(GTK_TABLE(bandTable),buttonBand10,1,2,2,3);

    buttonBand11 = gtk_button_new_with_label ("6");
    gtk_widget_modify_bg(buttonBand11, GTK_STATE_NORMAL, &bandButtonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBand11);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBand11),50,25);
    g_signal_connect(G_OBJECT(buttonBand11),"clicked",G_CALLBACK(bandCallback),NULL);
    gtk_widget_show(buttonBand11);
    gtk_table_attach_defaults(GTK_TABLE(bandTable),buttonBand11,2,3,2,3);

    buttonBand12 = gtk_button_new_with_label ("GEN");
    gtk_widget_modify_bg(buttonBand12, GTK_STATE_NORMAL, &bandButtonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBand12);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBand12),50,25);
    g_signal_connect(G_OBJECT(buttonBand12),"clicked",G_CALLBACK(bandCallback),NULL);
    gtk_widget_show(buttonBand12);
    gtk_table_attach_defaults(GTK_TABLE(bandTable),buttonBand12,3,4,2,3);

    buttonBand13 = gtk_button_new_with_label ("WWV");
    gtk_widget_modify_bg(buttonBand13, GTK_STATE_NORMAL, &bandButtonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBand13);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBand13),50,25);
    g_signal_connect(G_OBJECT(buttonBand13),"clicked",G_CALLBACK(bandCallback),NULL);
    gtk_widget_show(buttonBand13);
    gtk_table_attach_defaults(GTK_TABLE(bandTable),buttonBand13,0,1,3,4);

    buttonBand14 = gtk_button_new_with_label ("XVTR");
    gtk_widget_modify_bg(buttonBand14, GTK_STATE_NORMAL, &bandButtonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonBand14);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &black);
    gtk_widget_set_size_request(GTK_WIDGET(buttonBand14),50,25);
    g_signal_connect(G_OBJECT(buttonBand14),"clicked",G_CALLBACK(bandCallback),NULL);
    gtk_widget_show(buttonBand14);
    gtk_table_attach_defaults(GTK_TABLE(bandTable),buttonBand14,1,2,3,4);

    configureXVTRButton();

    if(!displayHF) {
        setXVTRTitles();
    }

    gtk_container_add(GTK_CONTAINER(bandFrame),bandTable);
    gtk_widget_show(bandTable);
    gtk_widget_show(bandFrame);


    return bandFrame;
  
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the bandstack. 
* 
* @param band -- Band value
* @param stack -- Stack index
* @param lo
* @param a -- Frequency A
* @param b -- Frequency B
* @param mode -- Operating mode
* @param filter -- Default filter
*/
/* ----------------------------------------------------------------------------*/
void setBandstack(int band,int stack,long long lo,long long a,long long b,int mode,int filter) {
    int current;
    BANDSTACK_ENTRY* entry;

    current=bandstack[band].current_entry;
    entry=&bandstack[band].entry[current];
    entry->frequencyA=a;
    entry->mode=mode;
    entry->filter=filter;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Save the bandstack state. 
*/
/* ----------------------------------------------------------------------------*/
void bandSaveState() {
    char string[128];
    int current;
    BANDSTACK_ENTRY* entry;
    XVTR_ENTRY* xvtr_entry;

    //save current band info
    if(displayHF) {
        current=bandstack[band].current_entry;
        entry=&bandstack[band].entry[current];
        entry->frequencyA=frequencyA;
        entry->mode=mode;
        entry->filter=filter;
        entry->var1Low=filterVar1Low;
        entry->var1High=filterVar1High;
        entry->var2Low=filterVar2Low;
        entry->var2High=filterVar2High;
        entry->step=frequencyIncrement;
        entry->preamp=preamp;
        entry->spectrumHigh=spectrumMAX;
        entry->spectrumLow=spectrumMIN;
        entry->spectrumStep=spectrumSTEP;
        entry->waterfallHigh=waterfallHighThreshold;
        entry->waterfallLow=waterfallLowThreshold;
    } else {
        xvtr_entry=&xvtr[xvtr_band];
        xvtr_entry->rxFrequency=frequencyA;
        xvtr_entry->txFrequency=frequencyB;
        xvtr_entry->mode=mode;
        xvtr_entry->filter=filter;
        xvtr_entry->var1Low=filterVar1Low;
        xvtr_entry->var1High=filterVar1High;
        xvtr_entry->var2Low=filterVar2Low;
        xvtr_entry->var2High=filterVar2High;
        xvtr_entry->step=frequencyIncrement;
        xvtr_entry->preamp=preamp;
        xvtr_entry->spectrumHigh=spectrumMAX;
        xvtr_entry->spectrumLow=spectrumMIN;
        xvtr_entry->spectrumStep=spectrumSTEP;
        xvtr_entry->waterfallHigh=waterfallHighThreshold;
    }

    int b;
    int stack;
    char name[128];
    for(b=0;b<BANDS;b++) {
        sprintf(string,"%d",bandstack[b].entries);
        sprintf(name,"band.%d.entries",b);
        setProperty(name,string);

        sprintf(string,"%d",bandstack[b].current_entry);
        sprintf(name,"band.%d.current",b);
        setProperty(name,string);

        for(stack=0;stack<bandstack[b].entries;stack++) {
            entry=bandstack[b].entry;
            entry+=stack;

            sprintf(string,"%lld",entry->frequencyA);
            sprintf(name,"band.%d.stack.%d.a",b,stack);
            setProperty(name,string);

            sprintf(string,"%d",entry->mode);
            sprintf(name,"band.%d.stack.%d.mode",b,stack);
            setProperty(name,string);

            sprintf(string,"%d",entry->filter);
            sprintf(name,"band.%d.stack.%d.filter",b,stack);
            setProperty(name,string);

            sprintf(string,"%d",entry->var1Low);
            sprintf(name,"band.%d.stack.%d.var1Low",b,stack);
            setProperty(name,string);

            sprintf(string,"%d",entry->var1High);
            sprintf(name,"band.%d.stack.%d.var1High",b,stack);
            setProperty(name,string);

            sprintf(string,"%d",entry->var2Low);
            sprintf(name,"band.%d.stack.%d.var2Low",b,stack);
            setProperty(name,string);

            sprintf(string,"%d",entry->var2High);
            sprintf(name,"band.%d.stack.%d.var2High",b,stack);
            setProperty(name,string);

            sprintf(string,"%d",entry->step);
            sprintf(name,"band.%d.stack.%d.step",b,stack);
            setProperty(name,string);

            sprintf(string,"%d",entry->preamp);
            sprintf(name,"band.%d.stack.%d.preamp",b,stack);
            setProperty(name,string);

            sprintf(string,"%d",entry->spectrumHigh);
            sprintf(name,"band.%d.stack.%d.spectrumHigh",b,stack);
            setProperty(name,string);

            sprintf(string,"%d",entry->spectrumLow);
            sprintf(name,"band.%d.stack.%d.spectrumLow",b,stack);
            setProperty(name,string);

            sprintf(string,"%d",entry->spectrumStep);
            sprintf(name,"band.%d.stack.%d.spectrumStep",b,stack);
            setProperty(name,string);

            sprintf(string,"%d",entry->waterfallHigh);
            sprintf(name,"band.%d.stack.%d.waterfallHigh",b,stack);
            setProperty(name,string);

            sprintf(string,"%d",entry->waterfallLow);
            sprintf(name,"band.%d.stack.%d.waterfallLow",b,stack);
            setProperty(name,string);

        }
    }

    sprintf(string,"%d",band);
    setProperty("band",string);

    // save xvtr entries
    for(b=0;b<12;b++) {
        xvtr_entry=&xvtr[b];

        if(strcmp(xvtr_entry->name,"")!=0) {
            sprintf(name,"xvtr.%d.name",b);
            setProperty(name,xvtr_entry->name);

            sprintf(string,"%lld",xvtr_entry->rxFrequency);
            sprintf(name,"xvtr.%d.rxFrequency",b);
            setProperty(name,string);

            sprintf(string,"%lld",xvtr_entry->rxFrequencyMin);
            sprintf(name,"xvtr.%d.rxFrequencyMin",b);
            setProperty(name,string);

            sprintf(string,"%lld",xvtr_entry->rxFrequencyMax);
            sprintf(name,"xvtr.%d.rxFrequencyMax",b);
            setProperty(name,string);

            sprintf(string,"%lld",xvtr_entry->rxFrequencyLO);
            sprintf(name,"xvtr.%d.rxFrequencyLO",b);
            setProperty(name,string);

            sprintf(string,"%lld",xvtr_entry->txFrequency);
            sprintf(name,"xvtr.%d.txFrequency",b);
            setProperty(name,string);

            sprintf(string,"%lld",xvtr_entry->txFrequencyMin);
            sprintf(name,"xvtr.%d.txFrequencyMin",b);
            setProperty(name,string);

            sprintf(string,"%lld",xvtr_entry->txFrequencyMax);
            sprintf(name,"xvtr.%d.txFrequencyMax",b);
            setProperty(name,string);

            sprintf(string,"%lld",xvtr_entry->txFrequencyLO);
            sprintf(name,"xvtr.%d.txFrequencyLO",b);
            setProperty(name,string);

        }
    }

    sprintf(string,"%d",xvtr_band);
    setProperty("xvtr_band",string);

    sprintf(string,"%d",displayHF);
    setProperty("displayHF",string);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief count of XVTR configs
*/
/* ----------------------------------------------------------------------------*/

void configureXVTRButton() {
    int i;
    int count=0;
    for(i=0;i<12;i++) {
        if(strcmp(xvtr[i].name,"")) {
            count++;
        }
    }

    if(displayHF) {
        gtk_widget_set_sensitive(buttonBand14,count>0);
    }

    if(count==0) {
        if(displayHF==0) {
            // force back to HF bands
            selectBand(buttonBand14);
        }
    } else {
        if(displayHF==0) {
            setXVTRTitles();
            // check xvtr_band still valid
            if(strcmp(xvtr[xvtr_band].name,"")==0) {
                for(i=0;i<12;i++) {
                    if(strcmp(xvtr[i].name,"")) {
                        xvtr_band=i;
                        if(displayHF==0) {
                        }
                        break;
                    }
                }
            }
            forceBand(xvtr_band);
        }
    }

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Restore the bandstack state and xvtr state.
*/
/* ----------------------------------------------------------------------------*/
void bandRestoreState() {
    char* value;

    int b;
    int stack;
    char name[128];
    BANDSTACK_ENTRY* entry;
    int current;
    XVTR_ENTRY* xvtr_entry;
 
    // setup default bandstacks
    bandstack[0].entries=3;
    bandstack[0].current_entry=0;
    bandstack[0].entry=bandstack160;
    bandstack[1].entries=3;
    bandstack[1].current_entry=0;
    bandstack[1].entry=bandstack80;
    bandstack[2].entries=5;
    bandstack[2].current_entry=0;
    bandstack[2].entry=bandstack60;
    bandstack[3].entries=3;
    bandstack[3].current_entry=0;
    bandstack[3].entry=bandstack40;
    bandstack[4].entries=3;
    bandstack[4].current_entry=0;
    bandstack[4].entry=bandstack30;
    bandstack[5].entries=3;
    bandstack[5].current_entry=0;
    bandstack[5].entry=bandstack20;
    bandstack[6].entries=3;
    bandstack[6].current_entry=0;
    bandstack[6].entry=bandstack18;
    bandstack[7].entries=3;
    bandstack[7].current_entry=0;
    bandstack[7].entry=bandstack15;
    bandstack[8].entries=3;
    bandstack[8].current_entry=0;
    bandstack[8].entry=bandstack12;
    bandstack[9].entries=3;
    bandstack[9].current_entry=0;
    bandstack[9].entry=bandstack10;
    bandstack[10].entries=3;
    bandstack[10].current_entry=0;
    bandstack[10].entry=bandstack50;
    bandstack[11].entries=3;
    bandstack[11].current_entry=0;
    bandstack[11].entry=bandstackGEN;
    bandstack[12].entries=5;
    bandstack[12].current_entry=0;
    bandstack[12].entry=bandstackWWV;

    // load the bandstack entries
    for(b=0;b<BANDS;b++) {

        sprintf(name,"band.%d.entries",b);
        value=getProperty(name);
        if(value) bandstack[b].entries=atoi(value);

        sprintf(name,"band.%d.current",b);
        value=getProperty(name);
        if(value) bandstack[b].current_entry=atoi(value);

        for(stack=0;stack<bandstack[b].entries;stack++) {
            entry=bandstack[b].entry;
            entry+=stack;

            sprintf(name,"band.%d.stack.%d.a",b,stack);
            value=getProperty(name);
            if(value) entry->frequencyA=atoll(value);

            sprintf(name,"band.%d.stack.%d.mode",b,stack);
            value=getProperty(name);
            if(value) entry->mode=atoi(value);

            sprintf(name,"band.%d.stack.%d.filter",b,stack);
            value=getProperty(name);
            if(value) entry->filter=atoi(value);

            sprintf(name,"band.%d.stack.%d.var1Low",b,stack);
            value=getProperty(name);
            if(value) entry->var1Low=atoi(value);

            sprintf(name,"band.%d.stack.%d.var1High",b,stack);
            value=getProperty(name);
            if(value) entry->var1High=atoi(value);

            sprintf(name,"band.%d.stack.%d.var2Low",b,stack);
            value=getProperty(name);
            if(value) entry->var2Low=atoi(value);

            sprintf(name,"band.%d.stack.%d.var2High",b,stack);
            value=getProperty(name);
            if(value) entry->var2High=atoi(value);

            sprintf(name,"band.%d.stack.%d.step",b,stack);
            value=getProperty(name);
            if(value) entry->step=atoi(value);

            sprintf(name,"band.%d.stack.%d.preamp",b,stack);
            value=getProperty(name);
            if(value) entry->preamp=atoi(value);

            sprintf(name,"band.%d.stack.%d.spectrumHigh",b,stack);
            value=getProperty(name);
            if(value) entry->spectrumHigh=atoi(value);

            sprintf(name,"band.%d.stack.%d.spectrumLow",b,stack);
            value=getProperty(name);
            if(value) entry->spectrumLow=atoi(value);

            sprintf(name,"band.%d.stack.%d.spectrumStep",b,stack);
            value=getProperty(name);
            if(value) entry->spectrumStep=atoi(value);

            sprintf(name,"band.%d.stack.%d.waterfallHigh",b,stack);
            value=getProperty(name);
            if(value) entry->waterfallHigh=atoi(value);

            sprintf(name,"band.%d.stack.%d.waterfallLow",b,stack);
            value=getProperty(name);
            if(value) entry->waterfallLow=atoi(value);

        }
    }

    // setup XVTR
    for(b=0;b<12;b++) {
        xvtr_entry=&xvtr[b];

        sprintf(name,"xvtr.%d.name",b);
        value=getProperty(name);
        if(value) {
            strcpy(xvtr_entry->name,value);

            sprintf(name,"xvtr.%d.rxFrequency",b);
            value=getProperty(name);
            if(value) xvtr_entry->rxFrequency=atoll(value);

            sprintf(name,"xvtr.%d.rxFrequencyMin",b);
            value=getProperty(name);
            if(value) xvtr_entry->rxFrequencyMin=atoll(value);

            sprintf(name,"xvtr.%d.rxFrequencyMax",b);
            value=getProperty(name);
            if(value) xvtr_entry->rxFrequencyMax=atoll(value);

            sprintf(name,"xvtr.%d.rxFrequencyLO",b);
            value=getProperty(name);
            if(value) xvtr_entry->rxFrequencyLO=atoll(value);

            sprintf(name,"xvtr.%d.txFrequency",b);
            value=getProperty(name);
            if(value) xvtr_entry->txFrequency=atoll(value);

            sprintf(name,"xvtr.%d.txFrequencyMin",b);
            value=getProperty(name);
            if(value) xvtr_entry->txFrequencyMin=atoll(value);

            sprintf(name,"xvtr.%d.txFrequencyMax",b);
            value=getProperty(name);
            if(value) xvtr_entry->txFrequencyMax=atoll(value);

            sprintf(name,"xvtr.%d.txFrequencyLO",b);
            value=getProperty(name);
            if(value) xvtr_entry->txFrequencyLO=atoll(value);

            sprintf(name,"xvtr.%d.mode",b);
            value=getProperty(name);
            if(value) xvtr_entry->mode=atoi(value);

            sprintf(name,"xvtr.%d.filter",b);
            value=getProperty(name);
            if(value) xvtr_entry->filter=atoi(value);

            sprintf(name,"xvtr.%d.var1Low",b);
            value=getProperty(name);
            if(value) xvtr_entry->var1Low=atoi(value);

            sprintf(name,"xvtr.%d.var1High",b);
            value=getProperty(name);
            if(value) xvtr_entry->var1High=atoi(value);

            sprintf(name,"xvtr.%d.var2Low",b);
            value=getProperty(name);
            if(value) xvtr_entry->var2Low=atoi(value);

            sprintf(name,"xvtr.%d.var2High",b);
            value=getProperty(name);
            if(value) xvtr_entry->var2High=atoi(value);

            sprintf(name,"xvtr.%d.step",b);
            value=getProperty(name);
            if(value) xvtr_entry->step=atoi(value);

            sprintf(name,"xvtr.%d.preamp",b);
            value=getProperty(name);
            if(value) xvtr_entry->preamp=atoi(value);

            sprintf(name,"xvtr.%d.spectrumHigh",b);
            value=getProperty(name);
            if(value) xvtr_entry->spectrumHigh=atoi(value);

            sprintf(name,"xvtr.%d.spectrumLow",b);
            value=getProperty(name);
            if(value) xvtr_entry->spectrumLow=atoi(value);

            sprintf(name,"xvtr.%d.spectrumStep",b);
            value=getProperty(name);
            if(value) xvtr_entry->spectrumStep=atoi(value);
    
            sprintf(name,"xvtr.%d.waterfallHigh",b);
            value=getProperty(name);
            if(value) xvtr_entry->waterfallHigh=atoi(value);

            sprintf(name,"xvtr.%d.waterfallLow",b);
            value=getProperty(name);
            if(value) xvtr_entry->waterfallLow=atoi(value);
        }

    }


    value=getProperty("band");
    if(value) band=atoi(value);

    value=getProperty("xvtr_band");
    if(value) xvtr_band=atoi(value);

    value=getProperty("displayHF");
    if(value) displayHF=atoi(value);

}

BAND_LIMITS* getBandLimits(long long minDisplay,long long maxDisplay) {
    BAND_LIMITS* limits;
    int i;

    for(i=0;i<NUM_BAND_LIMITS;i++) {
        limits=&bandLimits[i];
        if((minDisplay<=limits->minFrequency&&maxDisplay>=limits->minFrequency) ||
           (minDisplay<=limits->maxFrequency&&maxDisplay>=limits->maxFrequency)) {
            return limits;
        }
    }

    return NULL;
}

XVTR_ENTRY* getXvtrEntry(int i) {
    return &xvtr[i];
}
