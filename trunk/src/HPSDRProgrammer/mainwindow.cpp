/*
 * File:   mainwindow.cpp
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


#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QDebug"
#include "QCursor"
#include "QFile"
#include "QFileDialog"
#include "QRect"
#include "QTimer"

#include "pcap.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    int i;
    int nInterfaces;

    qDebug()<<"HPSDRProgrammer"<<VERSION;

    ui->setupUi(this);

    receiveThread=NULL;
    rawReceiveThread=NULL;
    discovery=NULL;

#ifdef __WIN32
    ui->privilegesLabel->setText("You must be running with Administrator privileges to be able to read/write raw ethernet frames.");
    QRect rect=ui->interfaceComboBox->geometry();
    rect.setWidth(ui->interfaceLabel->width());
    ui->interfaceComboBox->setGeometry(rect);
#else
    ui->privilegesLabel->setText("You must be running as root to be able to read/write raw ethernet frames.");
#endif

    for (i = 0; i < interfaces.getInterfaces(); ++i)
    {   ui->interfaceComboBox->addItem(interfaces.getInterfaceNameAt(i));
        ++nInterfaces;
    }

    connect(ui->actionQuit,SIGNAL(triggered()),this,SLOT(quit()));
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(about()));
    connect(ui->interfaceComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(interfaceSelected(int)));
    connect(ui->browsePushButton,SIGNAL(clicked()),this,SLOT(browse()));
    connect(ui->programPushButton,SIGNAL(clicked()),this,SLOT(program()));
    connect(ui->erasePushButton,SIGNAL(clicked()),this,SLOT(erase()));
    connect(ui->readMACPushButton,SIGNAL(clicked()),this,SLOT(getMAC()));
    connect(ui->readIPPushButton,SIGNAL(clicked()),this,SLOT(getIP()));
    connect(ui->writeIPPushButton,SIGNAL(clicked()),this,SLOT(setIP()));

    connect(ui->discoverPushButton,SIGNAL(clicked()),this,SLOT(discover()));
    connect(ui->metisComboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(metisSelected(int)));

    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(tabChanged(int)));

    connect(ui->interrogatePushButton,SIGNAL(clicked()),this,SLOT(jtagInterrogate()));
    connect(ui->jtagBrowsePushButton,SIGNAL(clicked()),this,SLOT(jtagBrowse()));
    connect(ui->jtagFlashBrowsePushButton,SIGNAL(clicked()),this,SLOT(jtagFlashBrowse()));
    connect(ui->jtagFlashProgramPushButton,SIGNAL(clicked()),this,SLOT(jtagFlashProgram()));

    if(ui->interfaceComboBox->count()>0) {
        ui->interfaceComboBox->setCurrentIndex(0);
        interfaceSelected(0);
    } else {
        // dont allow discovery if no interface found
        ui->discoverPushButton->setEnabled(false);
    }

    bootloader=false;

    ui->tabWidget->setTabEnabled(1,true);
    ui->tabWidget->setTabEnabled(2,true);

    ui->tabWidget_2->setTabEnabled(1,true);
    ui->tabWidget_2->setTabEnabled(2,false);
    ui->tabWidget_2->setTabEnabled(3,false);
    ui->tabWidget_2->setTabEnabled(4,false);

    aboutDialog.setVersion(VERSION);
}

MainWindow::~MainWindow() {
    delete ui;
}

#ifdef Q_WS_MAC
void MainWindow::setPath(char* path) {
    myPath=path;
}
#endif

void MainWindow::quit() {
    exit(0);
}

void MainWindow::about() {
    aboutDialog.setVisible(TRUE);
}

// private function to display message in the status window
void MainWindow::status(QString text) {
    qDebug()<<"status:"<<text;
    ui->statusListWidget->insertItem(ui->statusListWidget->count()-1,text);
    ui->statusListWidget->setCurrentRow(ui->statusListWidget->count()-1);
}

// SLOT - interfaceSelected - called when the interface selection is changed
void MainWindow::interfaceSelected(int index) {
    bool ok;
    interfaceName=interfaces.getInterfaceNameAt(index);
    ip=interfaces.getInterfaceIPAddress(index);
    hwAddress=interfaces.getInterfaceHardwareAddress(index);
    if(hwAddress==NULL) {
        ui->interfaceLabel->setText("");
        status("Inteface is not a valid network device");
    } else {
        QString text;
        text.sprintf("MAC=%s  IP=%ld.%ld.%ld.%ld",
                     hwAddress.toAscii().constData(),
                     (ip>>24)&0xFF,(ip>>16)&0xFF,(ip>>8)&0xFF,ip&0xFF);
        ui->interfaceLabel->setText(text);

        hw[0]=(unsigned char)hwAddress.mid(0,2).toInt(&ok,16);
        hw[1]=(unsigned char)hwAddress.mid(3,2).toInt(&ok,16);
        hw[2]=(unsigned char)hwAddress.mid(6,2).toInt(&ok,16);
        hw[3]=(unsigned char)hwAddress.mid(9,2).toInt(&ok,16);
        hw[4]=(unsigned char)hwAddress.mid(12,2).toInt(&ok,16);
        hw[5]=(unsigned char)hwAddress.mid(15,2).toInt(&ok,16);
    }
}

// SLOT - browse - called when the "Browse ..." button on the Program tab is pressed.
void MainWindow::browse() {
    //QString fileName=QFileDialog::getOpenFileName(this,tr("Select File"),"",tr("pof Files (*.pof)"));
    QString fileName=QFileDialog::getOpenFileName(this,tr("Select File"),"",tr("rbf Files (*.rbf)"));
    ui->fileLineEdit->setText(fileName);
}


// private load an rbf file
int MainWindow::loadRBF(QString filename) {
    int length;
    int i;
    int rc;

    QFile rbfFile(filename);
    rbfFile.open(QIODevice::ReadOnly);
    QDataStream in(&rbfFile);
    length=((rbfFile.size()+255)/256)*256;
    data=(char*)malloc(length);


    qDebug() << "file size=" << rbfFile.size() << "length=" << length;
    status(filename);
    status("reading file...");
    if(in.readRawData(data,rbfFile.size())!=rbfFile.size()) {
        status("Error: could not read rbf file");
        rbfFile.close();
        QApplication::restoreOverrideCursor();
        blocks=0;
        rc=1;
    } else {
        status("file read successfully");

        // pad out to mod 256 with 0xFF
        for(i=rbfFile.size();i<length;i++) {
            data[i]=0xFF;
        }
        rbfFile.close();

        start=0;
        end=length;
        blocks=length/256;

        qDebug() <<"start="<<start<<" end="<<end<<" blocks="<<blocks;
        rc=0;
    }
    return rc;
}

// SLOT - program - called when the "Program" button on the Program tab is pressed.
void MainWindow::program() {

    ui->statusListWidget->clear();
    ui->statusListWidget->addItem("");
    percent=0;

    // check that an interface has been selected
    if(ui->interfaceComboBox->currentIndex()!=-1) {
        // check that a file has been selected
        if(ui->fileLineEdit->text().endsWith(".rbf")) {
        //if(ui->fileLineEdit->text().endsWith(".pof")) {


            QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

            // load thefile
            //loadPOF(ui->fileLineEdit->text());
            if(loadRBF(ui->fileLineEdit->text())==0) {
                if(bootloader) {
                    bootloaderProgram();
                } else {
                    flashProgram();
                }
            }
        } else {
            status("Error: no file selected");
        }
    } else {
        status("Error: no interface selected");
    }
}

void MainWindow::bootloaderProgram() {
    char errbuf[PCAP_ERRBUF_SIZE];

    size=0;
    data_command=PROGRAM_FLASH;
    qDebug()<<"MainWindow::bootloaderProgram";
    handle=pcap_open_live(interfaces.getPcapName(ui->interfaceComboBox->currentText().toAscii().constData()),1024,1,TIMEOUT,errbuf);
    if (handle == NULL) {
        qDebug()<<"Couldn't open device "<<ui->interfaceComboBox->currentText().toAscii().constData()<<errbuf;
        status("Error: cannot open interface (are you running as root)");
    } else {
        rawReceiveThread=new RawReceiveThread(hw,handle);
        rawReceiveThread->start();
        QObject::connect(rawReceiveThread,SIGNAL(eraseCompleted()),this,SLOT(eraseCompleted()));
        QObject::connect(rawReceiveThread,SIGNAL(nextBuffer()),this,SLOT(nextBuffer()));
        QObject::connect(rawReceiveThread,SIGNAL(timeout()),this,SLOT(timeout()));

        // start by erasing
        state=ERASING;
        eraseData();
    }
}

void MainWindow::flashProgram() {
    int on=1;
    int rc;

    size=blocks;
    data_command=PROGRAM_METIS_FLASH;

    // start a thread to listen for replies
    QString myip=interfaces.getInterfaceIPAddress(interfaceName);
    receiveThread=new ReceiveThread(myip,metisHostAddress);

    QObject::connect(receiveThread,SIGNAL(eraseCompleted()),this,SLOT(eraseCompleted()));
    QObject::connect(receiveThread,SIGNAL(nextBuffer()),this,SLOT(nextBuffer()));
    QObject::connect(receiveThread,SIGNAL(timeout()),this,SLOT(timeout()));

    state=ERASING;
    eraseData();

}

// SLOT - erase - called when the "Erase" button on the Erase taqb is pressed
void MainWindow::erase() {

    ui->statusListWidget->clear();
    status("");

    qDebug()<<"erase";

    ui->statusListWidget->clear();
    ui->statusListWidget->addItem("");
    percent=0;

    // check that an interface has been selected
    if(ui->interfaceComboBox->currentIndex()!=-1) {
        QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
        if(bootloader) {
            bootloaderErase();
        } else {
            flashErase();
        }
    } else {
        status("Error: no interface selected");
    }
}

void MainWindow::bootloaderErase() {
    char errbuf[PCAP_ERRBUF_SIZE];

    handle=pcap_open_live(interfaces.getPcapName(ui->interfaceComboBox->currentText().toAscii().constData()),1024,1,TIMEOUT,errbuf);
    if (handle == NULL) {
        qDebug()<<"Couldn't open device "<<ui->interfaceComboBox->currentText().toAscii().constData()<<errbuf;
        status("Error: cannot open interface (are you running as root)");
    } else {
        rawReceiveThread=new RawReceiveThread(hw,handle);
        QObject::connect(rawReceiveThread,SIGNAL(eraseCompleted()),this,SLOT(eraseCompleted()));
        QObject::connect(rawReceiveThread,SIGNAL(timeout()),this,SLOT(timeout()));
        rawReceiveThread->start();

        // start by erasing
        state=ERASING_ONLY;
        eraseData();
    }
}

void MainWindow::flashErase() {
    int on=1;
    int rc;

    qDebug()<<"MainWindow::flashErase";

    QString myip=interfaces.getInterfaceIPAddress(interfaceName);
    receiveThread=new ReceiveThread(myip,metisHostAddress);

    // start erasing
    state=ERASING_ONLY;
    eraseData();
}

// SLOT - getMac - called when the "Read" button on the MAC Address tab is pressed.
void MainWindow::getMAC() {
    char errbuf[PCAP_ERRBUF_SIZE];


    qDebug()<<"getMAC";

    ui->statusListWidget->clear();
    status("");

    ui->macLineEdit->setText("");
    percent=0;

    // check that an interface has been selected
    if(ui->interfaceComboBox->currentIndex()!=-1) {
        //hw=interfaces.getInterfaceHardwareAddress(ui->interfaceComboBox->currentText());
        //ip=interfaces.getInterfaceIPAddress(ui->interfaceComboBox->currentText());

        handle=pcap_open_live(interfaces.getPcapName(ui->interfaceComboBox->currentText().toAscii().constData()),1024,1,TIMEOUT,errbuf);
        if (handle == NULL) {
            qDebug()<<"Couldn't open device "<<ui->interfaceComboBox->currentText().toAscii().constData()<<errbuf;
            status("Error: cannot open interface (are you running as root)");
        } else {

            rawReceiveThread=new RawReceiveThread(hw,handle);
            rawReceiveThread->start();
            QObject::connect(rawReceiveThread,SIGNAL(macAddress(unsigned char*)),this,SLOT(macAddress(unsigned char*)));
            QObject::connect(rawReceiveThread,SIGNAL(timeout()),this,SLOT(timeout()));


            state=READ_MAC;
            readMAC();
        }
    } else {
        status("Error: no interface selected");
    }

}

// SLOT - macAddress - called when the reply packet is received containg the Metis MAC address.
void MainWindow::macAddress(unsigned char* mac) {
    QString text;
    text.sprintf("%02X:%02X:%02X:%02X:%02X:%02X",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    ui->macLineEdit->setText(text);
    status("Metis MAC address read successfully");
    idle();
}

// SLOT - getIP - called when the "Read" button on the IP Address tab is pressed.
void MainWindow::getIP() {
    char errbuf[PCAP_ERRBUF_SIZE];

    qDebug()<<"getIP: "<<interfaceName;

    ui->statusListWidget->clear();
    status("");

    ui->ipALineEdit->setText("");
    ui->ipBLineEdit->setText("");
    ui->ipCLineEdit->setText("");
    ui->ipDLineEdit->setText("");
    percent=0;

    // check that an interface has been selected
    if(ui->interfaceComboBox->currentIndex()!=-1) {

        handle=pcap_open_live(interfaces.getPcapName(ui->interfaceComboBox->currentText().toAscii().constData()),1024,1,TIMEOUT,errbuf);
        if (handle == NULL) {
            qDebug()<<"Couldn't open device "<<ui->interfaceComboBox->currentText().toAscii().constData()<<errbuf;
            status("Error: cannot open interface (are you running as root)");
        } else {

            rawReceiveThread=new RawReceiveThread(hw,handle);
            rawReceiveThread->start();
            QObject::connect(rawReceiveThread,SIGNAL(ipAddress(unsigned char*)),this,SLOT(ipAddress(unsigned char*)));
            QObject::connect(rawReceiveThread,SIGNAL(timeout()),this,SLOT(timeout()));

            state=READ_IP;
            readIP();
        }
    } else {
        status("Error: no interface selected");
    }
}

// SLOT - ipAddress - called when the reply packet is received containing Metis IP address.
void MainWindow::ipAddress(unsigned char* ip) {
    QString text;
    text.sprintf("%d",ip[0]);
    ui->ipALineEdit->setText(text);
    text.sprintf("%d",ip[1]);
    ui->ipBLineEdit->setText(text);
    text.sprintf("%d",ip[2]);
    ui->ipCLineEdit->setText(text);
    text.sprintf("%d",ip[3]);
    ui->ipDLineEdit->setText(text);
    status("Metis IP address read successfully");
    idle();
}

// SLOT - setIP - called when the "Write" button on the IP Address tab is pressed.
void MainWindow::setIP() {
    char errbuf[PCAP_ERRBUF_SIZE];
    unsigned char buffer[66];
    int addr[4];
    int i;

    qDebug()<<"setIP";

    ui->statusListWidget->clear();
    status("");

    // will need to run Discovery again
    ui->metisComboBox->clear();
    metis.clear();

    addr[0]=ui->ipALineEdit->text().toInt();
    addr[1]=ui->ipBLineEdit->text().toInt();
    addr[2]=ui->ipCLineEdit->text().toInt();
    addr[3]=ui->ipDLineEdit->text().toInt();
    if((addr[0]<0 || addr[0]>255) || (addr[1]<0 || addr[1]>255) || (addr[2]<0 || addr[2]>255) || (addr[3]<0 || addr[3]>255)) {
        status("Error: invalid IP address");
    } else {

        handle=pcap_open_live(interfaces.getPcapName(ui->interfaceComboBox->currentText().toAscii().constData()),1024,1,TIMEOUT,errbuf);
        if (handle == NULL) {
            qDebug()<<"Couldn't open device "<<ui->interfaceComboBox->currentText().toAscii().constData()<<errbuf;
            status("Error: cannot open interface (are you running as root)");
        } else {

            state=WRITE_IP;

            /*set the frame header*/
            buffer[0]=0x11; // dest address
            buffer[1]=0x22;
            buffer[2]=0x33;
            buffer[3]=0x44;
            buffer[4]=0x55;
            buffer[5]=0x66;

            buffer[6]=hw[0]; // src address
            buffer[7]=hw[1];
            buffer[8]=hw[2];
            buffer[9]=hw[3];
            buffer[10]=hw[4];
            buffer[11]=hw[5];

            buffer[12]=0xEF; // protocol
            buffer[13]=0xFE;

            buffer[14]=0x03; //
            buffer[15]=WRITE_METIS_IP;

            /*fill the frame with 0x00*/
            buffer[16]=(unsigned char)addr[0];
            buffer[17]=(unsigned char)addr[1];
            buffer[18]=(unsigned char)addr[2];
            buffer[19]=(unsigned char)addr[3];

            for(i=0;i<46;i++) {
                buffer[i+20]=(unsigned char)0x00;
            }

           if(pcap_sendpacket(handle,buffer,62)!=0) {
                qDebug()<<"pcap_sendpacket failed";
                status("send write ip command failed");
                idle();
            } else {
                status("Written Metis IP address");
            }
        }

        idle();
    }
}

