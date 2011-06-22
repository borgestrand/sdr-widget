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

#ifndef FREQUENCYINFO_H
#define FREQUENCYINFO_H

#include <QObject>
#include <QString>

class FrequencyInfo {
public:
    FrequencyInfo();
    FrequencyInfo(long long min,long long max,QString descr,int b,bool t);
    QString getDescription();
    bool isFrequency(long long frequency);
    int getBand();
    bool canTransmit();
private:
    long long minFrequency;
    long long maxFrequency;
    QString description;
    int band;
    bool transmit;
};

#endif // FREQUENCYINFO_H
