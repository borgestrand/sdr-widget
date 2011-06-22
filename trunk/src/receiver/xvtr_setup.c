/** 
* @file xvtr_setup.c
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
// xvtr_setup.c
//

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "xvtr.h"
#include "band.h"

GtkWidget* xvtrPage;

GtkWidget* buttonLabel[12];
GtkWidget* minRxFrequency[12];
GtkWidget* maxRxFrequency[12];
GtkWidget* loRxFrequency[12];
GtkWidget* minTxFrequency[12];
GtkWidget* maxTxFrequency[12];
GtkWidget* loTxFrequency[12];


void validate_numeric(GtkEntry* entry,const gchar* text,gint length,gint* position,gpointer data) {
  GtkEditable *editable = GTK_EDITABLE(entry);
  int i, count=0;
  gchar *result=g_new(gchar,length);

  for (i=0; i < length; i++) {
    if (!isdigit(text[i]))
      continue;
    result[count++]=text[i];
  }
  
  if (count > 0) {
    g_signal_handlers_block_by_func (G_OBJECT (editable),
                                     G_CALLBACK (validate_numeric),
                                     data);
    gtk_editable_insert_text (editable, result, count, position);
    g_signal_handlers_unblock_by_func (G_OBJECT (editable),
                                       G_CALLBACK (validate_numeric),
                                       data);
  }
  g_signal_stop_emission_by_name (G_OBJECT (editable), "insert_text");

  g_free (result);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief display setup UI
*/
GtkWidget* xvtrSetupUI() {
    GtkWidget* item;
    int i;
    char temp[32];
    XVTR_ENTRY* xvtr_entry;

    xvtrPage=gtk_table_new(14,8,FALSE);

    gtk_table_set_col_spacings(GTK_TABLE(xvtrPage),10);

    item=gtk_label_new("Button");
    gtk_widget_show(item);
    gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,0,1,0,1);
    item=gtk_label_new("Label");
    gtk_widget_show(item);
    gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,1,2,0,1);
    item=gtk_label_new("Min Rx Freq (Hz)");
    gtk_widget_show(item);
    gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,2,3,0,1);
    item=gtk_label_new("Max Rx Freq (Hz)");
    gtk_widget_show(item);
    gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,3,4,0,1);
    item=gtk_label_new("LO Rx Freq (Hz)");
    gtk_widget_show(item);
    gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,4,5,0,1);
    item=gtk_label_new("Min Tx Freq (Hz)");
    gtk_widget_show(item);
    gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,5,6,0,1);
    item=gtk_label_new("Max Tx Freq (Hz)");
    gtk_widget_show(item);
    gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,6,7,0,1);
    item=gtk_label_new("LO Tx Freq (Hz)");
    gtk_widget_show(item);
    gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,7,8,0,1);

    for(i=0;i<12;i++) {
        sprintf(temp,"%d",i);
        item=gtk_label_new(temp);
        gtk_widget_show(item);
        gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,0,1,i+1,i+2);

        xvtr_entry=getXvtrEntry(i);

        item=gtk_entry_new_with_max_length(4);
        gtk_editable_set_editable(GTK_EDITABLE(item),TRUE);
        gtk_entry_set_text(GTK_ENTRY(item),xvtr_entry->name);
        gtk_widget_show(item);
        gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,1,2,i+1,i+2);
        buttonLabel[i]=item;
        
        sprintf(temp,"%lld",xvtr_entry->rxFrequencyMin);
        item=gtk_entry_new_with_max_length(12);
        gtk_editable_set_editable(GTK_EDITABLE(item),TRUE);
        g_signal_connect(G_OBJECT(item),"insert_text",G_CALLBACK(validate_numeric),NULL);
        gtk_entry_set_text(GTK_ENTRY(item),temp);
        gtk_widget_show(item);
        gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,2,3,i+1,i+2);
        minRxFrequency[i]=item;
        
        sprintf(temp,"%lld",xvtr_entry->rxFrequencyMax);
        item=gtk_entry_new_with_max_length(12);
        gtk_editable_set_editable(GTK_EDITABLE(item),TRUE);
        g_signal_connect(G_OBJECT(item),"insert_text",G_CALLBACK(validate_numeric),NULL);
        gtk_entry_set_text(GTK_ENTRY(item),temp);
        gtk_widget_show(item);
        gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,3,4,i+1,i+2);
        maxRxFrequency[i]=item;
        
        sprintf(temp,"%lld",xvtr_entry->rxFrequencyLO);
        item=gtk_entry_new_with_max_length(12);
        gtk_editable_set_editable(GTK_EDITABLE(item),TRUE);
        g_signal_connect(G_OBJECT(item),"insert_text",G_CALLBACK(validate_numeric),NULL);
        gtk_entry_set_text(GTK_ENTRY(item),temp);
        gtk_widget_show(item);
        gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,4,5,i+1,i+2);
        loRxFrequency[i]=item;
        
        sprintf(temp,"%lld",xvtr_entry->txFrequencyMin);
        item=gtk_entry_new_with_max_length(12);
        gtk_editable_set_editable(GTK_EDITABLE(item),TRUE);
        g_signal_connect(G_OBJECT(item),"insert_text",G_CALLBACK(validate_numeric),NULL);
        gtk_entry_set_text(GTK_ENTRY(item),temp);
        gtk_widget_show(item);
        gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,5,6,i+1,i+2);
        minTxFrequency[i]=item;
        
        sprintf(temp,"%lld",xvtr_entry->txFrequencyMax);
        item=gtk_entry_new_with_max_length(12);
        gtk_editable_set_editable(GTK_EDITABLE(item),TRUE);
        g_signal_connect(G_OBJECT(item),"insert_text",G_CALLBACK(validate_numeric),NULL);
        gtk_entry_set_text(GTK_ENTRY(item),temp);
        gtk_widget_show(item);
        gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,6,7,i+1,i+2);
        maxTxFrequency[i]=item;
        
        sprintf(temp,"%lld",xvtr_entry->txFrequencyLO);
        item=gtk_entry_new_with_max_length(12);
        gtk_editable_set_editable(GTK_EDITABLE(item),TRUE);
        g_signal_connect(G_OBJECT(item),"insert_text",G_CALLBACK(validate_numeric),NULL);
        gtk_entry_set_text(GTK_ENTRY(item),temp);
        gtk_widget_show(item);
        gtk_table_attach_defaults(GTK_TABLE(xvtrPage),item,7,8,i+1,i+2);
        loTxFrequency[i]=item;
    }

    gtk_widget_show(xvtrPage);
    return xvtrPage;

}

