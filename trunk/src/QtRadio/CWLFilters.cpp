/* 
 * File:   CWLFilters.cpp
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

#include "CWLFilters.h"
#include "Filter.h"
#include "Filters.h"

CWLFilters::CWLFilters() {
    filters[0].init("1.0k",500,500);
    filters[1].init("800",400,400);
    filters[2].init("750",375,375);
    filters[3].init("600",300,300);
    filters[4].init("500",250,250);
    filters[5].init("400",200,200);
    filters[6].init("250",125,125);
    filters[7].init("100",50,50);
    filters[8].init("50",25,25);
    filters[9].init("25",13,13);

    selectFilter(5);
}

CWLFilters::~CWLFilters() {
}