// private function to send the command to erase
void MainWindow::eraseData() {
    eraseTimeouts=0;
    status("Erasing device ... (takes several seconds)");
    if(bootloader) {
        sendRawCommand(ERASE_METIS_FLASH);
    } else {    
        sendCommand(ERASE_METIS_FLASH);
        // wait 20 seconds to allow replys
        QTimer::singleShot(20000,this,SLOT(erase_timeout()));
    }
}

// slot for erase timout
void MainWindow::erase_timeout() {
    qDebug()<<"MainWindow::erase_timeout";
    if(state==ERASING || state==ERASING_ONLY) {
        status("Error: erase timeout.");
        status("Power cycle and try again.");
        idle();
        QApplication::restoreOverrideCursor();
    }
}

// private function to send command to read MAC address from Metis
void MainWindow::readMAC() {
    eraseTimeouts=0;
    status("Reading Metis MAC Address ...");
    sendRawCommand(READ_METIS_MAC);
}

// private function to read the IP address from Metis.
void MainWindow::readIP() {
    eraseTimeouts=0;
    status("Reading Metis IP address ...");
    sendRawCommand(READ_METIS_IP);
}

// private function to send a command.
void MainWindow::sendCommand(unsigned char command) {
    unsigned char buffer[64];
    int i;

    qDebug()<<"sendCommand "<<command;

    buffer[0]=0xEF; // protocol
    buffer[1]=0xFE;

    buffer[2]=0x03;
    buffer[3]=command;

    /*fill the frame with 0x00*/
    for(i=0;i<60;i++) {
        buffer[i+4]=(unsigned char)0x00;
    }

    receiveThread->send((const char*)buffer,sizeof(buffer));

}

