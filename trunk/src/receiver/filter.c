/** 
* @file filter.c
* @brief Filter functions
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
#include "cw.h"
#include "filter.h"
#include "main.h"
#include "property.h"
#include "mode.h"


int filterLow;
int filterHigh;

int txFilterLowCut=300;
int txFilterHighCut=3000;

int filterVar1Low;
int filterVar1High;
int filterVar2Low;
int filterVar2High;

gint filterTimerId;

int filter;

GtkWidget* filterFrame;
GtkWidget* filterTable;

GtkWidget* buttonHighPlus;
GtkWidget* filterHighDisplay;
GdkPixmap* filterHighDisplayPixmap;
GtkWidget* buttonHighMinus;
GtkWidget* buttonLowPlus;
GtkWidget* filterLowDisplay;
GdkPixmap* filterLowDisplayPixmap;
GtkWidget* buttonLowMinus;
GtkWidget* buttonVar1;
GtkWidget* buttonVar2;

GtkWidget* buttonF0;
GtkWidget* buttonF1;
GtkWidget* buttonF2;
GtkWidget* buttonF3;
GtkWidget* buttonF4;
GtkWidget* buttonF5;
GtkWidget* buttonF6;
GtkWidget* buttonF7;
GtkWidget* buttonF8;
GtkWidget* buttonF9;
GtkWidget* currentFilterButton=NULL;

gint filterRootX=0;
gint filterRootY=0;

FILTER filterLSB[FILTERS]={
    {-5150,-150,"5.0k"},
    {-4550,-150,"4.4k"},
    {-3950,-150,"3.8k"},
    {-3450,-150,"3.3k"},
    {-3050,-150,"2.9k"},
    {-2850,-150,"2.7k"},
    {-2550,-150,"2.4k"},
    {-2250,-150,"2.1k"},
    {-1950,-150,"1.8k"},
    {-1150,-150,"1.0k"},
    {-2850,-150,"Var1"},
    {-2850,-150,"Var2"}
    };

FILTER filterDIGL[FILTERS]={
    {-5150,-150,"5.0k"},
    {-4550,-150,"4.4k"},
    {-3950,-150,"3.8k"},
    {-3450,-150,"3.3k"},
    {-3050,-150,"2.9k"},
    {-2850,-150,"2.7k"},
    {-2550,-150,"2.4k"},
    {-2250,-150,"2.1k"},
    {-1950,-150,"1.8k"},
    {-1150,-150,"1.0k"},
    {-2850,-150,"Var1"},
    {-2850,-150,"Var2"}
    };

FILTER filterUSB[FILTERS]={
    {150,5150,"5.0k"},
    {150,4550,"4.4k"},
    {150,3950,"3.8k"},
    {150,3450,"3.3k"},
    {150,3050,"2.9k"},
    {150,2850,"2.7k"},
    {150,2550,"2.4k"},
    {150,2250,"2.1k"},
    {150,1950,"1.8k"},
    {150,1150,"1.0k"},
    {150,2850,"Var1"},
    {150,2850,"Var2"}
    };

FILTER filterDIGU[FILTERS]={
    {150,5150,"5.0k"},
    {150,4550,"4.4k"},
    {150,3950,"3.8k"},
    {150,3450,"3.3k"},
    {150,3050,"2.9k"},
    {150,2850,"2.7k"},
    {150,2550,"2.4k"},
    {150,2250,"2.1k"},
    {150,1950,"1.8k"},
    {150,1150,"1.0k"},
    {150,2850,"Var1"},
    {150,2850,"Var2"}
    };

FILTER filterCWL[FILTERS]={
    {500,500,"1.0k"},
    {400,400,"800"},
    {375,375,"750"},
    {300,300,"600"},
    {250,250,"500"},
    {200,200,"400"},
    {125,125,"250"},
    {50,50,"100"},
    {25,25,"50"},
    {13,13,"25"},
    {250,250,"Var1"},
    {250,250,"Var2"}
    };

FILTER filterCWU[FILTERS]={
    {500,500,"1.0k"},
    {400,400,"800"},
    {375,375,"750"},
    {300,300,"600"},
    {250,250,"500"},
    {200,200,"400"},
    {125,125,"250"},
    {50,50,"100"},
    {25,25,"50"},
    {13,13,"25"},
    {250,250,"Var1"},
    {250,250,"Var2"}
    };

FILTER filterAM[FILTERS]={
    {-8000,8000,"16k"},
    {-6000,6000,"12k"},
    {-5000,5000,"10k"},
    {-4000,4000,"8k"},
    {-3300,3300,"6.6k"},
    {-2600,2600,"5.2k"},
    {-2000,2000,"4.0k"},
    {-1550,1550,"3.1k"},
    {-1450,1450,"2.9k"},
    {-1200,1200,"2.4k"},
    {-3300,3300,"Var1"},
    {-3300,3300,"Var2"}
    };

FILTER filterSAM[FILTERS]={
    {-8000,8000,"16k"},
    {-6000,6000,"12k"},
    {-5000,5000,"10k"},
    {-4000,4000,"8k"},
    {-3300,3300,"6.6k"},
    {-2600,2600,"5.2k"},
    {-2000,2000,"4.0k"},
    {-1550,1550,"3.1k"},
    {-1450,1450,"2.9k"},
    {-1200,1200,"2.4k"},
    {-3300,3300,"Var1"},
    {-3300,3300,"Var2"}
    };

FILTER filterFMN[FILTERS]={
    {-8000,8000,"16k"},
    {-6000,6000,"12k"},
    {-5000,5000,"10k"},
    {-4000,4000,"8k"},
    {-3300,3300,"6.6k"},
    {-2600,2600,"5.2k"},
    {-2000,2000,"4.0k"},
    {-1550,1550,"3.1k"},
    {-1450,1450,"2.9k"},
    {-1200,1200,"2.4k"},
    {-3300,3300,"Var1"},
    {-3300,3300,"Var2"}
    };

FILTER filterDSB[FILTERS]={
    {-8000,8000,"16k"},
    {-6000,6000,"12k"},
    {-5000,5000,"10k"},
    {-4000,4000,"8k"},
    {-3300,3300,"6.6k"},
    {-2600,2600,"5.2k"},
    {-2000,2000,"4.0k"},
    {-1550,1550,"3.1k"},
    {-1450,1450,"2.9k"},
    {-1200,1200,"2.4k"},
    {-3300,3300,"Var1"},
    {-3300,3300,"Var2"}
    };

/* --------------------------------------------------------------------------*/
/** 
* @brief Determine the maximum of two numbers.
* 
* @param a
* @param b
* 
* @return 
*/
int max(int a,int b) {
   if(a>b) return a;
   return b;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Determine the minimum of two numbers.
* 
* @param a
* @param b
* 
* @return 
*/
int min(int a,int b) {
   if(a<b) return a;
   return b;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Draw the low filter
* 
* @param queue
*/
void drawFilterLow(gboolean queue) {
    GdkGC* gc;
    PangoContext *context;
    PangoLayout *layout;
    char temp[128];
    if(filterLowDisplay) {
        if(filterLowDisplayPixmap) g_object_unref(filterLowDisplayPixmap);

        filterLowDisplayPixmap=gdk_pixmap_new(filterLowDisplay->window,filterLowDisplay->allocation.width,filterLowDisplay->allocation.height,-1);

        gc=gdk_gc_new(filterLowDisplay->window);
        gdk_gc_set_rgb_fg_color(gc,&black);
        gdk_draw_rectangle(filterLowDisplayPixmap,
                           gc,
                           TRUE,
                           0,0,
                           filterLowDisplay->allocation.width,
                           filterLowDisplay->allocation.height);

        context = gdk_pango_context_get_for_screen (gdk_screen_get_default ());
        layout = pango_layout_new (context);
        pango_layout_set_width(layout,filterLowDisplay->allocation.width*PANGO_SCALE);
        pango_layout_set_alignment(layout,PANGO_ALIGN_RIGHT);
        sprintf(temp,"<span foreground='#7AAA6E' background='#000000' font_desc='Sans Bold 10'>Low   % 5d </span>",filterLow);
        pango_layout_set_markup (layout,temp, -1);
        gdk_draw_layout(GDK_DRAWABLE(filterLowDisplayPixmap),gc,0,2,layout);

        g_object_unref(context);
        g_object_unref(layout);
        g_object_unref(gc);

        if(queue) gtk_widget_queue_draw(filterLowDisplay);
    }

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Draw the high filter.
* 
* @param queue
*/
void drawFilterHigh(gboolean queue) {
    GdkGC* gc;
    PangoContext *context;
    PangoLayout *layout;
    char temp[128];

    if(filterHighDisplay) {
        if(filterHighDisplayPixmap) g_object_unref(filterHighDisplayPixmap);

        filterHighDisplayPixmap=gdk_pixmap_new(filterHighDisplay->window,filterHighDisplay->allocation.width,filterHighDisplay->allocation.height,-1);

        gc=gdk_gc_new(filterHighDisplay->window);
        gdk_gc_set_rgb_fg_color(gc,&black);
        gdk_draw_rectangle(filterHighDisplayPixmap,
                           gc,
                           TRUE,
                           0,0,
                           filterHighDisplay->allocation.width,
                           filterHighDisplay->allocation.height);

        context = gdk_pango_context_get_for_screen (gdk_screen_get_default ());
        layout = pango_layout_new (context);
        pango_layout_set_width(layout,filterHighDisplay->allocation.width*PANGO_SCALE);
        pango_layout_set_alignment(layout,PANGO_ALIGN_RIGHT);
        sprintf(temp,"<span foreground='#7AAA6E' background='#000000' font_desc='Sans Bold 10'>High  % 5d </span>",filterHigh);
        pango_layout_set_markup (layout,temp, -1);
        gdk_draw_layout(GDK_DRAWABLE(filterHighDisplayPixmap),gc,0,2,layout);

        g_object_unref(context);
        g_object_unref(layout);
        g_object_unref(gc);

        if(queue) gtk_widget_queue_draw(filterHighDisplay);
    }

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Save variable filter settings
* 
*/

void saveVariableFilters() {
        switch(mode) {
            case modeLSB:
                if(filter==filterVar1) {
                    filterLSB[filterVar1].high=filterHigh;
                    filterLSB[filterVar1].low=filterLow;
                } else if(filter==filterVar2) {
                    filterLSB[filterVar2].high=filterHigh;
                    filterLSB[filterVar2].low=filterLow;
                }
                break;
            case modeDIGL:
                if(filter==filterVar1) {
                    filterDIGL[filterVar1].high=filterHigh;
                    filterDIGL[filterVar1].low=filterLow;
                } else if(filter==filterVar2) {
                    filterDIGL[filterVar2].high=filterHigh;
                    filterDIGL[filterVar2].low=filterLow;
                }
                break;
            case modeUSB:
                if(filter==filterVar1) {
                    filterUSB[filterVar1].high=filterHigh;
                    filterUSB[filterVar1].low=filterLow;
                } else if(filter==filterVar2) {
                    filterUSB[filterVar2].high=filterHigh;
                    filterUSB[filterVar2].low=filterLow;
                }
                break;
            case modeDIGU:
                if(filter==filterVar1) {
                    filterDIGU[filterVar1].high=filterHigh;
                    filterDIGU[filterVar1].low=filterLow;
                } else if(filter==filterVar2) {
                    filterDIGU[filterVar2].high=filterHigh;
                    filterDIGU[filterVar2].low=filterLow;
                }
                break;
            case modeCWL:
                if(filter==filterVar1) {
                    filterCWL[filterVar1].high=-(-cwPitch-filterHigh);
                    filterCWL[filterVar1].low=-(filterLow+cwPitch);
                } else if(filter==filterVar2) {
                    filterCWL[filterVar2].high=-(-cwPitch-filterHigh);
                    filterCWL[filterVar2].low=-(filterLow+cwPitch);
                }
                break;
            case modeCWU:
                if(filter==filterVar1) {
                    filterCWU[filterVar1].high=filterHigh-cwPitch;
                    filterCWU[filterVar1].low=cwPitch-filterLow;
                } else if(filter==filterVar2) {
                    filterCWU[filterVar2].high=filterHigh-cwPitch;
                    filterCWU[filterVar2].low=cwPitch-filterLow;
                }
                break;
            case modeDSB:
                if(filter==filterVar1) {
                    filterDSB[filterVar1].high=filterHigh;
                    filterDSB[filterVar1].low=filterLow;
                } else if(filter==filterVar2) {
                    filterDSB[filterVar2].high=filterHigh;
                    filterDSB[filterVar2].low=filterLow;
                }
                break;
            case modeAM:
                if(filter==filterVar1) {
                    filterAM[filterVar1].high=filterHigh;
                    filterAM[filterVar1].low=filterLow;
                } else if(filter==filterVar2) {
                    filterAM[filterVar2].high=filterHigh;
                    filterAM[filterVar2].low=filterLow;
                }
                break;
            case modeFMN:
                if(filter==filterVar1) {
                    filterFMN[filterVar1].high=filterHigh;
                    filterFMN[filterVar1].low=filterLow;
                } else if(filter==filterVar2) {
                    filterFMN[filterVar2].high=filterHigh;
                    filterFMN[filterVar2].low=filterLow;
                }
                break;
            case modeSAM:
                if(filter==filterVar1) {
                    filterSAM[filterVar1].high=filterHigh;
                    filterSAM[filterVar1].low=filterLow;
                } else if(filter==filterVar2) {
                    filterSAM[filterVar2].high=filterHigh;
                    filterSAM[filterVar2].low=filterLow;
                }
                break;
        }
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Select the filter
* 
* @param widget
*/
void selectFilter(GtkWidget* widget) {
    GtkWidget* label;
    char temp[128];


    if(currentFilterButton) {
        // reset the button state
        label=gtk_bin_get_child((GtkBin*)currentFilterButton);
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &black);

        // save variable filter setting
        saveVariableFilters();
    }
    if(widget) {
        label=gtk_bin_get_child((GtkBin*)widget);
        gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &buttonSelected);
        gtk_widget_modify_fg(label, GTK_STATE_PRELIGHT, &buttonSelected);
        currentFilterButton=widget;
    }

    if(widget==buttonF0) {
        filter=filterF0;
    } else if(widget==buttonF1) {
        filter=filterF1;
    } else if(widget==buttonF2) {
        filter=filterF2;
    } else if(widget==buttonF3) {
        filter=filterF3;
    } else if(widget==buttonF4) {
        filter=filterF4;
    } else if(widget==buttonF5) {
        filter=filterF5;
    } else if(widget==buttonF6) {
        filter=filterF6;
    } else if(widget==buttonF7) {
        filter=filterF7;
    } else if(widget==buttonF8) {
        filter=filterF8;
    } else if(widget==buttonF9) {
        filter=filterF9;
    } else if(widget==buttonVar1) {
        filter=filterVar1;
    } else if(widget==buttonVar2) {
        filter=filterVar2;
    }

    switch(mode) {
        case modeLSB:
            filterLow=filterLSB[filter].low;
            filterHigh=filterLSB[filter].high;
            break;
        case modeDIGL:
            filterLow=filterDIGL[filter].low;
            filterHigh=filterDIGL[filter].high;
            break;
        case modeUSB:
            filterLow=filterUSB[filter].low;
            filterHigh=filterUSB[filter].high;
            break;
        case modeDIGU:
            filterLow=filterDIGU[filter].low;
            filterHigh=filterDIGU[filter].high;
            break;
        case modeCWL:
            filterLow=-cwPitch-filterCWL[filter].low;
            filterHigh=-cwPitch+filterCWL[filter].high;
            break;
        case modeCWU:
            filterLow=cwPitch-filterCWU[filter].low;
            filterHigh=cwPitch+filterCWU[filter].high;
            break;
        case modeDSB:
            filterLow=filterDSB[filter].low;
            filterHigh=filterDSB[filter].high;
            break;
        case modeAM:
            filterLow=filterAM[filter].low;
            filterHigh=filterAM[filter].high;
            break;
        case modeFMN:
            filterLow=filterFMN[filter].low;
            filterHigh=filterFMN[filter].high;
            break;
        case modeSAM:
            filterLow=filterSAM[filter].low;
            filterHigh=filterSAM[filter].high;
            break;
        default:
            filterLow=-6000;
            filterHigh=6000;
            break;
    }
    
    //sprintf(temp,"setFilter %d %d",filterLow,filterHigh);
    //writeCommand(temp);
    SetRXFilter(0,0,(double)filterLow,(double)filterHigh);
    SetRXFilter(0,1,(double)filterLow,(double)filterHigh);
    SetTXFilter(1,(double)filterLow,(double)filterHigh);

    drawFilterHigh(TRUE);
    drawFilterLow(TRUE);

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the filter.
* 
* @param f
*/
void setFilter(int f) {
    GtkWidget* widget;
        switch(f) {
            case filterF0:
                widget=buttonF0;
                break;
            case filterF1:
                widget=buttonF1;
                break;
            case filterF2:
                widget=buttonF2;
                break;
            case filterF3:
                widget=buttonF3;
                break;
            case filterF4:
                widget=buttonF4;
                break;
            case filterF5:
                widget=buttonF5;
                break;
            case filterF6:
                widget=buttonF6;
                break;
            case filterF7:
                widget=buttonF7;
                break;
            case filterF8:
                widget=buttonF8;
                break;
            case filterF9:
                widget=buttonF9;
                break;
            case filterVar1:
                widget=buttonVar1;
                break;
            case filterVar2:
                widget=buttonVar2;
                break;
        }
        selectFilter(widget);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Filter button callback
* 
* @param widget
* @param data
*/
void filterButtonCallback(GtkWidget* widget,gpointer data) {
    selectFilter(widget);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Filter high display configure event 
* 
* @param widget
* @param event
* 
* @return 
*/
gboolean filterHighDisplay_configure_event(GtkWidget* widget,GdkEventConfigure* event) {
    GdkGC* gc;
    PangoContext *context;
    PangoLayout *layout;
    char temp[128];

    if(filterHighDisplayPixmap) g_object_unref(filterHighDisplayPixmap);

    filterHighDisplayPixmap=gdk_pixmap_new(widget->window,widget->allocation.width,widget->allocation.height,-1);

    drawFilterHigh(FALSE);

    return TRUE;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Filter high display expose event 
* 
* @param widget
* @param event
* 
* @return 
*/
gboolean filterHighDisplay_expose_event(GtkWidget* widget,GdkEventExpose* event) {
    gdk_draw_drawable(widget->window,
                    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                    filterHighDisplayPixmap,
                    event->area.x, event->area.y,
                    event->area.x, event->area.y,
                    event->area.width, event->area.height);
    return FALSE;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Filter low display configure event
* 
* @param widget
* @param event
* 
* @return 
*/
gboolean filterLowDisplay_configure_event(GtkWidget* widget,GdkEventConfigure* event) {
    GdkGC* gc;
    PangoContext *context;
    PangoLayout *layout;
    char temp[128];

    if(filterLowDisplayPixmap) g_object_unref(filterLowDisplayPixmap);

    filterLowDisplayPixmap=gdk_pixmap_new(widget->window,widget->allocation.width,widget->allocation.height,-1);

    drawFilterLow(FALSE);
    return TRUE;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Filter low display expose event
* 
* @param widget
* @param event
* 
* @return 
*/
gboolean filterLowDisplay_expose_event(GtkWidget* widget,GdkEventExpose* event) {
    gdk_draw_drawable(widget->window,
                    widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                    filterLowDisplayPixmap,
                    event->area.x, event->area.y,
                    event->area.x, event->area.y,
                    event->area.width, event->area.height);
    return FALSE;
}


/* --------------------------------------------------------------------------*/
/** 
* @brief Update filter
* 
* @param widget
*/
void updateFilter(GtkWidget* widget) {
    char temp[80];
    switch(filter) {
        case filterVar1:
            if(widget==buttonHighPlus) {
                filterVar1High+=10;
                filterHigh=filterVar1High;
                drawFilterHigh(TRUE);
            } else if(widget==buttonHighMinus) {
                filterVar1High-=10;
                filterHigh=filterVar1High;
                drawFilterHigh(TRUE);
            } else if(widget==buttonLowPlus) {
                filterVar1Low+=10;
                filterLow=filterVar1Low;
                drawFilterLow(TRUE);
            } else if(widget==buttonLowMinus) {
                filterVar1Low-=10;
                filterLow=filterVar1Low;
                drawFilterLow(TRUE);
            }
            break;
        case filterVar2:
            if(widget==buttonHighPlus) {
                filterVar2High+=10;
                filterHigh=filterVar2High;
                drawFilterHigh(TRUE);
            } else if(widget==buttonHighMinus) {
                filterVar2High-=10;
                filterHigh=filterVar2High;
                drawFilterHigh(TRUE);
            } else if(widget==buttonLowPlus) {
                filterVar2Low+=10;
                filterLow=filterVar2Low;
                drawFilterLow(TRUE);
            } else if(widget==buttonLowMinus) {
                filterVar2Low-=10;
                filterLow=filterVar2Low;
                drawFilterLow(TRUE);
            }
            break;
    }
    //sprintf(temp,"setFilter %d %d",filterLow,filterHigh);
    //writeCommand(temp);
    SetRXFilter(0,0,(double)filterLow,(double)filterHigh);
    SetRXFilter(0,1,(double)filterLow,(double)filterHigh);
    SetTXFilter(1,(double)filterLow,(double)filterHigh);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Filter increment timer
* 
* @param widget
* 
* @return 
*/
gint filterIncrementTimer(gpointer widget) {
    // stop the current timer
    gtk_timeout_remove(filterTimerId);        
    // update the filter
    updateFilter((GtkWidget*)widget);
    // start a timer
    filterTimerId=gtk_timeout_add(50,filterIncrementTimer,widget);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Filter increment callback
* 
* @param widget
* @param data
* 
* @return 
*/
gint filterIncrementCallback(GtkWidget* widget,gpointer data) {
    // increment/decrement the filter high/low
    updateFilter(widget);
    // start a timer
    filterTimerId=gtk_timeout_add(500,filterIncrementTimer,widget);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Filter released callback 
* 
* @param widget
* @param data
*/
void filterReleasedCallback(GtkWidget* widget,gpointer data) {
    // stop the timer
    gtk_timeout_remove(filterTimerId);        
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Filter scoll event 
* 
* @param widget
* @param event
* 
* @return 
*/
gboolean filter_scroll_event(GtkWidget* widget,GdkEventScroll* event) {
    char temp[80];
    switch(filter) {
        case filterVar1:
            if(event->direction==GDK_SCROLL_UP) {
                if(widget==filterHighDisplay) {
                    filterVar1High+=10;
                    filterHigh=filterVar1High;
                    drawFilterHigh(TRUE);
                } else if(widget==filterLowDisplay) {
                    filterVar1Low+=10;
                    filterLow=filterVar1Low;
                    drawFilterLow(TRUE);
                }
            } else {
                if(widget==filterHighDisplay) {
                    filterVar1High-=10;
                    filterHigh=filterVar1High;
                    drawFilterHigh(TRUE);
                } else if(widget==filterLowDisplay) {
                    filterVar1Low-=10;
                    filterLow=filterVar1Low;
                    drawFilterLow(TRUE);
                }
            }
            break;
        case filterVar2:
            if(event->direction==GDK_SCROLL_UP) {
                if(widget==filterHighDisplay) {
                    filterVar2High+=10;
                    filterHigh=filterVar1High;
                    drawFilterHigh(TRUE);
                } else if(widget==filterLowDisplay) {
                    filterVar2Low+=10;
                    filterLow=filterVar1Low;
                    drawFilterLow(TRUE);
                }
            } else {
                if(widget==filterHighDisplay) {
                    filterVar2High-=10;
                    filterHigh=filterVar1High;
                    drawFilterHigh(TRUE);
                } else if(widget==filterLowDisplay) {
                    filterVar2Low-=10;
                    filterLow=filterVar1Low;
                    drawFilterLow(TRUE);
                }
            }
            break;
    }
    //sprintf(temp,"setFilter %d %d",filterLow,filterHigh);
    //writeCommand(temp);
    SetRXFilter(0,0,(double)filterLow,(double)filterHigh);
    SetRXFilter(0,1,(double)filterLow,(double)filterHigh);
    SetTXFilter(1,(double)filterLow,(double)filterHigh);
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Build Filter User Interface 
* 
* @return 
*/
GtkWidget* buildFilterUI() {

    GtkWidget* label;

    filterFrame=gtk_frame_new("Filter");
    gtk_widget_modify_bg(filterFrame,GTK_STATE_NORMAL,&background);
    gtk_widget_modify_fg(gtk_frame_get_label_widget(GTK_FRAME(filterFrame)),GTK_STATE_NORMAL,&white);

    filterTable=gtk_table_new(3,4,TRUE);

    // filter settings
    buttonHighPlus = gtk_button_new_with_label ("+");
    gtk_widget_modify_bg(buttonHighPlus, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonHighPlus);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonHighPlus),20,25);
    g_signal_connect(G_OBJECT(buttonHighPlus),"pressed",G_CALLBACK(filterIncrementCallback),NULL);
    g_signal_connect(G_OBJECT(buttonHighPlus),"released",G_CALLBACK(filterReleasedCallback),NULL);
    gtk_widget_show(buttonHighPlus);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonHighPlus,0,1,3,4);

    filterHighDisplay=gtk_drawing_area_new();
    gtk_widget_set_size_request(GTK_WIDGET(filterHighDisplay),60,25);
    gtk_widget_show(filterHighDisplay);
    g_signal_connect(G_OBJECT (filterHighDisplay),"configure_event",G_CALLBACK(filterHighDisplay_configure_event),NULL);
    g_signal_connect(G_OBJECT (filterHighDisplay),"expose_event",G_CALLBACK(filterHighDisplay_expose_event),NULL);
    g_signal_connect(G_OBJECT(filterHighDisplay),"scroll_event",G_CALLBACK(filter_scroll_event),NULL);
    gtk_widget_set_events(filterHighDisplay,GDK_EXPOSURE_MASK|GDK_SCROLL_MASK);
    gtk_widget_show(filterHighDisplay);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),filterHighDisplay,1,3,3,4);

    buttonHighMinus = gtk_button_new_with_label ("-");
    gtk_widget_modify_bg(buttonHighMinus, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonHighMinus);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonHighMinus),20,25);
    g_signal_connect(G_OBJECT(buttonHighMinus),"pressed",G_CALLBACK(filterIncrementCallback),NULL);
    g_signal_connect(G_OBJECT(buttonHighMinus),"released",G_CALLBACK(filterReleasedCallback),NULL);
    gtk_widget_show(buttonHighMinus);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonHighMinus,3,4,3,4);

    buttonLowPlus = gtk_button_new_with_label ("+");
    gtk_widget_modify_bg(buttonLowPlus, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonLowPlus);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonLowPlus),20,25);
    g_signal_connect(G_OBJECT(buttonLowPlus),"pressed",G_CALLBACK(filterIncrementCallback),NULL);
    g_signal_connect(G_OBJECT(buttonLowPlus),"released",G_CALLBACK(filterReleasedCallback),NULL);
    gtk_widget_show(buttonLowPlus);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonLowPlus,0,1,4,5);

    filterLowDisplay=gtk_drawing_area_new();
    gtk_widget_set_size_request(GTK_WIDGET(filterLowDisplay),60,25);
    gtk_widget_show(filterLowDisplay);
    g_signal_connect(G_OBJECT (filterLowDisplay),"configure_event",G_CALLBACK(filterLowDisplay_configure_event),NULL);
    g_signal_connect(G_OBJECT (filterLowDisplay),"expose_event",G_CALLBACK(filterLowDisplay_expose_event),NULL);
    g_signal_connect(G_OBJECT(filterLowDisplay),"scroll_event",G_CALLBACK(filter_scroll_event),NULL);
    gtk_widget_set_events(filterLowDisplay,GDK_EXPOSURE_MASK|GDK_SCROLL_MASK);
    gtk_widget_show(filterLowDisplay);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),filterLowDisplay,1,3,4,5);

    buttonLowMinus = gtk_button_new_with_label ("-");
    gtk_widget_modify_bg(buttonLowMinus, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonLowMinus);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonLowMinus),20,25);
    g_signal_connect(G_OBJECT(buttonLowMinus),"pressed",G_CALLBACK(filterIncrementCallback),NULL);
    g_signal_connect(G_OBJECT(buttonLowMinus),"released",G_CALLBACK(filterReleasedCallback),NULL);
    gtk_widget_show(buttonLowMinus);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonLowMinus,3,4,4,5);

    buttonVar1 = gtk_button_new_with_label ("VAR1");
    gtk_widget_modify_bg(buttonVar1, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonVar1);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonVar1),50,25);
    g_signal_connect(G_OBJECT(buttonVar1),"clicked",G_CALLBACK(filterButtonCallback),NULL);
    gtk_widget_show(buttonVar1);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonVar1,2,3,2,3);

    buttonVar2 = gtk_button_new_with_label ("VAR2");
    gtk_widget_modify_bg(buttonVar2, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonVar2);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonVar2),50,25);
    g_signal_connect(G_OBJECT(buttonVar2),"clicked",G_CALLBACK(filterButtonCallback),NULL);
    gtk_widget_show(buttonVar2);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonVar2,3,4,2,3);


    // predefined filters
    buttonF0 = gtk_button_new_with_label ("6.0K");
    gtk_widget_modify_bg(buttonF0, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonF0);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonF0),50,25);
    g_signal_connect(G_OBJECT(buttonF0),"clicked",G_CALLBACK(filterButtonCallback),NULL);
    gtk_widget_show(buttonF0);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonF0,0,1,0,1);

    buttonF1 = gtk_button_new_with_label ("4.0K");
    gtk_widget_modify_bg(buttonF1, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonF1);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonF1),50,25);
    g_signal_connect(G_OBJECT(buttonF1),"clicked",G_CALLBACK(filterButtonCallback),NULL);
    gtk_widget_show(buttonF1);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonF1,1,2,0,1);

    buttonF2 = gtk_button_new_with_label ("2.6K");
    gtk_widget_modify_bg(buttonF2, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonF2);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonF2),50,25);
    g_signal_connect(G_OBJECT(buttonF2),"clicked",G_CALLBACK(filterButtonCallback),NULL);
    gtk_widget_show(buttonF2);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonF2,2,3,0,1);

    buttonF3 = gtk_button_new_with_label ("2.1K");
    gtk_widget_modify_bg(buttonF3, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonF3);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonF3),50,25);
    g_signal_connect(G_OBJECT(buttonF3),"clicked",G_CALLBACK(filterButtonCallback),NULL);
    gtk_widget_show(buttonF3);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonF3,3,4,0,1);

    buttonF4 = gtk_button_new_with_label ("1.0K");
    gtk_widget_modify_bg(buttonF4, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonF4);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonF4),50,25);
    g_signal_connect(G_OBJECT(buttonF4),"clicked",G_CALLBACK(filterButtonCallback),NULL);
    gtk_widget_show(buttonF4);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonF4,0,1,1,2);

    buttonF5 = gtk_button_new_with_label ("500");
    gtk_widget_modify_bg(buttonF5, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonF5);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonF5),50,25);
    g_signal_connect(G_OBJECT(buttonF5),"clicked",G_CALLBACK(filterButtonCallback),NULL);
    gtk_widget_show(buttonF5);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonF5,1,2,1,2);

    buttonF6 = gtk_button_new_with_label ("250");
    gtk_widget_modify_bg(buttonF6, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonF6);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonF6),50,25);
    g_signal_connect(G_OBJECT(buttonF6),"clicked",G_CALLBACK(filterButtonCallback),NULL);
    gtk_widget_show(buttonF6);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonF6,2,3,1,2);

    buttonF7 = gtk_button_new_with_label ("100");
    gtk_widget_modify_bg(buttonF7, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonF7);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonF7),50,25);
    g_signal_connect(G_OBJECT(buttonF7),"clicked",G_CALLBACK(filterButtonCallback),NULL);
    gtk_widget_show(buttonF7);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonF7,3,4,1,2);

    buttonF8 = gtk_button_new_with_label ("50");
    gtk_widget_modify_bg(buttonF8, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonF8);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonF8),50,25);
    g_signal_connect(G_OBJECT(buttonF8),"clicked",G_CALLBACK(filterButtonCallback),NULL);
    gtk_widget_show(buttonF8);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonF8,0,1,2,3);

    buttonF9 = gtk_button_new_with_label ("25");
    gtk_widget_modify_bg(buttonF9, GTK_STATE_NORMAL, &buttonBackground);
    label=gtk_bin_get_child((GtkBin*)buttonF9);
    gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
    gtk_widget_set_size_request(GTK_WIDGET(buttonF9),50,25);
    g_signal_connect(G_OBJECT(buttonF9),"clicked",G_CALLBACK(filterButtonCallback),NULL);
    gtk_widget_show(buttonF9);
    gtk_table_attach_defaults(GTK_TABLE(filterTable),buttonF9,1,2,2,3);

    gtk_container_add(GTK_CONTAINER(filterFrame),filterTable);
    gtk_widget_show(filterTable);
    gtk_widget_show(filterFrame);

    return filterFrame;
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Save the filter state
*/
void filterSaveState() {
    char value[128];

    saveVariableFilters();

    // save the current filter selection
    sprintf(value,"%d",filter);
    setProperty("filter",value);

    // save the Var1 and Var2 settings
    sprintf(value,"%d",filterLSB[filterVar1].low);
    setProperty("filter.lsb.var1.low",value);
    sprintf(value,"%d",filterLSB[filterVar1].high);
    setProperty("filter.lsb.var1.high",value);
    sprintf(value,"%d",filterLSB[filterVar2].low);
    setProperty("filter.lsb.var2.low",value);
    sprintf(value,"%d",filterLSB[filterVar2].high);
    setProperty("filter.lsb.var2.high",value);
    
    sprintf(value,"%d",filterDIGL[filterVar1].low);
    setProperty("filter.digl.var1.low",value);
    sprintf(value,"%d",filterDIGL[filterVar1].high);
    setProperty("filter.digl.var1.high",value);
    sprintf(value,"%d",filterDIGL[filterVar2].low);
    setProperty("filter.digl.var2.low",value);
    sprintf(value,"%d",filterDIGL[filterVar2].high);
    setProperty("filter.digl.var2.high",value);
    
    sprintf(value,"%d",filterCWL[filterVar1].low);
    setProperty("filter.cwl.var1.low",value);
    sprintf(value,"%d",filterCWL[filterVar1].high);
    setProperty("filter.cwl.var1.high",value);
    sprintf(value,"%d",filterCWL[filterVar2].low);
    setProperty("filter.cwl.var2.low",value);
    sprintf(value,"%d",filterCWL[filterVar2].high);
    setProperty("filter.cwl.var2.high",value);
    
    sprintf(value,"%d",filterUSB[filterVar1].low);
    setProperty("filter.usb.var1.low",value);
    sprintf(value,"%d",filterUSB[filterVar1].high);
    setProperty("filter.usb.var1.high",value);
    sprintf(value,"%d",filterUSB[filterVar2].low);
    setProperty("filter.usb.var2.low",value);
    sprintf(value,"%d",filterUSB[filterVar2].high);
    setProperty("filter.usb.var2.high",value);
    
    sprintf(value,"%d",filterDIGU[filterVar1].low);
    setProperty("filter.digu.var1.low",value);
    sprintf(value,"%d",filterDIGU[filterVar1].high);
    setProperty("filter.digu.var1.high",value);
    sprintf(value,"%d",filterDIGU[filterVar2].low);
    setProperty("filter.digu.var2.low",value);
    sprintf(value,"%d",filterDIGU[filterVar2].high);
    setProperty("filter.digu.var2.high",value);
    
    sprintf(value,"%d",filterCWU[filterVar1].low);
    setProperty("filter.cwu.var1.low",value);
    sprintf(value,"%d",filterCWU[filterVar1].high);
    setProperty("filter.cwu.var1.high",value);
    sprintf(value,"%d",filterCWU[filterVar2].low);
    setProperty("filter.cwu.var2.low",value);
    sprintf(value,"%d",filterCWU[filterVar2].high);
    setProperty("filter.cwu.var2.high",value);
    
    sprintf(value,"%d",filterAM[filterVar1].low);
    setProperty("filter.am.var1.low",value);
    sprintf(value,"%d",filterAM[filterVar1].high);
    setProperty("filter.am.var1.high",value);
    sprintf(value,"%d",filterAM[filterVar2].low);
    setProperty("filter.am.var2.low",value);
    sprintf(value,"%d",filterAM[filterVar2].high);
    setProperty("filter.am.var2.high",value);
    
    sprintf(value,"%d",filterSAM[filterVar1].low);
    setProperty("filter.sam.var1.low",value);
    sprintf(value,"%d",filterSAM[filterVar1].high);
    setProperty("filter.sam.var1.high",value);
    sprintf(value,"%d",filterSAM[filterVar2].low);
    setProperty("filter.sam.var2.low",value);
    sprintf(value,"%d",filterSAM[filterVar2].high);
    setProperty("filter.sam.var2.high",value);
    
    sprintf(value,"%d",filterFMN[filterVar1].low);
    setProperty("filter.fmn.var1.low",value);
    sprintf(value,"%d",filterFMN[filterVar1].high);
    setProperty("filter.fmn.var1.high",value);
    sprintf(value,"%d",filterFMN[filterVar2].low);
    setProperty("filter.fmn.var2.low",value);
    sprintf(value,"%d",filterFMN[filterVar2].high);
    setProperty("filter.fmn.var2.high",value);
    
    sprintf(value,"%d",filterDSB[filterVar1].low);
    setProperty("filter.dsb.var1.low",value);
    sprintf(value,"%d",filterDSB[filterVar1].high);
    setProperty("filter.dsb.var1.high",value);
    sprintf(value,"%d",filterDSB[filterVar2].low);
    setProperty("filter.dsb.var2.low",value);
    sprintf(value,"%d",filterDSB[filterVar2].high);
    setProperty("filter.dsb.var2.high",value);
    
}

