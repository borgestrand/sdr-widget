/*
 * File:   Interfaces.cpp
 * Author: John Melton, G0ORX/N6LYT
 *
 * Created on 23 November 2010
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

#include <QDebug>
#include <QString>
#include "pcap.h"
#include "Interfaces.h"

void showAddr(char * name, struct sockaddr *a)
{   qDebug()<<name;
    for (int i = 0; i < 14; ++i)
    {   int b = a->sa_data[i];
        qDebug() << b;
    }
}

// get a list of the interfaces on the system
Interfaces::Interfaces() {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_addr_t* addr;
    int nqInterfaces;       // count qualified interfaces in the list from QNetworkInterface::allInterfaces()

    nqInterfaces=0;
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    foreach (QNetworkInterface iface, list) {
        //qDebug() << iface.name() <<"="<< iface.hardwareAddress();
        QList<QNetworkAddressEntry> addressEntries=iface.addressEntries();
        foreach(QNetworkAddressEntry addr,addressEntries) {
            if((addr.ip().toIPv4Address() != 0) && (addr.ip().toIPv4Address() != 0x7f000001)) {     // we only want those with "real" IP addresses
                qDebug()<<iface.name()<<iface.hardwareAddress()<<addr.ip().toString();
                interfaces.append(iface);
                nqInterfaces++;
            }
        }
    }

    qDebug() << "Interfaces found " << nqInterfaces;

    /* Retrieve the device list on the local machine */
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        qDebug()<<"Error in pcap_findalldevs: "<< errbuf;
        exit(1);
    }

    nInterfaces=0;
    for(dev=alldevs;dev;dev=dev->next) {
        for(addr=dev->addresses;addr;addr=addr->next){
            struct sockaddr_in* s=(struct sockaddr_in*)addr->addr;
            if(s->sin_family==AF_INET) {
                if(s->sin_addr.s_addr==0x00000000 || s->sin_addr.s_addr==0x0100007F) {
                    // ignore no address or local interface
                } else {
                    qDebug()<<dev->name;
                    qDebug()<<"   address="<<s->sin_addr.s_addr;
                    if (dev->description != NULL) qDebug()<<"  description "<<dev->description;
                    for (int i = 0; i < nqInterfaces; ++i) {
                        if (strstr(dev->name, interfaces[i].name().toAscii())) {
                            qDebug()<<"Qt interface #"<<i;
                            interfaceNames.insert(i, dev->name);
                        }
                    }
                    ++nInterfaces;
                }
            }
        }
    }

    qDebug() << "Interfaces found "<<nInterfaces;
}


int Interfaces::getInterfaces() {
    return nInterfaces;
}

QString Interfaces::getInterfaceNameAt(int index) {
    QString name;
    int i=0;
    foreach(QString n, interfaceNames)
    {   if (i == index)
        {   name = n;
            break;
        }
        ++i;
    }
    return name;
}

QString Interfaces::getInterfaceHardwareAddress(int index) {
    QString addr=0L;
    int i=0;
    foreach (QNetworkInterface iface, interfaces) {
        if(i==index) {
            addr=iface.hardwareAddress();
            break;
        }
        i++;
    }
    return addr;
}

long Interfaces::getInterfaceIPAddress(int index) {
    long a=0L;
    int i = 0;
    foreach (QNetworkInterface iface, interfaces) {
        if(i==index) {
            QList<QNetworkAddressEntry> addressEntries=iface.addressEntries();
            foreach(QNetworkAddressEntry addr,addressEntries) {
                if((addr.ip().toIPv4Address() != 0) && (addr.ip().toIPv4Address() != 0x7f000001)) {
                    a=addr.ip().toIPv4Address();
                    break;
                }
            }
            break;
        }
        i++;
    }
    return a;
}

