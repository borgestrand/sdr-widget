/*!
 * \file MainWindow.cpp
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



#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    version = "0.1";
    serverState = false;
    ser = new Server();

    connect( ui->pushButton, SIGNAL(clicked()), this, SLOT(startServer()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ser->timer, SIGNAL(timeout()), ser, SLOT(sendDatagrams()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::startServer()
{
    ser->initSocket();
    if( serverState == false )
    {
        ui->pushButton->setText("Stop");
        ui->statusBar->showMessage("Server started");
        ser->udpSocket->connectToHost(QHostAddress::LocalHost, 15000);
        serverState = true;
    }else{
        ui->pushButton->setText("Start");
        ui->statusBar->showMessage("Server stopped");
        ser->udpSocket->disconnect();
        serverState = false;
    }

}

void MainWindow::about()
{
    QString str;
    str = "<h2> Test Server program </h2> Written by Dave Larsen, KV0S";
    str.append("<br>part of the HPSDR Athena project ");
    str.append("<br>Copyright 2009 <br>Version ");
    str.append( version);
    str.append("<br>Compiled using Qt Version ");
    str.append( qVersion() );
    str.append("<br>For more information,  <a href=\"http://openhpsdr.org/wiki/index.php?title=ATHENA\">  http://openhpsdr.org/wiki/index.php?title=ATHENA  </a>");
    QMessageBox::about(this, "About Test Server", str );

 }
