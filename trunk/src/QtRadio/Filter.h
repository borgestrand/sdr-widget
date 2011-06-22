/* 
 * File:   Filter.h
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

#ifndef FILTER_H
#define	FILTER_H

#include <QObject>
#include <QSettings>
#include <QDebug>

class Filter : QObject {
    Q_OBJECT
public:
    Filter();
    Filter(QString t,int l,int h);
    virtual ~Filter();
    void init(QString t,int l,int h);
    void setText(QString t);
    void setLow(int l);
    void setHigh(int h);
    QString getText();
    int getLow();
    int getHigh();
private:
    QString text;
    int low;
    int high;
};

#endif	/* FILTER_H */

