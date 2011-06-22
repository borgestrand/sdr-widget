/*
 * File:   RawReceiveThread.cpp
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

#include "RawReceiveThread.h"

RawReceiveThread::RawReceiveThread(unsigned char* hwAddress,pcap_t* hdl) {
    qDebug()<<"RawReceiveThread";
    hw=hwAddress;
    handle=hdl;
    stopped=false;
}

void RawReceiveThread::stop() {
    qDebug()<<"RawReceiveThread::stop";
    stopped=true;
}

void RawReceiveThread::run() {
    struct pcap_pkthdr header;
    unsigned char *packet;

    qDebug() << "RawReceiveThread::run";

    while(!stopped) {
        if(handle==NULL) {
            qDebug()<<"pcap_next handle is NULL";
            break;
        }
        packet=(unsigned char*)pcap_next(handle,&header);
        if(packet==NULL) {
            if(stopped) {
                break;
            }
            //qDebug() <<"RawReceiveThread received NULL packet";
            if(handle==NULL) {
                break;
            }
            emit timeout();
        } else {
            qDebug() <<"RawReceiveThread received packet length "<<header.len;
            if(header.len>=22) {
                // check destination is us
                if(packet[0]==hw[0] && packet[1]==hw[1] && packet[2]==hw[2] &&
                   packet[3]==hw[3] && packet[4]==hw[4] && packet[5]==hw[5]) {
                    // check source is Metis
                    if(packet[6]==0x11 && packet[7]==0x22 && packet[8]==0x33 &&
                       packet[9]==0x44 && packet[10]==0x55 && packet[11]==0x66) {

                        //qDebug() << "received raw packet length"<<header.len;

                        // check valid protocol
                        if(packet[12]==0xEF && packet[13]==0xFE) {
                            if(packet[14]==0x03) {
                                switch(packet[15]) {
                                case INVALID_COMMAND:
                                    // invalid command
                                    qDebug()<<"received INVALID_COMMAND reply";
                                    break;
                                case ERASE_DONE:
                                    emit eraseCompleted();
                                    break;
                                case SEND_MORE:
                                    emit nextBuffer();
                                    break;
                                case HAVE_MAC_ADDRESS:
                                    emit macAddress(&packet[16]);
                                    break;
                                case HAVE_IP_ADDRESS:
                                    emit ipAddress(&packet[16]);
                                    break;
                                case FPGA_ID:
                                    emit fpgaId(&packet[16]);
                                    break;
                                default:
                                    qDebug()<<"received invalid reply"<<packet[15];
                                }
                            }
                        } else {
                            qDebug()<<"expected 0x03 found"<<packet[14];
                        }
                    } else {
                        qDebug()<<"expected 0xEF 0xFE found"<<packet[13]<<packet[13];
                    }
                }
            } else {
            }
        }
    }

    if(handle!=NULL) {
        pcap_close(handle);
    }
}
