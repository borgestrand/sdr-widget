/* 
 * File:   Filter.cpp
 * Author: John Melton, G0ORX/N6LYT
 * 
 * Created on 14 August 2010, 10:08
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

#include "Filter.h"

Filter::Filter() {
}

Filter::Filter(QString t,int l,int h) {
    text=t;
    low=l;
    high=h;
}

Filter::~Filter() {
}

void Filter::init(QString t,int l,int h) {
    text=t;
    low=l;
    high=h;
}

void Filter::setText(QString t) {
    text=t;
}

void Filter::setLow(int l) {
    low=l;
}

void Filter::setHigh(int h) {
    high=h;
}

QString Filter::getText() {
    return text;
}

int Filter::getLow() {
    return low;
}

int Filter::getHigh() {
    return high;
}