// private function to send a command.
void MainWindow::sendRawCommand(unsigned char command) {
    unsigned char buffer[62];
    int i;

    //qDebug()<<"sendCommand "<<command;

    if(handle!=NULL) {
        /*set the frame header*/
        buffer[0]=0x11; // dest address
        buffer[1]=0x22;
        buffer[2]=0x33;
        buffer[3]=0x44;
        buffer[4]=0x55;
        buffer[5]=0x66;

        buffer[6]=hw[0]; // src address
        buffer[7]=hw[1];
        buffer[8]=hw[2];
        buffer[9]=hw[3];
        buffer[10]=hw[4];
        buffer[11]=hw[5];

        buffer[12]=0xEF; // protocol
        buffer[13]=0xFE;

        buffer[14]=0x03;
        buffer[15]=command;

        /*fill the frame with 0x00*/
        for(i=0;i<46;i++) {
                buffer[i+16]=(unsigned char)0x00;
        }

        if(handle!=NULL) {
            //qDebug() << "pcap_sendpacket";
            if(pcap_sendpacket(handle,buffer,62)!=0) {
                qDebug()<<"pcap_sendpacket failed";
                status("send command failed");
                idle();
            }
        }
    }
}

// private function to send 256 byte block of the pof file.
void MainWindow::sendData() {
    unsigned char buffer[264];

    qDebug()<<"sendData offset="<<offset;

    buffer[0]=0xEF;
    buffer[1]=0xFE;
    buffer[2]=0x03;
    buffer[3]=PROGRAM_METIS_FLASH;
    buffer[4]=(blocks>>24)&0xFF;
    buffer[5]=(blocks>>16)&0xFF;
    buffer[6]=(blocks>>8)&0xFF;
    buffer[7]=blocks&0xFF;

    /*fill the frame with some data*/
    for(int i=0;i<256;i++) {
        buffer[i+8]=(unsigned char)data[i+offset];
    }

    receiveThread->send((const char*)buffer,sizeof(buffer));

    QString text;
    int p=(offset+256)*100/(end-start);
    if(p!=percent) {
        if((p%20)==0) {
            percent=p;
            text.sprintf("Programming device %d%% written ...",percent);
            status(text);
        }
    }

}

