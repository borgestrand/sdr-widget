/* 
 * File:   FMNFilters.cpp
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

#include "FMNFilters.h"
#include "Filter.h"
#include "Filters.h"

FMNFilters::FMNFilters() {
    filters[0].init("16k",-8000,8000);
    filters[1].init("12k",-6000,6000);
    filters[2].init("10k",-5000,5000);
    filters[3].init("8k",-4000,4000);
    filters[4].init("6.6k",-3300,3300);
    filters[5].init("5.2k",-2600,2600);
    filters[6].init("4.0k",-2000,2000);
    filters[7].init("3.1k",-1550,1550);
    filters[8].init("2.9k",-1450,1450);
    filters[9].init("2.4k",-1200,1200);

    selectFilter(4);
}

FMNFilters::~FMNFilters() {
}

