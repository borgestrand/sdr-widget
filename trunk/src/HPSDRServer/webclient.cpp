#include "webclient.h"

#include <QtNetwork/QTcpSocket>
#include <QTextStream>
#include <QStringList>
#include <QTimer>

#include <QDebug>

 WebClient::WebClient(QTcpSocket* socket,Server* s)
 {
     //qDebug()<<"WebClient::WebClient";

     server=s;

     connect(socket, SIGNAL(readyRead()), this, SLOT(readClient()));
     connect(socket, SIGNAL(disconnected()), this, SLOT(discardClient()));

     //qDebug()<<"WebClient::WebClient ... exit";
 }

 void WebClient::readClient() {
     //qDebug()<<"WebClient::readClient";
     QTcpSocket* tcpSocket = (QTcpSocket*)sender();

     if (tcpSocket->canReadLine()) {
         QByteArray buffer=tcpSocket->readLine();
         //qDebug()<<buffer;

         // parse the request
         QStringList tokens = QString(buffer).split(QRegExp("[ \r\n][ \r\n]*"));

         // extract the method
         method=tokens[0];  // should be GET/PUT/etc
         //qDebug()<<"METHOD = "<<method;

         // parse the arguments
         QStringList arguments = tokens[1].split(QRegExp("[?&\r\n]"));

         // the first argument is the url
         url=arguments[0];
         //qDebug()<<"URL = "<<url;

         // remaining arguments are really arguments
         int nArguments=arguments.count();
         for(int i=1;i<nArguments;i++) {
             //qDebug()<<"argument ="<<arguments[i];
             QStringList argument=arguments[i].split("=");
             if(argument.count()==2) {
                 params[argument[0]]=argument[1];
             }
         }

         outputStream=new QTextStream(tcpSocket);
         outputStream->setAutoDetectUnicode(true);

         // set the HTTP header and content type
         *outputStream << "HTTP/1.0 200 Ok\r\n"
                 "Content-Type: text/html; charset=\"utf-8\"\r\n"
                 "\r\n";

         // process the request
         if(url=="/device.html") {
             device();
         } else if(url=="/discover.html") {
             discover();
         } else if(url=="/start.html") {
             start();
         } else if(url=="/stop.html") {
             stop();
         } else if(url=="/configure.html") {
             configure();
         }

         // just one page to display
         index();

         tcpSocket->close();
         //qDebug()<<"closed socket";

         if (tcpSocket->state() == QTcpSocket::UnconnectedState) {
             delete tcpSocket;
             //qDebug()<<"deleted socket";
         }
     }
     //qDebug()<<"WebClient::readClient ... exit";
}

 void WebClient::discardClient()
 {
     //qDebug()<<"WebClient::discardClient";
     /*
     QTcpSocket* socket = (QTcpSocket*)sender();
     socket->close();
     if (socket->state() == QTcpSocket::UnconnectedState) {
         delete socket;
     }
     */
     //qDebug()<<"WebClient::discardClient ... exit";
 }

 void WebClient::index() {
     *outputStream << "<html>\r\n"
             "<head>\r\n"
             "<title>HPSDRServer</title>\r\n";
     if(url=="/" || url=="/index.html") {
         if(server->getState()==Server::RUNNING) {
             *outputStream << "<meta http-equiv=\"refresh\" content=\"10;url=index.html\">\r\n";
         }
         *outputStream << "</head>\r\n"
                 "<body>\r\n"
                 "<table border=\"0\" width=\"800\">\r\n"
                 "<tr bgcolor=\"lightblue\">\r\n"
                 "<td>\r\n"
                 "HPSDRServer";
         if(server->getState()==Server::RUNNING) {
             *outputStream << " (RUNNING)";
         } else {
             *outputStream << " (STOPPED)";
         }
         *outputStream << "</td>\r\n"
                 "<td align=\"right\"><a href=\"index.html\">Refresh</a></td>"
                 "</tr>\r\n"
                 "</table>\r\n";

         *outputStream << "<table width=\"800\">\r\n"
                       << "<tr bgcolor=\"skyblue\" width=\"100%\">\r\n"
                       << "<td>Device: <form action=\"device.html\">"
                       << "<SELECT"
                       << ((server->getState()==Server::RUNNING)?" disabled ":"")
                       << " name=\"device\">\r\n"
                       << server->getDevicesHTML()
                       << "</SELECT>\r\n"
                       << "<input"
                       << ((server->getState()==Server::RUNNING)?" disabled ":"")
                       << " type='SUBMIT' value='Apply'>"
                       << "</form>\r\n"
                       <<   "</td>\r\n"
                       << "</tr>\r\n"
                       << "</table>\r\n";

         if(server->getDevice()=="Metis") {
             *outputStream << "<table width=\"800\">\r\n"
                           << "<tr bgcolor=\"skyblue\" width=\"100%\">\r\n"
                           <<   "<td>Interface: <form action=\"discover.html\">"
                           <<  "<SELECT"
                           << ((server->getState()==Server::RUNNING)?" disabled":"")
                           <<  " name=\"interface\">\r\n"
                           << server->getInterfacesHTML()
                           << "</SELECT>\r\n"
                           <<     "<input"
                           << ((server->getState()==Server::RUNNING)?" disabled ":"")
                           <<     " type='SUBMIT' value='Discover'>"
                           <<     "</form>\r\n"
                           <<   "</td>\r\n"
                           << "</tr>\r\n"
                           << "</table>\r\n";

             if(server->getInterface()!="") {
                 *outputStream << "<table width=\"800\">\r\n"
                                  "<tr bgcolor=\"skyblue\" width=\"100%\">\r\n";
                 if(server->getState()==Server::STOPPED) {
                     *outputStream << "<td>Metis: <form action=\"start.html\">\r\n";
                 } else {
                     *outputStream << "<td>Metis: <form action=\"stop.html\">\r\n";
                 }
                 *outputStream << "<SELECT"
                               << ((server->getState()==Server::RUNNING)?" disabled ":"")
                               << " name=\"metis\">\r\n";
                 *outputStream << server->getMetisHTML();
                 *outputStream << "</SELECT>\r\n";
                 if(server->getMetisCount()>0) {
                     if(server->getState()==Server::STOPPED) {
                         *outputStream << "<input type='SUBMIT' value='Start'>";
                     } else {
                         *outputStream << "<input type='SUBMIT' value='Stop'>";
                     }
                 }
                 *outputStream << "</form>\r\n"
                                    "</td>\r\n"
                                  "</tr>\r\n"
                                  "</table>\r\n";
             }

         } else if(server->getDevice()=="Ozy") {

             *outputStream << "<table width=\"800\">\r\n"
                              "<tr bgcolor=\"skyblue\" width=\"100%\">\r\n";
             if(server->getState()==Server::STOPPED) {
                 *outputStream << "<td>Ozy: <form action=\"start.html\">\r\n"
                                  "<input type='SUBMIT' value='Start'>";
             } else {
                 *outputStream << "<td>Ozy: <form action=\"stop.html\">\r\n"
                                  "<input type='SUBMIT' value='Stop'>";
             }
             *outputStream << "</form>\r\n"
                                "</td>\r\n"
                              "</tr>\r\n"
                              "</table>\r\n";

         }

         *outputStream << "<table width=\"800\">\r\n"
                          "<tr bgcolor=\"skyblue\" width=\"100%\">\r\n"
                          "<td><form action=\"configure.html\">";

         // sample rate
         *outputStream << "Sample Rate: <SELECT name=\"samplerate\">"
                          "<OPTION VALUE=\"48000\"";
         if(server->getSampleRate()=="48000") {
             *outputStream << " SELECTED";
         }
         *outputStream << ">48000</OPTION>"
                          "<OPTION VALUE=\"96000\"";
         if(server->getSampleRate()=="96000") {
             *outputStream << " SELECTED";
         }
         *outputStream << ">96000</OPTION>"
                          "<OPTION VALUE=\"192000\"";
         if(server->getSampleRate()=="192000") {
             *outputStream << " SELECTED";
         }
         *outputStream << ">192000</OPTION>"
                          "</SELECT>";

         // receivers
         *outputStream << "<BR>Receivers: <SELECT name=\"receivers\">"
                          "<OPTION VALUE=\"1\"";
         if(server->getReceivers()=="1") {
             *outputStream << " SELECTED";
         }
         *outputStream << ">1</OPTION>"
                          "<OPTION VALUE=\"2\"";
         if(server->getReceivers()=="2") {
             *outputStream << " SELECTED";
         }
         *outputStream << ">2</OPTION>"
                          "<OPTION VALUE=\"3\"";
         if(server->getReceivers()=="3") {
             *outputStream << " SELECTED";
         }
         *outputStream << ">3</OPTION>"
                          "<OPTION VALUE=\"4\"";
         if(server->getReceivers()=="4") {
             *outputStream << " SELECTED";
         }
         *outputStream << ">4</OPTION>"
                          "</SELECT>";

         // 10MHz clock source
         *outputStream << "<BR>10 Mhz clock source:";

         *outputStream << "<SELECT name=\"clock10\">";

         *outputStream << "<OPTION VALUE=\"Atlas\"";
         if(server->get10MHzClock()=="Atlas") {
             *outputStream << " SELECTED";
         }
         *outputStream << ">Atlas</OPTION>";

         *outputStream << "<OPTION VALUE=\"Mercury\"";
         if(server->get10MHzClock()=="Mercury") {
             *outputStream << " SELECTED";
         }
         *outputStream << ">Mercury</OPTION>";

         *outputStream << "<OPTION VALUE=\"Penelope\"";
         if(server->get10MHzClock()=="Penelope") {
             *outputStream << " SELECTED";
         }
         *outputStream << ">Penelope</OPTION>";

         *outputStream << "</SELECT>";

         // 122.88MHz clock source
         *outputStream << "<BR>122.88 Mhz clock source:";

         *outputStream << "<SELECT name=\"clock122_88\">";

         *outputStream << "<OPTION VALUE=\"Mercury\"";
         if(server->get122_88MHzClock()=="Mercury") {
             *outputStream << " SELECTED";
         }
         *outputStream << ">Mercury</OPTION>";

         *outputStream << "<OPTION VALUE=\"Penelope\"";
         if(server->get122_88MHzClock()=="Penelope") {
             *outputStream << " SELECTED";
         }
         *outputStream << ">Penelope</OPTION>";

         *outputStream << "</SELECT>";

         *outputStream << "<BR>Preamp: <INPUT TYPE=\"CHECKBOX\" NAME=\"preamp\"";
         if(server->getPreamp()=="on") {
             *outputStream << " CHECKED";
         }
         *outputStream << ">";

         *outputStream << "<BR>Random: <INPUT TYPE=\"CHECKBOX\" NAME=\"random\"";
         if(server->getRandom()=="on") {
             *outputStream << " CHECKED";
         }
         *outputStream << ">";

         *outputStream << "<BR>Dither: <INPUT TYPE=\"CHECKBOX\" NAME=\"dither\"";
         if(server->getDither()=="on") {
             *outputStream << " CHECKED";
         }
         *outputStream << ">";

         *outputStream << "<BR>Class E: <INPUT TYPE=\"CHECKBOX\" NAME=\"classe\"";
         if(server->getClassE()=="on") {
             *outputStream << " CHECKED";
         }
         *outputStream << ">";

         *outputStream << "<BR>Line In: <INPUT TYPE=\"CHECKBOX\" NAME=\"linein\"";
         if(server->getLineIn()=="on") {
             *outputStream << " CHECKED";
         }
         *outputStream << ">";

         *outputStream << "<BR>Mic Boost (20dB): <INPUT TYPE=\"CHECKBOX\" NAME=\"micboost\"";
         if(server->getMicBoost()=="on") {
             *outputStream << " CHECKED";
         }
         *outputStream << ">";
         *outputStream << "<BR>Mic Gain: <INPUT TYPE=\"TEXT\" NAME=\"micgain\" VALUE=\"";
         *outputStream << server->getMicGain();
         *outputStream << "\">";

         *outputStream << "<br><input type='SUBMIT' value='Configure'>"
                          "</form>\r\n"
                            "</td>\r\n";

         *outputStream << "<td>\r\n";
         QString control;
         control.sprintf("%02X %02X %02X %02X %02X",server->getControlOut(0),server->getControlOut(1),server->getControlOut(2),server->getControlOut(3),server->getControlOut(4));
         *outputStream << "control out:" << control;
         *outputStream << "<br>mox:" << server->getMox();
         *outputStream << "<br>Mic Gain:" << server->getMicGain();
         *outputStream << "</td>\r\n";

         *outputStream << "</tr>\r\n"
                          "</table>\r\n";

         if(server->getState()==Server::RUNNING) {

             // show software versions
             *outputStream << "<table width=\"800\">\r\n"
                              "<tr bgcolor=\"skyblue\" width=\"100%\">\r\n"
                              "<td>";

             *outputStream << server->getDevice()<<" Software Version: ";
             *outputStream << server->getOzySoftwareVersion();
             *outputStream << "<BR>Mercury Software Version: ";
             *outputStream << server->getMercurySoftwareVersion();
             *outputStream << "<BR>Penelope Software Version: ";
             *outputStream << server->getPenelopeSoftwareVersion();

             // show received frame count
             *outputStream << "<BR><BR>Received Frames: "<< server->getReceivedFrames();
             if(server->getDevice()=="Metis") {
                 *outputStream << "<BR>Receive Sequence Errors: "<< server->getReceiveSequenceError();
             }


             *outputStream << "</td>\r\n";

             // show receivers
             *outputStream << "<td>";

             *outputStream << server->getReceiversHTML();

             *outputStream << "</td>\r\n"
                              "</tr>\r\n"
                              "</table>\r\n";


         }

         *outputStream << server->getErrorHTML();

         *outputStream << "</body>\r\n"
                 "</html>\r\n";
     } else {
         *outputStream << "<meta http-equiv=\"refresh\" content=\"0;url=index.html\">\r\n";
         *outputStream << "</head>\r\n"
                 "<body>\r\n"
                 "</body>\r\n"
                 "</html>\r\n";
     }

 }

 void WebClient::device() {
     server->setDevice(params["device"]);
 }

 void WebClient::discover() {
     qDebug()<<"WebClient::discover";
     QString i=QUrl::fromPercentEncoding(params["interface"].toLocal8Bit());
     i.replace("+"," ");
     server->clearError();
     server->setInterface(i);
     server->clearMetis();
     server->bind();

     Discovery* discovery=new Discovery(server);
 }

 void WebClient::start() {
     qDebug()<<"WebClient::start";
     QString m=QUrl::fromPercentEncoding(params["metis"].toLocal8Bit());
     m.replace("+"," ");

     qDebug()<<m;

     server->clearError();
     server->setMetis(m);
     server->start();
 }

 void WebClient::stop() {
     qDebug()<<"WebClient::stop";
     server->clearError();
     server->stop();
 }

 void WebClient::configure() {
     server->setSampleRate(params["samplerate"]);
     server->setReceivers(params["receivers"]);
     server->set10MHzClock(params["clock10"]);
     server->set122_88MHzClock(params["clock122_88"]);
     server->setPreamp(params["preamp"]);
     server->setRandom(params["random"]);
     server->setDither(params["dither"]);
     server->setClassE(params["classe"]);
     server->setLineIn(params["linein"]);
     server->setMicBoost(params["micboost"]);
     server->setMicGain(atof(params["micgain"].toAscii().constData()));
 }