// private function to send 256 byte block of the pof file.
void MainWindow::sendRawData() {
    unsigned char buffer[276];

    qDebug()<<"sendRawData offset="<<offset;

    if(handle!=NULL) {
        /*set the frame header*/
        buffer[0]=0x11; // dest address
        buffer[1]=0x22;
        buffer[2]=0x33;
        buffer[3]=0x44;
        buffer[4]=0x55;
        buffer[5]=0x66;

        buffer[6]=hw[0]; // src address
        buffer[7]=hw[1];
        buffer[8]=hw[2];
        buffer[9]=hw[3];
        buffer[10]=hw[4];
        buffer[11]=hw[5];

        buffer[12]=0xEF; // protocol
        buffer[13]=0xFE;

        buffer[14]=0x03;
        buffer[15]=PROGRAM_METIS_FLASH;

        buffer[16]=0x00;
        buffer[17]=0x00;
        buffer[18]=0x00;
        buffer[19]=0x00;

        /*fill the frame with some data*/
        for(int i=0;i<256;i++) {
                buffer[i+20]=(unsigned char)data[i+offset];
        }

        if(pcap_sendpacket(handle,buffer,276)!=0) {
            qDebug()<<"pcap_sendpacket failed";
            status("send data command failed");
            idle();
        } else {
            QString text;
            int p=offset*100/(end-start);
            if(p!=percent) {
                if((p%20)==0) {
                    percent=p;
                    text.sprintf("Programming device %d%% written ...",percent);
                    status(text);
                }
            }
        }
    }
}