void saveXvtrSettings() {
    int i;
    const gchar* temp;
    XVTR_ENTRY* xvtr_entry;

    for(i=0;i<12;i++) {
        xvtr_entry=getXvtrEntry(i);
        temp=gtk_entry_get_text(GTK_ENTRY(buttonLabel[i]));
        strcpy(xvtr_entry->name,temp);

        temp=gtk_entry_get_text(GTK_ENTRY(minRxFrequency[i]));
        if(strcmp(temp,"")==0) {
            xvtr_entry->rxFrequencyMin=0LL;
        } else {
            xvtr_entry->rxFrequencyMin=atoll(temp);
        }

        temp=gtk_entry_get_text(GTK_ENTRY(maxRxFrequency[i]));
        if(strcmp(temp,"")==0) {
            xvtr_entry->rxFrequencyMax=0LL;
        } else {
            xvtr_entry->rxFrequencyMax=atoll(temp);
        }

        temp=gtk_entry_get_text(GTK_ENTRY(loRxFrequency[i]));
        if(strcmp(temp,"")==0) {
            xvtr_entry->rxFrequencyLO=0LL;
        } else {
            xvtr_entry->rxFrequencyLO=atoll(temp);
        }

        if((xvtr_entry->rxFrequency<xvtr_entry->rxFrequencyMin) ||
           (xvtr_entry->rxFrequency>xvtr_entry->rxFrequencyMax)) {
            xvtr_entry->rxFrequency=xvtr_entry->rxFrequencyMin;
        }

        temp=gtk_entry_get_text(GTK_ENTRY(minTxFrequency[i]));
        if(strcmp(temp,"")==0) {
            xvtr_entry->txFrequencyMin=0LL;
        } else {
            xvtr_entry->txFrequencyMin=atoll(temp);
        }

        temp=gtk_entry_get_text(GTK_ENTRY(maxTxFrequency[i]));
        if(strcmp(temp,"")==0) {
            xvtr_entry->txFrequencyMax=0LL;
        } else {
            xvtr_entry->txFrequencyMax=atoll(temp);
        }

        temp=gtk_entry_get_text(GTK_ENTRY(loTxFrequency[i]));
        if(strcmp(temp,"")==0) {
            xvtr_entry->txFrequencyLO=0LL;
        } else {
            xvtr_entry->txFrequencyLO=atoll(temp);
        }

        if((xvtr_entry->txFrequency<xvtr_entry->txFrequencyMin) ||
           (xvtr_entry->txFrequency>xvtr_entry->txFrequencyMax)) {
            xvtr_entry->txFrequency=xvtr_entry->txFrequencyMin;
        }

    }
}