/* --------------------------------------------------------------------------*/
/** 
* @brief Restore the filter state.
*/
void filterRestoreState() {
    char* value;
    value=getProperty("filter");
    if(value) filter=atoi(value);

    value=getProperty("filter.lsb.var1.low");
    if(value) filterLSB[filterVar1].low=atoi(value);
    value=getProperty("filter.lsb.var1.high");
    if(value) filterLSB[filterVar1].high=atoi(value);
    value=getProperty("filter.lsb.var2.low");
    if(value) filterLSB[filterVar2].low=atoi(value);
    value=getProperty("filter.lsb.var2.high");
    if(value) filterLSB[filterVar2].high=atoi(value);

    value=getProperty("filter.digl.var1.low");
    if(value) filterDIGL[filterVar1].low=atoi(value);
    value=getProperty("filter.digl.var1.high");
    if(value) filterDIGL[filterVar1].high=atoi(value);
    value=getProperty("filter.digl.var2.low");
    if(value) filterDIGL[filterVar2].low=atoi(value);
    value=getProperty("filter.digl.var2.high");
    if(value) filterDIGL[filterVar2].high=atoi(value);

    value=getProperty("filter.cwl.var1.low");
    if(value) filterCWL[filterVar1].low=atoi(value);
    value=getProperty("filter.cwl.var1.high");
    if(value) filterCWL[filterVar1].high=atoi(value);
    value=getProperty("filter.cwl.var2.low");
    if(value) filterCWL[filterVar2].low=atoi(value);
    value=getProperty("filter.cwl.var2.high");
    if(value) filterCWL[filterVar2].high=atoi(value);

    value=getProperty("filter.usb.var1.low");
    if(value) filterUSB[filterVar1].low=atoi(value);
    value=getProperty("filter.usb.var1.high");
    if(value) filterUSB[filterVar1].high=atoi(value);
    value=getProperty("filter.usb.var2.low");
    if(value) filterUSB[filterVar2].low=atoi(value);
    value=getProperty("filter.usb.var2.high");
    if(value) filterUSB[filterVar2].high=atoi(value);

    value=getProperty("filter.digu.var1.low");
    if(value) filterDIGU[filterVar1].low=atoi(value);
    value=getProperty("filter.digu.var1.high");
    if(value) filterDIGU[filterVar1].high=atoi(value);
    value=getProperty("filter.digu.var2.low");
    if(value) filterDIGU[filterVar2].low=atoi(value);
    value=getProperty("filter.digu.var2.high");
    if(value) filterDIGU[filterVar2].high=atoi(value);

    value=getProperty("filter.cwu.var1.low");
    if(value) filterCWU[filterVar1].low=atoi(value);
    value=getProperty("filter.cwu.var1.high");
    if(value) filterCWU[filterVar1].high=atoi(value);
    value=getProperty("filter.cwu.var2.low");
    if(value) filterCWU[filterVar2].low=atoi(value);
    value=getProperty("filter.cwu.var2.high");
    if(value) filterCWU[filterVar2].high=atoi(value);

    value=getProperty("filter.am.var1.low");
    if(value) filterAM[filterVar1].low=atoi(value);
    value=getProperty("filter.am.var1.high");
    if(value) filterAM[filterVar1].high=atoi(value);
    value=getProperty("filter.am.var2.low");
    if(value) filterAM[filterVar2].low=atoi(value);
    value=getProperty("filter.am.var2.high");
    if(value) filterAM[filterVar2].high=atoi(value);

    value=getProperty("filter.sam.var1.low");
    if(value) filterSAM[filterVar1].low=atoi(value);
    value=getProperty("filter.sam.var1.high");
    if(value) filterSAM[filterVar1].high=atoi(value);
    value=getProperty("filter.sam.var2.low");
    if(value) filterSAM[filterVar2].low=atoi(value);
    value=getProperty("filter.sam.var2.high");
    if(value) filterSAM[filterVar2].high=atoi(value);

    value=getProperty("filter.fmn.var1.low");
    if(value) filterFMN[filterVar1].low=atoi(value);
    value=getProperty("filter.fmn.var1.high");
    if(value) filterFMN[filterVar1].high=atoi(value);
    value=getProperty("filter.fmn.var2.low");
    if(value) filterFMN[filterVar2].low=atoi(value);
    value=getProperty("filter.fmn.var2.high");
    if(value) filterFMN[filterVar2].high=atoi(value);

    value=getProperty("filter.dsb.var1.low");
    if(value) filterDSB[filterVar1].low=atoi(value);
    value=getProperty("filter.dsb.var1.high");
    if(value) filterDSB[filterVar1].high=atoi(value);
    value=getProperty("filter.dsb.var2.low");
    if(value) filterDSB[filterVar2].low=atoi(value);
    value=getProperty("filter.dsb.var2.high");
    if(value) filterDSB[filterVar2].high=atoi(value);

}