// private function to send 256 byte block of the pof file.
void MainWindow::sendJTAGData() {
    unsigned char buffer[276];
    int length=blocks*256;

    qDebug()<<"sendRawData offset="<<offset;

    if(handle!=NULL) {
        /*set the frame header*/
        buffer[0]=0x11; // dest address
        buffer[1]=0x22;
        buffer[2]=0x33;
        buffer[3]=0x44;
        buffer[4]=0x55;
        buffer[5]=0x66;

        buffer[6]=hw[0]; // src address
        buffer[7]=hw[1];
        buffer[8]=hw[2];
        buffer[9]=hw[3];
        buffer[10]=hw[4];
        buffer[11]=hw[5];

        buffer[12]=0xEF; // protocol
        buffer[13]=0xFE;

        buffer[14]=0x03;
        buffer[15]=data_command;


        buffer[16]=(length>>24)&0xFF;
        buffer[17]=(length>>16)&0xFF;
        buffer[18]=(length>>8)&0xFF;
        buffer[19]=length&0xFF;

        /*fill the frame with some data*/
        for(int i=0;i<256;i++) {
                buffer[i+20]=(unsigned char)data[i+offset];
        }

        if(pcap_sendpacket(handle,buffer,276)!=0) {
            qDebug()<<"pcap_sendpacket failed";
            status("send data command failed");
            idle();
        } else {
            QString text;
            int p=offset*100/(end-start);
            if(p!=percent) {
                if((p%20)==0) {
                    percent=p;
                    text.sprintf("Programming device %d%% written ...",percent);
                    status(text);
                }
            }
        }
    }
}

// private function to send 256 byte block of the pof file.
void MainWindow::sendJTAGFlashData() {
    unsigned char buffer[276];

    qDebug()<<"sendJTAGFlashData offset="<<offset;

    if(handle!=NULL) {
        /*set the frame header*/
        buffer[0]=0x11; // dest address
        buffer[1]=0x22;
        buffer[2]=0x33;
        buffer[3]=0x44;
        buffer[4]=0x55;
        buffer[5]=0x66;

        buffer[6]=hw[0]; // src address
        buffer[7]=hw[1];
        buffer[8]=hw[2];
        buffer[9]=hw[3];
        buffer[10]=hw[4];
        buffer[11]=hw[5];

        buffer[12]=0xEF; // protocol
        buffer[13]=0xFE;

        buffer[14]=0x03;
        buffer[15]=PROGRAM_FLASH;

        buffer[16]=(blocks>>24)&0xFF;
        buffer[17]=(blocks>>16)&0xFF;
        buffer[18]=(blocks>>8)&0xFF;
        buffer[19]=blocks&0xFF;

        /*fill the frame with some data*/
        for(int i=0;i<256;i++) {
                buffer[i+20]=(unsigned char)data[i+offset];
        }

        if(pcap_sendpacket(handle,buffer,276)!=0) {
            qDebug()<<"pcap_sendpacket failed";
            status("send data command failed");
            idle();
        } else {
            QString text;
            int p=offset*100/(end-start);
            if(p!=percent) {
                if((p%20)==0) {
                    percent=p;
                    text.sprintf("Programming device %d%% written ...",percent);
                    status(text);
                }
            }
        }
    }
}

// SLOT - eraseCompleted
void MainWindow::eraseCompleted() {
    switch(state) {
    case IDLE:
        qDebug()<<"received eraseCompleted when state is IDLE";
        break;
    case ERASING:
        status("Device erased successfully");
        state=PROGRAMMING;
        offset=start;
        status("Programming device ...");
        if(bootloader) {
            sendRawData();
        } else {
            sendData();
        }
        break;
    case ERASING_ONLY:
        status("Device erased successfully");
        idle();
        QApplication::restoreOverrideCursor();
        break;
    case READ_MAC:
        qDebug()<<"received eraseCompleted when state is READ_MAC";
        break;
    case READ_IP:
        qDebug()<<"received eraseCompleted when state is READ_IP";
        break;
    case WRITE_IP:
        status("Metis IP written successfully");
        idle();
        break;
    case JTAG_INTERROGATE:
        qDebug()<<"received eraseCompleted when state is JTAG_INTERROGATE";
        break;
    case JTAG_PROGRAM:
        qDebug()<<"received eraseCompleted when state is JTAG_PROGRAM";
        break;
    case FLASH_ERASING:
        status("Flash erased successfully");
        // now load the flash
        loadFlash();
        break;
    case FLASH_PROGRAM:
        qDebug()<<"received eraseCompleted when state is FLASH_PROGRAM";
        break;
    }
}

