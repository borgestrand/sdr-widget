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

#include "Interfaces.h"

#include <pcap.h>

// get a list of the interfaces on the system
Interfaces::Interfaces() {

    nInterfaces=0;
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();

    foreach (QNetworkInterface iface, list) {
        if(!(iface.flags() & QNetworkInterface::IsLoopBack)&& iface.flags() & QNetworkInterface::IsUp){
            //qDebug() << iface.name() <<"="<< iface.hardwareAddress();
            QList<QNetworkAddressEntry> addressEntries=iface.addressEntries();
            foreach(QNetworkAddressEntry addr,addressEntries) {
                if(addr.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    qDebug()<<iface.name()<<iface.hardwareAddress()<<addr.ip().toString();
                    interfaces.append(iface);
                    interfaceNames.insert(nInterfaces, iface.humanReadableName().toAscii());
                    nInterfaces++;
                }
            }
        }
    }

    qDebug() << "Interfaces found " << nInterfaces;

    char errbuf[PCAP_ERRBUF_SIZE];

    int devices=pcap_findalldevs(&devs,errbuf);
    pcap_if_t* dev;
    for(dev=devs;dev!=NULL;dev=dev->next) {
        qDebug()<<dev->description<<":"<<dev->name;
    }
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

char* Interfaces::getPcapName(QString name) {
    qDebug()<<"getPcapName:"<<name;
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();
    QString n="";
    foreach (QNetworkInterface iface, list) {
        if(iface.humanReadableName()==name) {
            n=iface.name();
        }
    }

    qDebug()<<"getPcapName: n="<<n;
    if(n!="") {
        pcap_if_t* dev;
        for(dev=devs;dev!=NULL;dev=dev->next) {
            if(QString(dev->name).contains(n)) {
                qDebug()<<dev->description<<":"<<dev->name;
                return dev->name;
            }
        }
    }
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

QString Interfaces::getInterfaceIPAddress(QString name) {
    int i=0;

    foreach(QString n, interfaceNames) {
        if(name==n) {
            break;
        }
        ++i;
    }

    long a=0;
    QList<QNetworkAddressEntry> addressEntries=interfaces.at(i).addressEntries();
    foreach(QNetworkAddressEntry addr,addressEntries) {
        if((addr.ip().toIPv4Address() != 0) && (addr.ip().toIPv4Address() != 0x7f000001)) {
            a=addr.ip().toIPv4Address();
            break;
        }
    }

    QString ip;
    ip.sprintf("%ld.%ld.%ld.%ld",(a>>24)&0xFF,(a>>16)&0xFF,(a>>8)&0xFF,a&0xFF);

    return ip;
}
