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
#include "display_setup.h"
#include "xvtr_setup.h"

GtkWidget* setupWindow=NULL;
GtkWidget* setupNotebook;


void quitSetup();

/* --------------------------------------------------------------------------*/
/** 
* @brief ghpsdr_setup
*/
void ghpsdr_setup() {
    GtkWidget* page;
    if(setupWindow==NULL) {
        setupWindow=gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title((GtkWindow*)setupWindow,(gchar*)"HPSDR: setup");
        g_signal_connect(G_OBJECT(setupWindow),"destroy",G_CALLBACK(quitSetup),NULL);

        setupNotebook=gtk_notebook_new();

        page=displaySetupUI();
        gtk_notebook_append_page(GTK_NOTEBOOK(setupNotebook),page,gtk_label_new("Display"));
        gtk_widget_show(page);

        page=xvtrSetupUI();
        gtk_notebook_append_page(GTK_NOTEBOOK(setupNotebook),page,gtk_label_new("XVTR"));
        gtk_widget_show(page);

        gtk_widget_show(setupNotebook);
        gtk_container_add(GTK_CONTAINER(setupWindow), setupNotebook);
        gtk_widget_show(setupWindow);
    } else {
        gtk_window_set_keep_above(GTK_WINDOW(setupWindow),TRUE);
    }

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Quit setup
*/
void quitSetup() {
    saveXvtrSettings();
    configureXVTRButton();
    gtk_widget_destroy(setupWindow);
    setupWindow=NULL;
}