// SLOT - fpgaId
void MainWindow::fpgaId(unsigned char* data) {
    switch(state) {
    case JTAG_INTERROGATE:
        fpga_id=((data[0]&0xFF)<<16)+((data[1]&0xFF)<<8)+(data[2]&0xFF);
        if(fpga_id==0) {
            status("No Mercury or Penelope board found");
            status("Make sure that Metis is in the slot farthest away from the power connector.");
            status("The target board should be in the next slot adjacent to Metis.");
            status("The target board has the 'Last JTAG' jumper installed.");
            status("The are no other boards on the Atlas Bus.");
        } else if(fpga_id==0x020F30) {
            status("found Mercury");
            ui->jtagLineEdit->setText("Mercury - 0x020F30");
#ifdef Q_WS_WIN
            ui->jtagProgramLineEdit->setText("Mercury_JTAG.rbf");
#endif
#ifdef Q_WS_MAC
            QString rbfPath;
            rbfPath.sprintf("%s/../Resources/Mercury_JTAG.rbf",myPath);
            ui->jtagProgramLineEdit->setText(rbfPath);
#endif
#ifdef Q_WS_X11
            ui->jtagProgramLineEdit->setText("/usr/local/hpsdr/Mercury_JTAG.rbf");
#endif
        } else if(fpga_id==0x020B20) {
            status("found Penelope");
            ui->jtagLineEdit->setText("Penelope - 0x020B20");
#ifdef Q_WS_WIN
            ui->jtagProgramLineEdit->setText("Penelpe_JTAG.rbf");
#endif
#ifdef Q_WS_MAC
            QString rbfPath;
            rbfPath.sprintf("%s/../Resources/Penelope_JTAG.rbf",myPath);
            ui->jtagProgramLineEdit->setText(rbfPath);
#endif
#ifdef Q_WS_X11
            ui->jtagProgramLineEdit->setText("/usr/local/hpsdr/Penelope_JTAG.rbf");
#endif

        } else {
            status("unknown FPGA id");
            fpga_id=0;
        }
        idle();
        break;
    default:
        qDebug()<<"received fpgaId when state is not JTAG_INTERROGATE";
        break;
    }
}

// SLOT - called when a ready for next buffer reply packet is received.
void MainWindow::nextBuffer() {
    offset+=256;
    if(offset<end) {
        if(bootloader) {
            sendRawData();
        } else {
            sendData();
        }
    } else {
        status("Programming device completed successfully.");
        if(bootloader) {
            status("Remember to remove JP1 when you power cycle.");
        } else {
            ui->metisComboBox->clear();
            metis.clear();
            status("Please wait for Metis to restart.");
            status("If using DHCP this can take up to 5 seconds.");
            status("To use other functions you will need to run Discovery again.");
        }
        idle();
        QApplication::restoreOverrideCursor();
    }
}

// SLOT - called when a raw packet read times out (especially when erasing)
void MainWindow::timeout() {
    qDebug()<<"MainWindow::timeout state="<<state;
    switch(state) {
    case IDLE:
        // ignore
        break;
    case ERASING:
    case ERASING_ONLY:
        if(bootloader) {
            eraseTimeouts++;
            qDebug()<<"eraseTimeouts="<<eraseTimeouts;
            if(eraseTimeouts==MAX_ERASE_TIMEOUTS) {
                status("Error: erase timeout.");
                status("Have you set the jumper at JP1 on Metis and power cycled?");
                idle();
                QApplication::restoreOverrideCursor();
            }
        } else {
            status("Error: erase timeout.");
            status("Power cycle and try again.");
            idle();
            QApplication::restoreOverrideCursor();
        }
        break;
    case PROGRAMMING:
        //qDebug()<<"timeout";
        break;
    case READ_MAC:
        status("Error: timeout reading MAC address!");
        status("Check that the correct interface is selected.");
        status("Check that there is a jumper at JP1 on Metis.");
        idle();
        break;
    case READ_IP:
        status("Error: timeout reading IP address!");
        status("Check that the correct interface is selected.");
        status("Check that there is a jumper at JP1 on Metis.");
        idle();
        break;
    case WRITE_IP:
        // should not happen as there is no repsonse
        break;
    case JTAG_INTERROGATE:
        status("Error: timeout reading interrogating JTAG chain!");
        status("Check that the correct interface is selected.");
        status("Check that there is a jumper at JP1 on Metis.");
        idle();
        break;
    case JTAG_PROGRAM:
        //qDebug() << "timeout while state is JTAG_PROGRAM";
        break;
    case FLASH_ERASING:
        eraseTimeouts++;
        if(eraseTimeouts==MAX_ERASE_TIMEOUTS) {
            status("Error: erase timeout - power cycle and try again?");
            idle();
            QApplication::restoreOverrideCursor();
        }
        break;
    case FLASH_PROGRAM:
        //qDebug() << "timeout while state is FLASH_PROGRAM";
        break;
    }
}

// private function to set state to idle
void MainWindow::idle() {
    qDebug()<<"idle";
    state=IDLE;
    if(rawReceiveThread!=NULL) {
        rawReceiveThread->stop();
        rawReceiveThread=NULL;
    }
    if(receiveThread!=NULL) {
        receiveThread->stop();
        receiveThread=NULL;
    }

}


void MainWindow::discover() {
    ui->statusListWidget->clear();
    status("");
    status("Metis Discovery");

    ui->metisComboBox->clear();
    metis.clear();



    QString myip=interfaces.getInterfaceIPAddress(interfaceName);

    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));

    // start a thread to listen for discovery responses
    discovery=new Discovery(myip);
    connect(discovery,SIGNAL(metis_found(Metis*)),this,SLOT(metis_found(Metis*)));
    discovery->discover();
    // disable the Discovery button
    ui->discoverPushButton->setDisabled(true);

    // wait 2 seconds to allow replys
    QTimer::singleShot(2000,this,SLOT(discovery_timeout()));
}

