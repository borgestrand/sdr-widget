/*!
 * \file server.h
 * \brief
 *
 * \author Dave Larsen, KV0S

**/
/*
Copyright (C) 2009 - David R. Larsen
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QtNetwork>
#include <QTimer>

class Server : public QMainWindow
{
    Q_OBJECT

public:
    Server();
    QUdpSocket *udpSocket;
    QTimer *timer;
    void initSocket();

private slots:
    void sendDatagrams();

};

#endif // SERVER_H