/* --------------------------------------------------------------------------*/
/** 
* @brief Set filter values for mode.
*/
void setFilterValues(int mode) {
    GtkWidget* label;

    if(buttonF0) {
    switch(mode) {
        case modeLSB:
            gtk_button_set_label((GtkButton*)buttonF0,filterLSB[filterF0].title);
            gtk_button_set_label((GtkButton*)buttonF1,filterLSB[filterF1].title);
            gtk_button_set_label((GtkButton*)buttonF2,filterLSB[filterF2].title);
            gtk_button_set_label((GtkButton*)buttonF3,filterLSB[filterF3].title);
            gtk_button_set_label((GtkButton*)buttonF4,filterLSB[filterF4].title);
            gtk_button_set_label((GtkButton*)buttonF5,filterLSB[filterF5].title);
            gtk_button_set_label((GtkButton*)buttonF6,filterLSB[filterF6].title);
            gtk_button_set_label((GtkButton*)buttonF7,filterLSB[filterF7].title);
            gtk_button_set_label((GtkButton*)buttonF8,filterLSB[filterF8].title);
            gtk_button_set_label((GtkButton*)buttonF9,filterLSB[filterF9].title);
            gtk_button_set_label((GtkButton*)buttonVar1,filterLSB[filterVar1].title);
            gtk_button_set_label((GtkButton*)buttonVar2,filterLSB[filterVar2].title);
            break;
        case modeUSB:
            gtk_button_set_label((GtkButton*)buttonF0,filterUSB[filterF0].title);
            gtk_button_set_label((GtkButton*)buttonF1,filterUSB[filterF1].title);
            gtk_button_set_label((GtkButton*)buttonF2,filterUSB[filterF2].title);
            gtk_button_set_label((GtkButton*)buttonF3,filterUSB[filterF3].title);
            gtk_button_set_label((GtkButton*)buttonF4,filterUSB[filterF4].title);
            gtk_button_set_label((GtkButton*)buttonF5,filterUSB[filterF5].title);
            gtk_button_set_label((GtkButton*)buttonF6,filterUSB[filterF6].title);
            gtk_button_set_label((GtkButton*)buttonF7,filterUSB[filterF7].title);
            gtk_button_set_label((GtkButton*)buttonF8,filterUSB[filterF8].title);
            gtk_button_set_label((GtkButton*)buttonF9,filterUSB[filterF9].title);
            gtk_button_set_label((GtkButton*)buttonVar1,filterUSB[filterVar1].title);
            gtk_button_set_label((GtkButton*)buttonVar2,filterUSB[filterVar2].title);
            break;
        case modeDSB:
            gtk_button_set_label((GtkButton*)buttonF0,filterDSB[filterF0].title);
            gtk_button_set_label((GtkButton*)buttonF1,filterDSB[filterF1].title);
            gtk_button_set_label((GtkButton*)buttonF2,filterDSB[filterF2].title);
            gtk_button_set_label((GtkButton*)buttonF3,filterDSB[filterF3].title);
            gtk_button_set_label((GtkButton*)buttonF4,filterDSB[filterF4].title);
            gtk_button_set_label((GtkButton*)buttonF5,filterDSB[filterF5].title);
            gtk_button_set_label((GtkButton*)buttonF6,filterDSB[filterF6].title);
            gtk_button_set_label((GtkButton*)buttonF7,filterDSB[filterF7].title);
            gtk_button_set_label((GtkButton*)buttonF8,filterDSB[filterF8].title);
            gtk_button_set_label((GtkButton*)buttonF9,filterDSB[filterF9].title);
            gtk_button_set_label((GtkButton*)buttonVar1,filterDSB[filterVar1].title);
            gtk_button_set_label((GtkButton*)buttonVar2,filterDSB[filterVar2].title);
            break;
        case modeCWL:
            gtk_button_set_label((GtkButton*)buttonF0,filterCWL[filterF0].title);
            gtk_button_set_label((GtkButton*)buttonF1,filterCWL[filterF1].title);
            gtk_button_set_label((GtkButton*)buttonF2,filterCWL[filterF2].title);
            gtk_button_set_label((GtkButton*)buttonF3,filterCWL[filterF3].title);
            gtk_button_set_label((GtkButton*)buttonF4,filterCWL[filterF4].title);
            gtk_button_set_label((GtkButton*)buttonF5,filterCWL[filterF5].title);
            gtk_button_set_label((GtkButton*)buttonF6,filterCWL[filterF6].title);
            gtk_button_set_label((GtkButton*)buttonF7,filterCWL[filterF7].title);
            gtk_button_set_label((GtkButton*)buttonF8,filterCWL[filterF8].title);
            gtk_button_set_label((GtkButton*)buttonF9,filterCWL[filterF9].title);
            gtk_button_set_label((GtkButton*)buttonVar1,filterCWL[filterVar1].title);
            gtk_button_set_label((GtkButton*)buttonVar2,filterCWL[filterVar2].title);
            break;
        case modeCWU:
            gtk_button_set_label((GtkButton*)buttonF0,filterCWU[filterF0].title);
            gtk_button_set_label((GtkButton*)buttonF1,filterCWU[filterF1].title);
            gtk_button_set_label((GtkButton*)buttonF2,filterCWU[filterF2].title);
            gtk_button_set_label((GtkButton*)buttonF3,filterCWU[filterF3].title);
            gtk_button_set_label((GtkButton*)buttonF4,filterCWU[filterF4].title);
            gtk_button_set_label((GtkButton*)buttonF5,filterCWU[filterF5].title);
            gtk_button_set_label((GtkButton*)buttonF6,filterCWU[filterF6].title);
            gtk_button_set_label((GtkButton*)buttonF7,filterCWU[filterF7].title);
            gtk_button_set_label((GtkButton*)buttonF8,filterCWU[filterF8].title);
            gtk_button_set_label((GtkButton*)buttonF9,filterCWU[filterF9].title);
            gtk_button_set_label((GtkButton*)buttonVar1,filterCWU[filterVar1].title);
            gtk_button_set_label((GtkButton*)buttonVar2,filterCWU[filterVar2].title);
            break;
        case modeFMN:
            gtk_button_set_label((GtkButton*)buttonF0,filterFMN[filterF0].title);
            gtk_button_set_label((GtkButton*)buttonF1,filterFMN[filterF1].title);
            gtk_button_set_label((GtkButton*)buttonF2,filterFMN[filterF2].title);
            gtk_button_set_label((GtkButton*)buttonF3,filterFMN[filterF3].title);
            gtk_button_set_label((GtkButton*)buttonF4,filterFMN[filterF4].title);
            gtk_button_set_label((GtkButton*)buttonF5,filterFMN[filterF5].title);
            gtk_button_set_label((GtkButton*)buttonF6,filterFMN[filterF6].title);
            gtk_button_set_label((GtkButton*)buttonF7,filterFMN[filterF7].title);
            gtk_button_set_label((GtkButton*)buttonF8,filterFMN[filterF8].title);
            gtk_button_set_label((GtkButton*)buttonF9,filterFMN[filterF9].title);
            gtk_button_set_label((GtkButton*)buttonVar1,filterFMN[filterVar1].title);
            gtk_button_set_label((GtkButton*)buttonVar2,filterFMN[filterVar2].title);
            break;
        case modeAM:
            gtk_button_set_label((GtkButton*)buttonF0,filterAM[filterF0].title);
            gtk_button_set_label((GtkButton*)buttonF1,filterAM[filterF1].title);
            gtk_button_set_label((GtkButton*)buttonF2,filterAM[filterF2].title);
            gtk_button_set_label((GtkButton*)buttonF3,filterAM[filterF3].title);
            gtk_button_set_label((GtkButton*)buttonF4,filterAM[filterF4].title);
            gtk_button_set_label((GtkButton*)buttonF5,filterAM[filterF5].title);
            gtk_button_set_label((GtkButton*)buttonF6,filterAM[filterF6].title);
            gtk_button_set_label((GtkButton*)buttonF7,filterAM[filterF7].title);
            gtk_button_set_label((GtkButton*)buttonF8,filterAM[filterF8].title);
            gtk_button_set_label((GtkButton*)buttonF9,filterAM[filterF9].title);
            gtk_button_set_label((GtkButton*)buttonVar1,filterAM[filterVar1].title);
            gtk_button_set_label((GtkButton*)buttonVar2,filterAM[filterVar2].title);
            break;
        case modeDIGU:
            gtk_button_set_label((GtkButton*)buttonF0,filterDIGU[filterF0].title);
            gtk_button_set_label((GtkButton*)buttonF1,filterDIGU[filterF1].title);
            gtk_button_set_label((GtkButton*)buttonF2,filterDIGU[filterF2].title);
            gtk_button_set_label((GtkButton*)buttonF3,filterDIGU[filterF3].title);
            gtk_button_set_label((GtkButton*)buttonF4,filterDIGU[filterF4].title);
            gtk_button_set_label((GtkButton*)buttonF5,filterDIGU[filterF5].title);
            gtk_button_set_label((GtkButton*)buttonF6,filterDIGU[filterF6].title);
            gtk_button_set_label((GtkButton*)buttonF7,filterDIGU[filterF7].title);
            gtk_button_set_label((GtkButton*)buttonF8,filterDIGU[filterF8].title);
            gtk_button_set_label((GtkButton*)buttonF9,filterDIGU[filterF9].title);
            gtk_button_set_label((GtkButton*)buttonVar1,filterDIGU[filterVar1].title);
            gtk_button_set_label((GtkButton*)buttonVar2,filterDIGU[filterVar2].title);
            break;
        case modeSPEC:
            gtk_button_set_label((GtkButton*)buttonF0,"");
            gtk_button_set_label((GtkButton*)buttonF1,"");
            gtk_button_set_label((GtkButton*)buttonF2,"");
            gtk_button_set_label((GtkButton*)buttonF3,"");
            gtk_button_set_label((GtkButton*)buttonF4,"");
            gtk_button_set_label((GtkButton*)buttonF5,"");
            gtk_button_set_label((GtkButton*)buttonF6,"");
            gtk_button_set_label((GtkButton*)buttonF7,"");
            gtk_button_set_label((GtkButton*)buttonF8,"");
            gtk_button_set_label((GtkButton*)buttonF9,"");
            gtk_button_set_label((GtkButton*)buttonVar1,"");
            gtk_button_set_label((GtkButton*)buttonVar2,"");
            break;
        case modeDIGL:
            gtk_button_set_label((GtkButton*)buttonF0,filterDIGL[filterF0].title);
            gtk_button_set_label((GtkButton*)buttonF1,filterDIGL[filterF1].title);
            gtk_button_set_label((GtkButton*)buttonF2,filterDIGL[filterF2].title);
            gtk_button_set_label((GtkButton*)buttonF3,filterDIGL[filterF3].title);
            gtk_button_set_label((GtkButton*)buttonF4,filterDIGL[filterF4].title);
            gtk_button_set_label((GtkButton*)buttonF5,filterDIGL[filterF5].title);
            gtk_button_set_label((GtkButton*)buttonF6,filterDIGL[filterF6].title);
            gtk_button_set_label((GtkButton*)buttonF7,filterDIGL[filterF7].title);
            gtk_button_set_label((GtkButton*)buttonF8,filterDIGL[filterF8].title);
            gtk_button_set_label((GtkButton*)buttonF9,filterDIGL[filterF9].title);
            gtk_button_set_label((GtkButton*)buttonVar1,filterDIGL[filterVar1].title);
            gtk_button_set_label((GtkButton*)buttonVar2,filterDIGL[filterVar2].title);
            break;
        case modeSAM:
            gtk_button_set_label((GtkButton*)buttonF0,filterSAM[filterF0].title);
            gtk_button_set_label((GtkButton*)buttonF1,filterSAM[filterF1].title);
            gtk_button_set_label((GtkButton*)buttonF2,filterSAM[filterF2].title);
            gtk_button_set_label((GtkButton*)buttonF3,filterSAM[filterF3].title);
            gtk_button_set_label((GtkButton*)buttonF4,filterSAM[filterF4].title);
            gtk_button_set_label((GtkButton*)buttonF5,filterSAM[filterF5].title);
            gtk_button_set_label((GtkButton*)buttonF6,filterSAM[filterF6].title);
            gtk_button_set_label((GtkButton*)buttonF7,filterSAM[filterF7].title);
            gtk_button_set_label((GtkButton*)buttonF8,filterSAM[filterF8].title);
            gtk_button_set_label((GtkButton*)buttonF9,filterSAM[filterF9].title);
            gtk_button_set_label((GtkButton*)buttonVar1,filterSAM[filterVar1].title);
            gtk_button_set_label((GtkButton*)buttonVar2,filterSAM[filterVar2].title);
            break;
        case modeDRM:
            gtk_button_set_label((GtkButton*)buttonF0,"");
            gtk_button_set_label((GtkButton*)buttonF1,"");
            gtk_button_set_label((GtkButton*)buttonF2,"");
            gtk_button_set_label((GtkButton*)buttonF3,"");
            gtk_button_set_label((GtkButton*)buttonF4,"");
            gtk_button_set_label((GtkButton*)buttonF5,"");
            gtk_button_set_label((GtkButton*)buttonF6,"");
            gtk_button_set_label((GtkButton*)buttonF7,"");
            gtk_button_set_label((GtkButton*)buttonF8,"");
            gtk_button_set_label((GtkButton*)buttonF9,"");
            gtk_button_set_label((GtkButton*)buttonVar1,"");
            gtk_button_set_label((GtkButton*)buttonVar2,"");
            break;
    }

    switch(mode) {
        case modeDRM:
        case modeSPEC:
            gtk_widget_set_sensitive(buttonF0,FALSE);
            gtk_widget_set_sensitive(buttonF1,FALSE);
            gtk_widget_set_sensitive(buttonF2,FALSE);
            gtk_widget_set_sensitive(buttonF3,FALSE);
            gtk_widget_set_sensitive(buttonF4,FALSE);
            gtk_widget_set_sensitive(buttonF5,FALSE);
            gtk_widget_set_sensitive(buttonF6,FALSE);
            gtk_widget_set_sensitive(buttonF7,FALSE);
            gtk_widget_set_sensitive(buttonF8,FALSE);
            gtk_widget_set_sensitive(buttonF9,FALSE);
            gtk_widget_set_sensitive(buttonVar1,FALSE);
            gtk_widget_set_sensitive(buttonVar2,FALSE);
            break;
        default:
            gtk_widget_set_sensitive(buttonF0,TRUE);
            label=gtk_bin_get_child((GtkBin*)buttonF0);
            gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
            gtk_widget_set_sensitive(buttonF1,TRUE);
            label=gtk_bin_get_child((GtkBin*)buttonF1);
            gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
            gtk_widget_set_sensitive(buttonF2,TRUE);
            label=gtk_bin_get_child((GtkBin*)buttonF2);
            gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
            gtk_widget_set_sensitive(buttonF3,TRUE);
            label=gtk_bin_get_child((GtkBin*)buttonF3);
            gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
            gtk_widget_set_sensitive(buttonF4,TRUE);
            label=gtk_bin_get_child((GtkBin*)buttonF4);
            gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
            gtk_widget_set_sensitive(buttonF5,TRUE);
            label=gtk_bin_get_child((GtkBin*)buttonF5);
            gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
            gtk_widget_set_sensitive(buttonF6,TRUE);
            label=gtk_bin_get_child((GtkBin*)buttonF6);
            gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
            gtk_widget_set_sensitive(buttonF7,TRUE);
            label=gtk_bin_get_child((GtkBin*)buttonF7);
            gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
            gtk_widget_set_sensitive(buttonF8,TRUE);
            label=gtk_bin_get_child((GtkBin*)buttonF8);
            gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
            gtk_widget_set_sensitive(buttonF9,TRUE);
            label=gtk_bin_get_child((GtkBin*)buttonF9);
            gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
            gtk_widget_set_sensitive(buttonVar1,TRUE);
            label=gtk_bin_get_child((GtkBin*)buttonVar1);
            gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
            gtk_widget_set_sensitive(buttonVar2,TRUE);
            label=gtk_bin_get_child((GtkBin*)buttonVar2);
            gtk_widget_modify_fg(label, GTK_STATE_NORMAL, &white);
            break;
    }
    }

}