void MainWindow::discovery_timeout() {

    discovery->stop();
    if(ui->metisComboBox->count()>0) {
        ui->metisComboBox->setCurrentIndex(0);
        metisSelected(0);
    }

    // enable the Discovery button
    ui->discoverPushButton->setDisabled(false);

    QString text;
    text.sprintf("Discovery found %d Metis card(s)",ui->metisComboBox->count());
    status(text);
    if(ui->metisComboBox->count()==0) {
        status("Make sure the correct interface is selected.");
        status("Make sure that there is no jumper on JP1.");
    }
    QApplication::restoreOverrideCursor();
}

void MainWindow::metis_found(Metis* m) {

    if(htonl(m->getIpAddress())!=ip) {
        qDebug() << "metis_found";
        metis.append(m);
        ui->metisComboBox->addItem(m->toString());
        status(m->toString());
    }
}

void MainWindow::metisSelected(int index) {
    if(index>=0) {
        metisIP=metis.at(index)->getIpAddress();
        metisHostAddress=metis.at(index)->getHostAddress();
    }
}

void MainWindow::tabChanged(int index) {
    bootloader=(index==1);
    ui->tabWidget_2->setTabEnabled(2,bootloader);
    ui->tabWidget_2->setTabEnabled(3,bootloader);
    ui->tabWidget_2->setTabEnabled(4,bootloader);

}

void MainWindow::jtagInterrogate() {
    char errbuf[PCAP_ERRBUF_SIZE];

    ui->statusListWidget->clear();
    status("");

    handle=pcap_open_live(interfaces.getPcapName(ui->interfaceComboBox->currentText().toAscii().constData()),1024,1,TIMEOUT,errbuf);
    if (handle == NULL) {
        qDebug()<<"Couldn't open device "<<ui->interfaceComboBox->currentText().toAscii().constData()<<errbuf;
        status("Error: cannot open interface (are you running as root)");
    } else {
        rawReceiveThread=new RawReceiveThread(hw,handle);
        QObject::connect(rawReceiveThread,SIGNAL(fpgaId(unsigned char*)),this,SLOT(fpgaId(unsigned char*)));
        QObject::connect(rawReceiveThread,SIGNAL(timeout()),this,SLOT(timeout()));
        rawReceiveThread->start();

        state=JTAG_INTERROGATE;

        eraseTimeouts=0;

        status("Interrogating JTAG chain");
        sendRawCommand(GET_JTAG_DEVICE_ID);

    }
}

void MainWindow::jtagBrowse() {
    //QString fileName=QFileDialog::getOpenFileName(this,tr("Select File"),"",tr("pof Files (*.pof)"));
    QString fileName=QFileDialog::getOpenFileName(this,tr("Select File"),"",tr("rbf Files (*.rbf)"));
    ui->jtagProgramLineEdit->setText(fileName);
}

void MainWindow::jtagProgram() {

    qDebug()<<"MainWIndow::jtagProgram";

    ui->statusListWidget->clear();
    status("");

    if(fpga_id==0x020F30 || fpga_id==0x020B20) {
        if(ui->jtagProgramLineEdit->text().endsWith(".rbf")) {
            if(fpga_id==0x020F30) {
                // Mercury
                data_command=PROGRAM_MERCURY;
            } else {
                // Penelope
                data_command=PROGRAM_PENELOPE;
            }
            loadRBF(ui->jtagProgramLineEdit->text());
            if(blocks>0) {
                if(bootloader) {
                    jtagBootloaderProgram();
                } else {
                    //jtagFlashProgram();
                }
            }
        } else {
            status("No file selected.");
        }
    } else {
        status("No target defined");
    }
}

int MainWindow::loadMercuryRBF(QString filename) {
    int length;
    int i;
    int rc;

    qDebug()<<"MainWindow::loadMercuryRBF "<<filename;
    QFile rbfFile(filename);
    rbfFile.open(QIODevice::ReadOnly);
    QDataStream in(&rbfFile);
    length=((rbfFile.size()+16+255)/256)*256;
    data=(char*)malloc(length);


    qDebug() << "file size=" << rbfFile.size() << "length=" << length;
    status(filename);
    status("reading file...");
    if(in.readRawData(data,rbfFile.size())!=rbfFile.size()) {
        status("Error: could not read rbf file");
        rbfFile.close();
        QApplication::restoreOverrideCursor();
        blocks=0;
        rc=1;
    } else {
        status("file read successfully");

        // pad out to mod 256 with 0xFF
        for(i=rbfFile.size();i<length;i++) {
            data[i]=0xFF;
        }
        rbfFile.close();

        start=0;
        end=length;
        blocks=length/256;

        qDebug() <<"start="<<start<<" end="<<end<<" blocks="<<blocks;

        rc=0;
    }
    return rc;
}

int MainWindow::loadPenelopeRBF(QString filename) {
    int length;
    int i;
    int rc;

    qDebug()<<"MainWindow::loadPenelopeRBF "<<filename;

    QFile rbfFile(filename);
    rbfFile.open(QIODevice::ReadOnly);
    QDataStream in(&rbfFile);
    length=((rbfFile.size()-44+255)/256)*256;
    data=(char*)malloc(length+44);


    qDebug() << "file size=" << rbfFile.size() << "length=" << length;
    status(filename);
    status("reading file...");
    if(in.readRawData(data,rbfFile.size())!=rbfFile.size()) {
        status("Error: could not read rbf file");
        rbfFile.close();
        QApplication::restoreOverrideCursor();
        blocks=0;
        rc=1;
    } else {
        status("file read successfully");

        // pad out to mod 256 with 0xFF
        for(i=rbfFile.size();i<length;i++) {
            data[i]=0xFF;
        }
        rbfFile.close();

        start=44;
        end=length;
        blocks=length/256;

        qDebug() <<"start="<<start<<" end="<<end<<" blocks="<<blocks;

        rc=0;
    }
    return rc;
}

