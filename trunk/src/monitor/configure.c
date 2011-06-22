/**
* @file configure.c
* @brief configure monitor
* @author John Melton, G0ORX/N6LYT
* @version 0.1
* @date 2009-10-13
*/

/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
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
#include "configure.h"
#include "monitor.h"

static GtkWidget* dialog=NULL;
static GtkWidget* configureBox;
static GtkWidget* receiverSpinButton;
static GtkWidget* waterfallHighSpinButton;
static GtkWidget* waterfallLowSpinButton;
static GtkWidget* updateSpinButton;
static GtkWidget* frequency;

void receiverChanged(GtkSpinButton* spinButton,gpointer data);
void waterfallHighChanged(GtkSpinButton* spinButton,gpointer data);
void waterfallLowChanged(GtkSpinButton* spinButton,gpointer data);
void updateChanged(GtkSpinButton* spinButton,gpointer data);

GtkWidget* configure(int update_receiver) {
    GtkWidget* box;
    GtkWidget* label;
    char temp[16];
    if(dialog==NULL) {
        dialog=gtk_dialog_new_with_buttons("Monitor Configure",NULL,GTK_DIALOG_MODAL,GTK_STOCK_OK,GTK_RESPONSE_ACCEPT,NULL);
        configureBox=gtk_vbox_new(FALSE,5);
        box=gtk_hbox_new(FALSE,2);
        label=gtk_label_new("Receiver: ");
        gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,2);
        if(update_receiver) {
            receiverSpinButton=gtk_spin_button_new_with_range(-1,7,1);
            gtk_box_pack_start(GTK_BOX(box),receiverSpinButton,FALSE,FALSE,2);
            gtk_spin_button_set_value((GtkSpinButton*)receiverSpinButton,get_receiver());
            g_signal_connect(G_OBJECT(receiverSpinButton),"value-changed",G_CALLBACK(receiverChanged),NULL);
        } else {
            sprintf(temp,"%d",get_receiver());
            label=gtk_label_new(temp);
            gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,2);
        }
        gtk_box_pack_start(GTK_BOX(configureBox),box,FALSE,FALSE,2);

        box=gtk_hbox_new(FALSE,2);
        label=gtk_label_new("Waterfall High: ");
        gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,2);
        waterfallHighSpinButton=gtk_spin_button_new_with_range(-200,200,1);
        gtk_box_pack_start(GTK_BOX(box),waterfallHighSpinButton,FALSE,FALSE,2);
        gtk_spin_button_set_value((GtkSpinButton*)waterfallHighSpinButton,(double)get_waterfall_high());
        g_signal_connect(G_OBJECT(waterfallHighSpinButton),"value-changed",G_CALLBACK(waterfallHighChanged),NULL);
        gtk_box_pack_start(GTK_BOX(configureBox),box,FALSE,FALSE,2);

        box=gtk_hbox_new(FALSE,2);
        label=gtk_label_new("Waterfall Low: ");
        gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,2);
        waterfallLowSpinButton=gtk_spin_button_new_with_range(-200,200,1);
        gtk_box_pack_start(GTK_BOX(box),waterfallLowSpinButton,FALSE,FALSE,2);
        gtk_spin_button_set_value((GtkSpinButton*)waterfallLowSpinButton,(double)get_waterfall_low());
        g_signal_connect(G_OBJECT(waterfallLowSpinButton),"value-changed",G_CALLBACK(waterfallLowChanged),NULL);
        gtk_box_pack_start(GTK_BOX(configureBox),box,FALSE,FALSE,2);

        box=gtk_hbox_new(FALSE,2);
        label=gtk_label_new("Update (fps): ");
        gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,2);
        updateSpinButton=gtk_spin_button_new_with_range(1,40,1);
        gtk_box_pack_start(GTK_BOX(box),updateSpinButton,FALSE,FALSE,2);
        gtk_spin_button_set_value((GtkSpinButton*)updateSpinButton,(double)get_update_fps());
        g_signal_connect(G_OBJECT(updateSpinButton),"value-changed",G_CALLBACK(updateChanged),NULL);
        gtk_box_pack_start(GTK_BOX(configureBox),box,FALSE,FALSE,2);

        box=gtk_hbox_new(FALSE,2);
        label=gtk_label_new("Frequency (Hz): ");
        gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,2);
        frequency=gtk_entry_new_with_max_length(12);
        gtk_editable_set_editable(GTK_EDITABLE(frequency),TRUE);
        sprintf(temp,"%ld",get_frequency());
        gtk_entry_set_text(GTK_ENTRY(frequency),temp);
        gtk_box_pack_start(GTK_BOX(box),frequency,FALSE,FALSE,2);
        gtk_box_pack_start(GTK_BOX(configureBox),box,FALSE,FALSE,2);

        gtk_container_add (gtk_dialog_get_content_area (GTK_DIALOG(dialog)),configureBox);

        gtk_widget_show_all(dialog);
    } else {
        gtk_window_set_keep_above(GTK_WINDOW(dialog),TRUE);
    }

    return dialog;
}

void receiverChanged(GtkSpinButton* spinButton,gpointer data) {
    set_receiver(gtk_spin_button_get_value(spinButton));
}

void waterfallHighChanged(GtkSpinButton* spinButton,gpointer data) {
    set_waterfall_high((float)gtk_spin_button_get_value(spinButton));
}

void waterfallLowChanged(GtkSpinButton* spinButton,gpointer data) {
    set_waterfall_low((float)gtk_spin_button_get_value(spinButton));
}

void updateChanged(GtkSpinButton* spinButton,gpointer data) {
    set_update_fps(gtk_spin_button_get_value(spinButton));
}

long configure_get_frequency() {
    return atol(gtk_entry_get_text(GTK_ENTRY(frequency)));
}

void configure_destroy() {
    gtk_widget_destroy(dialog);
    dialog=NULL;
}
