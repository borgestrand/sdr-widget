/** 
* @file setup.c
* @brief Setup functions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-12
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


//
// setup.c
//

#include <gtk/gtk.h>
#include "bandscope.h"
#include "filter.h"
#include "main.h"
#include "mode.h"
#include "spectrum.h"
#include "spectrum_update.h"

GtkWidget* setupDisplayFixed;

GtkWidget* spectrumHighLabel;
GtkWidget* spectrumHighSpinButton;
GtkWidget* spectrumLowLabel;
GtkWidget* spectrumLowSpinButton;
GtkWidget* spectrumStepLabel;
GtkWidget* spectrumStepSpinButton;
GtkWidget* spectrumRateLabel;
GtkWidget* spectrumRateSpinButton;

GtkWidget* waterfallHighLabel;
GtkWidget* waterfallHighSpinButton;
GtkWidget* waterfallLowLabel;
GtkWidget* waterfallLowSpinButton;

GtkWidget* bandscopeHighLabel;
GtkWidget* bandscopeHighSpinButton;
GtkWidget* bandscopeLowLabel;
GtkWidget* bandscopeLowSpinButton;

GtkWidget* spectrumAverageCheckButton;
GtkWidget* spectrumSmoothingLabel;
GtkWidget* spectrumSmoothingSpinButton;

GtkWidget* bandscopeAverageCheckButton;
GtkWidget* bandscopeSmoothingLabel;
GtkWidget* bandscopeSmoothingSpinButton;

GtkWidget* cwPitchLabel;
GtkWidget* cwPitchSpinButton;

void spectrumHighChanged(GtkSpinButton* spinButton,gpointer data);
void spectrumLowChanged(GtkSpinButton* spinButton,gpointer data);
void spectrumStepChanged(GtkSpinButton* spinButton,gpointer data);
void spectrumRateChanged(GtkSpinButton* spinButton,gpointer data);
void waterfallHighChanged(GtkSpinButton* spinButton,gpointer data);
void waterfallLowChanged(GtkSpinButton* spinButton,gpointer data);
void bandscopeHighChanged(GtkSpinButton* spinButton,gpointer data);
void bandscopeLowChanged(GtkSpinButton* spinButton,gpointer data);
void cwPitchChanged(GtkSpinButton* spinButton,gpointer data);
void spectrumAverageChanged(GtkToggleButton* spinButton,gpointer data);
void spectrumSmoothingChanged(GtkSpinButton* spinButton,gpointer data);
void bandscopeAverageChanged(GtkToggleButton* spinButton,gpointer data);
void bandscopeSmoothingChanged(GtkSpinButton* spinButton,gpointer data);

/* --------------------------------------------------------------------------*/
/** 
* @brief display setup UI
*/
GtkWidget* displaySetupUI() {
        setupDisplayFixed=gtk_fixed_new();

        // add specturm controls
        spectrumHighLabel=gtk_label_new("Spectrum High");
        gtk_widget_show(spectrumHighLabel);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,spectrumHighLabel,10,10);
        spectrumHighSpinButton=gtk_spin_button_new_with_range(-200,200,5);
        gtk_spin_button_set_value((GtkSpinButton*)spectrumHighSpinButton,(double)spectrumMAX);
        g_signal_connect(G_OBJECT(spectrumHighSpinButton),"value-changed",G_CALLBACK(spectrumHighChanged),NULL);
        gtk_widget_show(spectrumHighSpinButton);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,spectrumHighSpinButton,250,10);

        spectrumLowLabel=gtk_label_new("Spectrum Low");
        gtk_widget_show(spectrumLowLabel);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,spectrumLowLabel,10,40);
        spectrumLowSpinButton=gtk_spin_button_new_with_range(-200,200,5);
        gtk_spin_button_set_value((GtkSpinButton*)spectrumLowSpinButton,(double)spectrumMIN);
        g_signal_connect(G_OBJECT(spectrumLowSpinButton),"value-changed",G_CALLBACK(spectrumLowChanged),NULL);
        gtk_widget_show(spectrumLowSpinButton);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,spectrumLowSpinButton,250,40);

        spectrumStepLabel=gtk_label_new("Spectrum Step");
        gtk_widget_show(spectrumStepLabel);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,spectrumStepLabel,10,70);
        spectrumStepSpinButton=gtk_spin_button_new_with_range(10,50,10);
        gtk_spin_button_set_value((GtkSpinButton*)spectrumStepSpinButton,(double)spectrumSTEP);
        g_signal_connect(G_OBJECT(spectrumStepSpinButton),"value-changed",G_CALLBACK(spectrumStepChanged),NULL);
        gtk_widget_show(spectrumStepSpinButton);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,spectrumStepSpinButton,250,70);

        spectrumRateLabel=gtk_label_new("Spectrum Update Rate");
        gtk_widget_show(spectrumRateLabel);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,spectrumRateLabel,10,100);
        spectrumRateSpinButton=gtk_spin_button_new_with_range(1,50,1);
        gtk_spin_button_set_value((GtkSpinButton*)spectrumRateSpinButton,(double)spectrumUpdatesPerSecond);
        g_signal_connect(G_OBJECT(spectrumRateSpinButton),"value-changed",G_CALLBACK(spectrumRateChanged),NULL);
        gtk_widget_show(spectrumRateSpinButton);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,spectrumRateSpinButton,250,100);


        // add waterfall controls
        waterfallHighLabel=gtk_label_new("Waterfall High");
        gtk_widget_show(waterfallHighLabel);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,waterfallHighLabel,10,140);
        waterfallHighSpinButton=gtk_spin_button_new_with_range(-200,200,5);
        gtk_spin_button_set_value((GtkSpinButton*)waterfallHighSpinButton,(double)waterfallHighThreshold);
        g_signal_connect(G_OBJECT(waterfallHighSpinButton),"value-changed",G_CALLBACK(waterfallHighChanged),NULL);
        gtk_widget_show(waterfallHighSpinButton);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,waterfallHighSpinButton,250,140);

        waterfallLowLabel=gtk_label_new("Waterfall Low");
        gtk_widget_show(waterfallLowLabel);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,waterfallLowLabel,10,170);
        waterfallLowSpinButton=gtk_spin_button_new_with_range(-200,200,5);
        gtk_spin_button_set_value((GtkSpinButton*)waterfallLowSpinButton,(double)waterfallLowThreshold);
        g_signal_connect(G_OBJECT(waterfallLowSpinButton),"value-changed",G_CALLBACK(waterfallLowChanged),NULL);
        gtk_widget_show(waterfallLowSpinButton);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,waterfallLowSpinButton,250,170);

        // add bandscope controls
        bandscopeHighLabel=gtk_label_new("Bandscope High");
        gtk_widget_show(bandscopeHighLabel);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,bandscopeHighLabel,10,210);
        bandscopeHighSpinButton=gtk_spin_button_new_with_range(-200,200,5);
        gtk_spin_button_set_value((GtkSpinButton*)bandscopeHighSpinButton,(double)bandscopeMAX);
        g_signal_connect(G_OBJECT(bandscopeHighSpinButton),"value-changed",G_CALLBACK(bandscopeHighChanged),NULL);
        gtk_widget_show(bandscopeHighSpinButton);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,bandscopeHighSpinButton,250,210);

        bandscopeLowLabel=gtk_label_new("Bandscope Low");
        gtk_widget_show(bandscopeLowLabel);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,bandscopeLowLabel,10,240);
        bandscopeLowSpinButton=gtk_spin_button_new_with_range(-200,200,5);
        gtk_spin_button_set_value((GtkSpinButton*)bandscopeLowSpinButton,(double)bandscopeMIN);
        g_signal_connect(G_OBJECT(bandscopeLowSpinButton),"value-changed",G_CALLBACK(bandscopeLowChanged),NULL);
        gtk_widget_show(bandscopeLowSpinButton);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,bandscopeLowSpinButton,250,240);

        // add cw pitch
        cwPitchLabel=gtk_label_new("CW Pitch");
        gtk_widget_show(cwPitchLabel);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,cwPitchLabel,10,280);
        cwPitchSpinButton=gtk_spin_button_new_with_range(200,1000,50);
        gtk_spin_button_set_value((GtkSpinButton*)cwPitchSpinButton,(double)cwPitch);
        g_signal_connect(G_OBJECT(cwPitchSpinButton),"value-changed",G_CALLBACK(cwPitchChanged),NULL);
        gtk_widget_show(cwPitchSpinButton);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,cwPitchSpinButton,250,280);

        // add Spectrum Average checkbox
        spectrumAverageCheckButton=gtk_check_button_new_with_label("Spectrum Averaging");
        gtk_toggle_button_set_active((GtkToggleButton*)spectrumAverageCheckButton,(gboolean)spectrumAverage);
        gtk_widget_show(spectrumAverageCheckButton);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,spectrumAverageCheckButton,250,320);
        g_signal_connect(G_OBJECT(spectrumAverageCheckButton),"toggled",G_CALLBACK(spectrumAverageChanged),NULL);

        // add Spectrum Smoothing
        spectrumSmoothingLabel=gtk_label_new("SpectrumAverage Smoothing");
        gtk_widget_show(spectrumSmoothingLabel);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,spectrumSmoothingLabel,10,360);
        spectrumSmoothingSpinButton=gtk_spin_button_new_with_range(0.0,1.0,0.1);
        gtk_spin_button_set_value((GtkSpinButton*)spectrumSmoothingSpinButton,(double)spectrumAverageSmoothing);
        g_signal_connect(G_OBJECT(spectrumSmoothingSpinButton),"value-changed",G_CALLBACK(spectrumSmoothingChanged),NULL);
        gtk_widget_show(spectrumSmoothingSpinButton);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,spectrumSmoothingSpinButton,250,360);

        // add Bandscope Average checkbox
        bandscopeAverageCheckButton=gtk_check_button_new_with_label("Bandscope Averaging");
        gtk_toggle_button_set_active((GtkToggleButton*)bandscopeAverageCheckButton,(gboolean)bandscopeAverage);
        gtk_widget_show(bandscopeAverageCheckButton);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,bandscopeAverageCheckButton,250,400);
        g_signal_connect(G_OBJECT(bandscopeAverageCheckButton),"toggled",G_CALLBACK(bandscopeAverageChanged),NULL);

        // add Bandscope Smoothing
        bandscopeSmoothingLabel=gtk_label_new("Bandscope Average Smoothing");
        gtk_widget_show(bandscopeSmoothingLabel);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,bandscopeSmoothingLabel,10,440);
        bandscopeSmoothingSpinButton=gtk_spin_button_new_with_range(0.0,1.0,0.1);
        gtk_spin_button_set_value((GtkSpinButton*)bandscopeSmoothingSpinButton,(double)bandscopeAverageSmoothing);
        g_signal_connect(G_OBJECT(bandscopeSmoothingSpinButton),"value-changed",G_CALLBACK(bandscopeSmoothingChanged),NULL);
        gtk_widget_show(bandscopeSmoothingSpinButton);
        gtk_fixed_put((GtkFixed*)setupDisplayFixed,bandscopeSmoothingSpinButton,250,440);

        gtk_widget_set_size_request(GTK_WIDGET(setupDisplayFixed),600,480);
        gtk_widget_show(setupDisplayFixed);

    return setupDisplayFixed;

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Spectrum high changed
* 
* @param spinButton
* @param data
*/
void spectrumHighChanged(GtkSpinButton* spinButton,gpointer data) {
    spectrumMAX=gtk_spin_button_get_value(spinButton);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Spectrum low changed
* 
* @param spinButton
* @param data
*/
void spectrumLowChanged(GtkSpinButton* spinButton,gpointer data) {
    spectrumMIN=gtk_spin_button_get_value(spinButton);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Spectrum step changed
* 
* @param spinButton
* @param data
*/
void spectrumStepChanged(GtkSpinButton* spinButton,gpointer data) {
    spectrumSTEP=gtk_spin_button_get_value(spinButton);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Spectrum rate changed
* 
* @param spinButton
* @param data
*/
void spectrumRateChanged(GtkSpinButton* spinButton,gpointer data) {
    setSpectrumUpdateRate(gtk_spin_button_get_value(spinButton));
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Waterfall high changed
* 
* @param spinButton
* @param data
*/
void waterfallHighChanged(GtkSpinButton* spinButton,gpointer data) {
    waterfallHighThreshold=gtk_spin_button_get_value(spinButton);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Waterfall low changed
* 
* @param spinButton
* @param data
*/
void waterfallLowChanged(GtkSpinButton* spinButton,gpointer data) {
    waterfallLowThreshold=gtk_spin_button_get_value(spinButton);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Bandscope high changed
* 
* @param spinButton
* @param data
*/
void bandscopeHighChanged(GtkSpinButton* spinButton,gpointer data) {
    bandscopeMAX=gtk_spin_button_get_value(spinButton);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Bandscope low changed
* 
* @param spinButton
* @param data
*/
void bandscopeLowChanged(GtkSpinButton* spinButton,gpointer data) {
    bandscopeMIN=gtk_spin_button_get_value(spinButton);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief CW Pitch changed
* 
* @param spinButton
* @param data
*/
void cwPitchChanged(GtkSpinButton* spinButton,gpointer data) {
    cwPitch=gtk_spin_button_get_value(spinButton);
    // could change the filter if in CW mode
    if(mode==modeCWL || mode==modeCWU) {
        setFilter(filter);
    }
}

void spectrumAverageChanged(GtkToggleButton* button,gpointer data) {
    spectrumAverage=gtk_toggle_button_get_active(button);
}

void spectrumSmoothingChanged(GtkSpinButton* spinButton,gpointer data) {
    spectrumAverageSmoothing=gtk_spin_button_get_value(spinButton);
}

void bandscopeAverageChanged(GtkToggleButton* button,gpointer data) {
    bandscopeAverage=gtk_toggle_button_get_active(button);
}

void bandscopeSmoothingChanged(GtkSpinButton* spinButton,gpointer data) {
    bandscopeAverageSmoothing=gtk_spin_button_get_value(spinButton);
}
