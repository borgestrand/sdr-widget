/*!
 * \file server.cpp
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


#include "server.h"
#include "mainwindow.h"

Server::Server()
{
  timer = new QTimer(this);

}



void Server::initSocket()
{
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::LocalHost, 7755);

    timer->start(1000);
}

void Server::sendDatagrams()
{
     QByteArray datagram;

     for( int i = 0; i < 512;i++)
     {
        datagram.append( QChar((qrand()/(RAND_MAX/255))) );
     }

     udpSocket->writeDatagram( datagram, QHostAddress::LocalHost, 15000);
}