void MainWindow::jtagBootloaderProgram() {
    char errbuf[PCAP_ERRBUF_SIZE];

    qDebug()<<"MainWindow::jtagBootloaderProgram";

    ui->statusListWidget->clear();
    status("");

    handle=pcap_open_live(interfaces.getPcapName(ui->interfaceComboBox->currentText().toAscii().constData()),1024,1,TIMEOUT,errbuf);
    if (handle == NULL) {
        qDebug()<<"Couldn't open device "<<ui->interfaceComboBox->currentText().toAscii().constData()<<errbuf;
        status("Error: cannot open interface (are you running as root)");
    } else {
        rawReceiveThread=new RawReceiveThread(hw,handle);
        rawReceiveThread->start();
        QObject::connect(rawReceiveThread,SIGNAL(commandCompleted()),this,SLOT(commandCompleted()));
        QObject::connect(rawReceiveThread,SIGNAL(nextBuffer()),this,SLOT(nextBuffer()));
        QObject::connect(rawReceiveThread,SIGNAL(timeout()),this,SLOT(timeout()));

        state=JTAG_PROGRAM;
        sendRawData();
    }
}

// private function to send the command to erase
void MainWindow::jtagEraseData() {
    eraseTimeouts=0;
    sendRawCommand(JTAG_ERASE_FLASH);
}

void MainWindow::jtagFlashBrowse() {
    //QString fileName=QFileDialog::getOpenFileName(this,tr("Select File"),"",tr("pof Files (*.pof)"));
    QString fileName=QFileDialog::getOpenFileName(this,tr("Select File"),"",tr("rbf Files (*.rbf)"));
    ui->jtagFlashProgramLineEdit->setText(fileName);
}

void MainWindow::jtagFlashProgram() {
    char errbuf[PCAP_ERRBUF_SIZE];

    qDebug()<<"MainWIndow::jtagFlashProgram";
    ui->statusListWidget->clear();
    status("");

    // validate file selection
    if(!ui->jtagProgramLineEdit->text().endsWith(".rbf")) {
        status("Error: No JTAG Program file selected.");
        return;
    }
    if(!ui->jtagFlashProgramLineEdit->text().endsWith(".rbf")) {
        status("Error: No Flash Program file selected");
        return;
    }

    // try to load the JTAG.rbf file
    if(fpga_id==0x020F30) {
        // Mercury
        if(loadMercuryRBF(ui->jtagProgramLineEdit->text())!=0) {
            status("Error: Failed to load Mercury_JTAG Program file.");
            return;
        }
        data_command=PROGRAM_MERCURY;
    } else if(fpga_id==0x020B20) {
        // Penelope
        if(loadPenelopeRBF(ui->jtagProgramLineEdit->text())!=0) {
            status("Error: Failed to load Penelope_JTAG Program file.");
            return;
        }
        data_command=PROGRAM_PENELOPE;
    } else {
        status("Error: Undefined FPGA ID.");
        return;
    }

    handle=pcap_open_live(interfaces.getPcapName(ui->interfaceComboBox->currentText().toAscii().constData()),1024,1,TIMEOUT,errbuf);
    if (handle == NULL) {
        qDebug()<<"Couldn't open device "<<ui->interfaceComboBox->currentText().toAscii().constData()<<errbuf;
        status("Error: cannot open interface (are you running as root)");
    }

    rawReceiveThread=new RawReceiveThread(hw,handle);
    rawReceiveThread->start();
    QObject::connect(rawReceiveThread,SIGNAL(eraseCompleted()),this,SLOT(eraseCompleted()));
    QObject::connect(rawReceiveThread,SIGNAL(nextBuffer()),this,SLOT(nextJTAGBuffer()));
    QObject::connect(rawReceiveThread,SIGNAL(timeout()),this,SLOT(timeout()));

    // send the JTAG.rbf file
    state=JTAG_PROGRAM;
    offset=start;

    sendJTAGData();
}

void MainWindow::nextJTAGBuffer() {
    qDebug()<<"MainWIndow::nextJTABBuffer state="<<state<<" offset="<<offset;
    if(state==JTAG_PROGRAM) {
        offset+=256;
        if(offset<end) {
            sendJTAGData();
        } else {
            status("Loaded successfully.");

            // wait 2 seconds to start erase
            QTimer::singleShot(2000,this,SLOT(startJTAGFlashErase()));

        }
    } else { // FLASH Programming
        offset+=256;
        if(offset<end) {
            sendJTAGFlashData();
        } else {
            status("Loaded Flash successfully.");
            status("Remember to remove JP1 when you power cycle");
            idle();
        }
    }
}

void MainWindow::startJTAGFlashErase() {
    state=FLASH_ERASING;
    status("Erasing Flash ... (takes several seconds)");
    jtagEraseData();
}

void MainWindow::loadFlash() {
    qDebug()<<"MainWindow::loadFlash";
    if(loadRBF(ui->jtagFlashProgramLineEdit->text())!=0) {
        status("Error: Failed to load Flash Program file.");
        return;
    }
    data_command=PROGRAM_MERCURY;
    state=FLASH_PROGRAM;
    offset=start;
    sendJTAGFlashData();
}
