/* 
 * File:   Mode.h
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 15 August 2010, 07:46
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

#ifndef MODE_H
#define	MODE_H

#include <QObject>

#define MODE_LSB 0
#define MODE_USB 1
#define MODE_DSB 2
#define MODE_CWL 3
#define MODE_CWU 4
#define MODE_FMN 5
#define MODE_AM 6
#define MODE_DIGU 7
#define MODE_SPEC 8
#define MODE_DIGL 9
#define MODE_SAM 10
#define MODE_DRM 11


class Mode : public QObject {
    Q_OBJECT
public:
    Mode();
    virtual ~Mode();
    void setMode(int m);
    int getMode();
    QString getStringMode();
    QString getStringMode(int mode);

signals:
    void modeChanged(int oldMode, int newMode);

private:
    int currentMode;

};

#endif	/* MODE_H */

