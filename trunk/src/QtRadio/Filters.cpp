/* 
 * File:   Filters.cpp
 * Author: John Melton, G0ORX/N6LYT
 * 
 * Created on 14 August 2010, 10:15
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


#include "Filters.h"
#include "Mode.h"
#include "FiltersBase.h"

Filters::Filters() {
    currentFilters=NULL;
}

Filters::Filters(const Filters& orig) {
}

Filters::~Filters() {
}

void Filters::selectFilters(FiltersBase* filters) {
    FiltersBase* oldFilters=currentFilters;
    currentFilters=filters;
    emit filtersChanged(oldFilters,currentFilters);
}

void Filters::selectFilter(int f) {
    int previousFilter=currentFilters->getSelected();
    currentFilters->selectFilter(f);
    emit filterChanged(previousFilter,f);
}

int Filters:: getFilter() {
    return currentFilters->getSelected();
}

int Filters::getLow() {
    return currentFilters->getLow();
}

int Filters::getHigh() {
    return currentFilters->getHigh();
}

QString Filters::getText() {
    return currentFilters->getText();
}
