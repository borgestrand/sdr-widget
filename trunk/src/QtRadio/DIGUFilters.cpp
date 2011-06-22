/* 
 * File:   DIGUFilters.cpp
 * Author: John Melton, G0ORX/N6LYT
 * 
 * Created on 14 August 2010, 10:22
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

#include "DIGUFilters.h"
#include "Filter.h"
#include "Filters.h"

DIGUFilters::DIGUFilters() {
    filters[0].init("5.0k",150,5150);
    filters[1].init("4.4k",150,4550);
    filters[2].init("3.8k",150,3950);
    filters[3].init("3.3k",150,3450);
    filters[4].init("2.9k",150,3050);
    filters[5].init("2.7k",150,2850);
    filters[6].init("2.4k",150,2550);
    filters[7].init("2.1k",150,2250);
    filters[8].init("1.8k",150,1950);
    filters[9].init("1.0k",150,1150);

    selectFilter(3);
}

DIGUFilters::~DIGUFilters() {
}

