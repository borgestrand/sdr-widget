#include "server.h"
#include "client.h"

#include <QUdpSocket>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

Server::Server() {

    // defaults
    device="Ozy";
    iface="";
    metis="";

    state=Server::STOPPED;

    receive_sequence=0;
    receive_sequence_error=0;

    sampleRate="96000";
    receivers="1";

    send_rx_frequency=0;
    send_tx_frequency=0;

    penny_change=1;

    for(int i=0;i<4;i++) {
        receiver[i]=new Receiver();
        receiver[i]->init(i);
        receiver[i]->setPlayAudio(i==0);
    }
    samples=0;

    mic_gain=0.26F;

    control_out[0]=(unsigned char)0x00; // MOX_DISABLED
    control_out[1]=(unsigned char)0xF9; // CONFIG_BOTH| MERCURY_122_88MHZ_SOURCE | MERCURY_10MHZ_SOURCE | MIC_SOURCE_PENELOPE | SPEED_96KHZ
    control_out[2]=(unsigned char)0x00; // MODE_OTHERS
    control_out[3]=(unsigned char)0x18; // ALEX_ATTENUATION_0DB | LT2208_GAIN_OFF | LT2208_DITHER_ON | LT2208_RANDOM_ON
    control_out[4]=(unsigned char)0x04; // DUPLEX

    clock10MHz="Mercury";
    clock122_88MHz=="Mercury";
    preamp="";
    random="on";
    dither="on";
    metis_buffer_index=8;
    duplex="";
    classE="";

    line_in="";
    mic_boost="";
    ptt=0;
    dot=0;
    dash=0;
    mox=0;
}

QUdpSocket* Server::getSocket() {
    return &socket;
}

void Server::bind() {
    socket.close();

    if(!socket.bind(QHostAddress(getInterfaceIPAddress(getInterface())),0,QUdpSocket::ReuseAddressHint)) {
        error.set("Error: Server::bind: bind failed");
        qDebug()<<"Error: Server::bind bind failed "<<socket.errorString();
        return;
    }
}

void Server::setDevice(QString d) {
    device=d;
}

void Server::setInterface(QString i) {
    iface=i;
}

void Server::setMetis(QString m) {
    metis=m;
}

QString Server::getDevice() {
    return device;
}

QString Server::getInterface() {
    return iface;
}

QString Server::getMetis() {
    return metis;
}

QString Server::getDevicesHTML() {
    QString s="<OPTION VALUE=\"Ozy\"";
    if(device=="Ozy") s.append(" SELECTED");
    s.append("> Ozy</OPTION>\r\n");
    s.append("<OPTION VALUE=\"Metis\"");
    if(device=="Metis") s.append(" SELECTED");
    s.append("> Metis</OPTION>\r\n");
    return s;
}

QString Server::getInterfacesHTML() {
    QString s="";
    int i;

    for(i=0;i<interfaces.count();i++) {
        s.append("<OPTION VALUE=\"");
        s.append(interfaces.getInterfaceNameAt(i));
        s.append("\"");
        if(iface==interfaces.getInterfaceNameAt(i)) s.append(" SELECTED");
        s.append("> ");
        s.append(interfaces.getInterfaceNameAt(i));
        s.append(" (");
        s.append(interfaces.getInterfaceIPAddress(interfaces.getInterfaceNameAt(i)));
        s.append(")");
        s.append("</OPTION>\r\n");
    }

    return s;
}

QString Server::getInterfaceIPAddress(QString iface) {
    return interfaces.getInterfaceIPAddress(iface);
}

QString Server::getMetisHTML() {
    QString s="";
    for(int i=0;i<metisCards.count();i++) {
        Metis m=metisCards.at(i);
        s.append("<OPTION VALUE=\"");
        s.append(m.toString());
        s.append("\"");
        if(metis==m.toString()) s.append(" SELECTED");
        s.append("> ");
        s.append(m.toString());
        s.append("</OPTION>\r\n");
    }
    return s;
}

void Server::clearMetis() {
    metisCards.clear();
}

void Server::addMetis(Metis metis) {
    metisCards.append(metis);
}

int Server::getMetisCount() {
    return metisCards.count();
}

int Server::getOzySoftwareVersion() {
    return ozy_software_version;
}

int Server::getMercurySoftwareVersion() {
    return mercury_software_version;
}

int Server::getPenelopeSoftwareVersion() {
    return penelope_software_version;
}

int Server::getReceiveSequenceError() {
    return receive_sequence_error;
}

int Server::getReceivedFrames() {
    return rx_frame;
}

QString Server::attach(int rx,Client* c) {
    if(c==(Client*)0) {
        qDebug()<<"Server::attach client is null";
    }
    return receiver[rx]->attach(c);
}

void Server::start() {

    qDebug()<<"Server::start";

    error.clear();

    if(device=="Metis") {
        receive_sequence=0;
        startMetis();
        state=Server::RUNNING;
        samples=0;
    } else if(device=="Ozy") {
        // nothing to do for Ozy
    } else {
        // no device selected !!!
    }

    // send 2 frames to get it really started
    start_frames=2;
    qDebug()<<"Server::start send frames";
    while(start_frames>0) {
        sendBuffer();
        start_frames--;
    }

    qDebug()<<"Server::start set receiver frequency";
    for(int i=0;i<receivers.toInt();i++) {
        receiver[i]->setFrequency(7056000);
        current_receiver=i;
        sendBuffer();
    }

    current_receiver=0;


    // startup the client server
    clientServer=new ClientServer(this);
}

void Server::stop() {
    if(device=="Metis") {
        state=Server::STOPPED;
        stopMetis();
    } else if(device=="Ozy") {
    } else {
    }
}

void Server::startMetis() {
    int i;

    qDebug()<<"Server::startMetis";

    rx_frame=0;
    send_sequence=-1;
    offset=8;

    // find the Metis card
    foreach (Metis m, metisCards) {
        if(metis==m.toString()) {
            metisAddress=m.getHostAddress();

            qDebug()<<"startMetis on interface"<<getInterface()<<getInterfaceIPAddress(getInterface());
            qDebug()<<"startMetis:"<<metis;
            //if(!socket->bind(QHostAddress(getInterfaceIPAddress(getInterface())),1024,QUdpSocket::ReuseAddressHint)) {
            //    error.set("Error: Server::startMetis: bind failed");
            //    qDebug()<<"Error: Server::startMetis bind failed "<<socket.errorString();
            //    return;
            //}

            connect(&socket,SIGNAL(readyRead()),this,SLOT(readyRead()));

            unsigned char buffer[64];

            buffer[0]=(char)0xEF;
            buffer[1]=(char)0XFE;
            buffer[2]=(char)0x04;
            buffer[3]=(char)0x01;
            for(i=4;i<64;i++) {
                buffer[i]=(char)0x00;
            }

            qDebug()<<"writeDatagram start command"<<metisAddress->toString();
            if(socket.writeDatagram((const char*)buffer,sizeof(buffer),*metisAddress,1024)<0) {
                qDebug()<<"Error: Discovery: writeDatagram failed "<<socket.errorString();
                return;
            }

            socket.flush();

            break;

        }
    }

}

void Server::stopMetis() {
    int i;

    qDebug()<<"Server::stopMetis";
    // find the Metis card
    foreach (Metis m, metisCards) {
        if(metis==m.toString()) {
            metisAddress=m.getHostAddress();

            qDebug()<<"Server::stopMetis metisAddress "<<metisAddress->toString();

            unsigned char buffer[64];

            buffer[0]=(char)0xEF;
            buffer[1]=(char)0XFE;
            buffer[2]=(char)0x04;
            buffer[3]=(char)0x00;
            for(i=4;i<64;i++) {
                buffer[i]=(char)0x00;
            }

            disconnect(&socket,SIGNAL(readyRead()),this,SLOT(readyRead()));

            qDebug()<<"Server::stopMetis writeDatagram";

            if(socket.writeDatagram((const char*)buffer,sizeof(buffer),*metisAddress,1024)<0) {
                qDebug()<<"Error: Server::stopMetis: writeDatagram failed "<<socket.errorString();
                return;
            }

            socket.flush();
            //socket.close();

            break;
        }
    }

}


void Server::readyRead() {

    QHostAddress metisAddress;
    quint16 metisPort;
    unsigned char receiveBuffer[1032];
    qint64 length;

    long sequence;

    if((length=socket.readDatagram((char*)&receiveBuffer,(qint64)sizeof(receiveBuffer),&metisAddress,&metisPort))!=1032) {
        qDebug()<<"Error: Server: readDatagram failed "<<socket.errorString();
        return;
    }

    //qDebug()<<"receiver"<<length<<"bytes: "<<receiveBuffer[0]<<","<<receiveBuffer[1]<<","<<receiveBuffer[2]<<","<<receiveBuffer[3];

    if(receiveBuffer[0]==0xEF && receiveBuffer[1]==0xFE) {
        // valid frame
        switch(receiveBuffer[2]) {
        case 1: // IQ data
            switch(receiveBuffer[3]) {
            case 4: // EP4 data
                break;
            case 6: // EP6 data
                sequence=((receiveBuffer[4]&0xFF)<<24)+((receiveBuffer[5]&0xFF)<<16)+((receiveBuffer[6]&0xFF)<<8)+(receiveBuffer[7]&0xFF);
                if(receive_sequence==0) {
                    receive_sequence=sequence;
                } else {
                    receive_sequence++;
                    if(receive_sequence!=sequence) {
                        //qDebug()<<"Sequence error: expected "<<receive_sequence<<" got "<<sequence;
                        receive_sequence=sequence;
                        receive_sequence_error++;
                    }
                }
                process_iq_buffer(&receiveBuffer[8]);
                process_iq_buffer(&receiveBuffer[520]);
                break;
            default:
                qDebug()<<"invalid EP";
                break;
            }
            break;
        default:
            qDebug()<<"Expected data packet (1) got "<<receiveBuffer[2];
            break;
        }
    } else {
        qDebug()<<"expected EFFE";
    }
}

void Server::process_iq_buffer(unsigned char* buffer) {
    int b=0;
    int b_max;
    int r;
    int left_sample,right_sample,mic_sample;
    float left_sample_float,right_sample_float,mic_sample_float;

    //qDebug()<<"process_iq_buffer";
    //if(rx_frame<10) {
    //    dump_ozy_buffer("received from Ozy:",rx_frame,buffer);
    //}

    if(buffer[b++]==0x7F && buffer[b++]==0x7F && buffer[b++]==0x7F) {

        // extract control bytes
        control_in[0]=buffer[b++];
        control_in[1]=buffer[b++];
        control_in[2]=buffer[b++];
        control_in[3]=buffer[b++];
        control_in[4]=buffer[b++];

        // extract PTT, DOT and DASH
        ptt=(control_in[0]&0x01)==0x01;
        dash=(control_in[0]&0x02)==0x02;
        dot=(control_in[0]&0x04)==0x04;

        switch((control_in[0]>>3)&0x1F) {
        case 0:
            lt2208ADCOverflow=control_in[1]&0x01;
            IO1=(control_in[1]&0x02)?0:1;
            IO2=(control_in[1]&0x04)?0:1;
            IO3=(control_in[1]&0x08)?0:1;
            if(mercury_software_version!=control_in[2]) {
                mercury_software_version=control_in[2];
                fprintf(stderr,"  Mercury Software version: %d (0x%0X)\n",mercury_software_version,mercury_software_version);
            }
            if(penelope_software_version!=control_in[3]) {
                penelope_software_version=control_in[3];
                fprintf(stderr,"  Penelope Software version: %d (0x%0X)\n",penelope_software_version,penelope_software_version);
            }
            if(ozy_software_version!=control_in[4]) {
                ozy_software_version=control_in[4];
                fprintf(stderr,"  Ozy Software version: %d (0x%0X)\n",ozy_software_version,ozy_software_version);
            }
            break;
                    case 1:
            forwardPower=(control_in[1]<<8)+control_in[2]; // from Penelope or Hermes

            alexForwardPower=(control_in[3]<<8)+control_in[4]; // from Alex or Apollo
            break;
                    case 2:
            alexForwardPower=(control_in[1]<<8)+control_in[2]; // from Alex or Apollo
            AIN3=(control_in[3]<<8)+control_in[4]; // from Pennelope or Hermes
            break;
                    case 3:
            AIN4=(control_in[1]<<8)+control_in[2]; // from Pennelope or Hermes
            AIN6=(control_in[3]<<8)+control_in[4]; // from Pennelope or Hermes
            break;
        }

        switch(receivers.toInt()) {
        case 1: b_max=512-0; break;
        case 2: b_max=512-0; break;
        case 3: b_max=512-4; break;
        case 4: b_max=512-10; break;
        case 5: b_max=512-24; break;
        case 6: b_max=512-10; break;
        case 7: b_max=512-20; break;
        case 8: b_max=512-4; break;
        }

        // extract the samples
        while(b<b_max) {
            // extract each of the receivers
            for(r=0;r<receivers.toInt();r++) {
                left_sample   = (int)((signed char)buffer[b++]) << 16;
                left_sample  += (int)((unsigned char)buffer[b++]) << 8;
                left_sample  += (int)((unsigned char)buffer[b++]);
                right_sample  = (int)((signed char)buffer[b++]) << 16;
                right_sample += (int)((unsigned char)buffer[b++]) << 8;
                right_sample += (int)((unsigned char)buffer[b++]);
                left_sample_float=(float)left_sample/8388607.0; // 24 bit sample
                right_sample_float=(float)right_sample/8388607.0; // 24 bit sample
                receiver[r]->put_iq_samples(samples,left_sample_float,right_sample_float);
            }
            mic_sample    = (int)((signed char) buffer[b++]) << 8;
            mic_sample   += (int)((unsigned char)buffer[b++]);
            mic_sample_float=(float)mic_sample/32767.0*mic_gain; // 16 bit sample
            for(r=0;r<receivers.toInt();r++) {
                receiver[r]->put_mic_samples(samples,mic_sample_float);
            }
            samples++;

            // when we have enough samples send them to the clients
            if(samples==1024) {
                // send I/Q and mic data to clients
                for(r=0;r<receivers.toInt();r++) {
                    receiver[r]->send_IQ_buffer();
                }
                samples=0;
            }
        }

    } else {
        qDebug()<<"Server::process_iq_buffer SYNC Error";
    }

    rx_frame++;

}

Server::STATES Server::getState() {
    return state;
}

void Server::setSampleRate(QString s) {
    qDebug()<<"Server::setSampleRate "<<s;
    unsigned char speed=0x00;
    sampleRate=s;

    control_out[1] &= 0xfc;
    switch(sampleRate.toInt()) {
    case 48000:
        speed=0x00;
        break;
    case 96000:
        speed=0x01;
        break;
    case 192000:
        speed=0x02;
        break;
    }
    control_out[1] |= speed;
}

void Server::setReceivers(QString r) {
    qDebug()<<"Server::setReceivers "<<r;
    receivers=r;
    control_out[4] &= 0xc7;
    control_out[4] |= (r.toInt()-1)<<3;
}

QString Server::getSampleRate() {
    return sampleRate;
}

QString Server::getReceivers() {
    return receivers;
}

void Server::set10MHzClock(QString c) {
    qDebug()<<"Server::set10MHzClock "<<c;
    int source=0;
    clock10MHz=c;
    if(c=="Atlas") {
        source=0;
    } else if(c=="Mercury") {
        source=2;
    } else if(c=="Penelope") {
        source=1;
    }
    control_out[1]=control_out[1]&0xF3;
    control_out[1]=control_out[1]|(source<<2);
}

QString Server::get10MHzClock() {
    return clock10MHz;
}

void Server::set122_88MHzClock(QString c) {
    qDebug()<<"Server::set122_88MHzClock "<<c;
    int source=0;
    clock122_88MHz=c;
    if(c=="Mercury") {
        source=1;
    } else if(c=="Penelope") {
        source=0;
    }
    control_out[1]=control_out[1]&0xEF;
    control_out[1]=control_out[1]|(source<<4);
}

QString Server::get122_88MHzClock() {
    return clock122_88MHz;
}

void Server::setPreamp(QString s) {
    qDebug()<<"Server::setPreamp "<<s;
    int p=0;
    preamp=s;
    p=(s=="on");
    control_out[3]=control_out[3]&0xFB;
    control_out[3]=control_out[3]|(p<<2);
}

QString Server::getPreamp() {
    return preamp;
}

void Server::setRandom(QString s) {
    qDebug()<<"Server::setRandom "<<s;
    int r=0;
    random=s;
    r=(s=="on");
    control_out[3]=control_out[3]&0xEF;
    control_out[3]=control_out[3]|(r<<4);
}

QString Server::getRandom() {
    return random;
}

void Server::setDither(QString s) {
    qDebug()<<"Server::setDither "<<s;
    int d=0;
    d=(s=="on");
    dither=s;
    control_out[3]=control_out[3]&0xF7;
    control_out[3]=control_out[3]|(d<<3);
}

QString Server::getDither() {
    return dither;
}

void Server::setDuplex(QString s) {
    qDebug()<<"Server::setDuplex "<<s;
    int d=0;
    d=(s=="on");
    duplex=s;
    control_out[4]=control_out[4]&0xFB;
    control_out[4]=control_out[4]|(d<<2);
}

QString Server::getDuplex() {
    return duplex;
}

void Server::setClassE(QString s) {
    qDebug()<<"Server::setClassE "<<s;
    int d=0;
    d=(s=="on");
    classE=s;
    control_out[2]=control_out[2]&0xFB;
    control_out[2]=control_out[2]|d;
}

QString Server::getClassE() {
    return classE;
}

void Server::sendBuffer() {
    unsigned char buffer[512];

//qDebug()<<"sendBuffer";
    buffer[0]=(unsigned char)0x7F;
    buffer[1]=(unsigned char)0x7F;
    buffer[2]=(unsigned char)0x7F;

    if(start_frames>0) {
        buffer[3]=control_out[0];
        buffer[4]=control_out[1];
        buffer[5]=control_out[2];
        buffer[6]=control_out[3];
        buffer[7]=control_out[4];
    } else {
        if(send_rx_frequency) {
            long frequency=receiver[current_receiver]->getFrequency();
            buffer[3]=control_out[0]|((current_receiver+2)<<1);
            buffer[4]=frequency>>24;
            buffer[5]=frequency>>16;
            buffer[6]=frequency>>8;
            buffer[7]=frequency;

        } else {
            buffer[3]=control_out[0];
            buffer[4]=control_out[1];
            buffer[5]=control_out[2];
            buffer[6]=control_out[3];
            buffer[7]=control_out[4];

        }
        current_receiver++;
        if(current_receiver>=receivers.toInt()) {
            current_receiver=0;
            if(send_rx_frequency) {
                send_rx_frequency=0;
            } else {
                send_rx_frequency=1;
            }
        }
    }

    for(int i=8;i<512;i++) {
        buffer[i]=(unsigned char)0x00;
    }

    if(device=="Metis") {
        send_metis_buffer(2,buffer);
    } else if(device=="Ozy") {
    }


}

void Server::send_metis_buffer(int ep,unsigned char* buffer) {
    int i;

    //qDebug()<<"send_metis_buffer offset="<<offset;
    if(offset==8) {
        send_sequence++;
        output_buffer[0]=0xEF;
        output_buffer[1]=0xFE;
        output_buffer[2]=0x01;
        output_buffer[3]=ep;
        output_buffer[4]=(send_sequence>>24)&0xFF;
        output_buffer[5]=(send_sequence>>16)&0xFF;
        output_buffer[6]=(send_sequence>>8)&0xFF;
        output_buffer[7]=(send_sequence)&0xFF;

        // copy the buffer over
        for(i=0;i<512;i++) {
            output_buffer[i+offset]=buffer[i];
        }
        offset=520;
    } else {
        // copy the buffer over
        for(i=0;i<512;i++) {
            output_buffer[i+offset]=buffer[i];
        }
        offset=8;

        // send the buffer
        //qDebug()<<"send_metis_buffer: writeDatagram";
        if(socket.writeDatagram((const char*)output_buffer,sizeof(output_buffer),*metisAddress,1024)<0) {
            error.set("Error: Server::sendMetisBuffer: writeDatagram failed");
            qDebug()<<"Error: Server::sendMetisBuffer writeDatagram failed "<<socket.errorString();
            return;
        }

        socket.flush();

    }

}

void Server::setFrequency(int rx,long f) {
    receiver[rx]->setFrequency(f);
}

QString Server::getReceiversHTML() {
    QString html;

    html.append("Receivers<blockquote>");

    html.append("<table border=\"1\"><tr><th>Rx</th><th>Client</th><th>Frequency</th><th>Audio to HPSDR</th></tr>");

    for(int i=0;i<receivers.toInt();i++) {
        html.append("<tr>");
        html.append("<td align=\"right\">");
        html.append(QString::number(i));
        html.append("</td>");
        Client* client=receiver[i]->getClient();
        if(client!=(Client*)0) {
            html.append("<td align=\"center\">");
            html.append(client->get_client_address().toString());
            html.append(":");
            html.append(QString::number(client->get_iq_port()));
            html.append("</td>");
            html.append("<td align=\"right\">");
            html.append(QString::number(receiver[i]->getFrequency()));
            html.append("</td>");
            html.append("<td align=\"center\">");
            html.append(receiver[i]->getPlayAudio()?"YES":"NO");
            html.append("</td>");
        } else {
            html.append("<td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td>");
        }
        html.append("</tr>");
    }
    html.append("</table>");
    html.append("</blockquote>");
    return html;
}

void Server::playAudio(float *buffer) {
    //  buffer contains 1024 Left Audio Samples, Right Audio Samples and Transmit I samples, Transmit Q samples.
    //qDebug()<<"Server::playAudio";
    // send audio to HPSDR always at 48000
    int increment=1;
    if(sampleRate=="48000") {
        increment=1;
    } else if(sampleRate=="96000") {
        increment=2;
    }else if(sampleRate=="192000") {
        increment=4;
    }

    float* left=buffer;
    float* right=&buffer[BUFFER_SIZE];
    float* tx_left=&buffer[BUFFER_SIZE*2];
    float* tx_right=&buffer[BUFFER_SIZE*3];

    short left_audio_sample;
    short right_audio_sample;
    short left_tx_sample;
    short right_tx_sample;
    for(int i=0;i<BUFFER_SIZE;i+=increment) {
        left_audio_sample=(short)(left[i]*32767.0);
        right_audio_sample=(short)(right[i]*32767.0);

        if(mox) {
            left_tx_sample=(short)(tx_left[i]*32767.0);
            right_tx_sample=(short)(tx_right[i]*32767.0);

            //qDebug()<<"tx["<<metis_buffer_index<<"]="<<left_tx_sample<<","<<right_tx_sample;
        } else {
            left_tx_sample=(short)0;
            right_tx_sample=(short)0;
        }

        metis_buffer[metis_buffer_index++]=left_audio_sample>>8;
        metis_buffer[metis_buffer_index++]=left_audio_sample;
        metis_buffer[metis_buffer_index++]=right_audio_sample>>8;
        metis_buffer[metis_buffer_index++]=right_audio_sample;

        metis_buffer[metis_buffer_index++]=left_tx_sample>>8;
        metis_buffer[metis_buffer_index++]=left_tx_sample;
        metis_buffer[metis_buffer_index++]=right_tx_sample>>8;
        metis_buffer[metis_buffer_index++]=right_tx_sample;

        if(metis_buffer_index==512) {
            metis_buffer[0]=(unsigned char)0x7F;
            metis_buffer[1]=(unsigned char)0x7F;
            metis_buffer[2]=(unsigned char)0x7F;

            if(send_tx_frequency) {
                send_tx_frequency=0;
                long frequency=receiver[current_receiver]->getFrequency();
                metis_buffer[3]=control_out[0]|0x02;
                metis_buffer[4]=(frequency>>24)&0xFF;
                metis_buffer[5]=(frequency>>16)&0xFF;
                metis_buffer[6]=(frequency>>8)&0xFF;
                metis_buffer[7]=frequency&0xFF;
            } else if(penny_change) {
                metis_buffer[3]=control_out[0]|0x12;
                metis_buffer[4]=0; // Hermes/PennyLane drive level
                metis_buffer[5]=0;
                if("on"==mic_boost) metis_buffer[5]|=0x01;
                if("on"==line_in) metis_buffer[5]|=0x02;
                metis_buffer[6]=0;
                metis_buffer[7]=0;
                penny_change=0;
            } else {
                if(send_rx_frequency) {
                    long frequency=receiver[current_receiver]->getFrequency();
                    metis_buffer[3]=control_out[0]|((current_receiver+2)<<1);
                    metis_buffer[4]=(frequency>>24)&0xFF;
                    metis_buffer[5]=(frequency>>16)&0xFF;
                    metis_buffer[6]=(frequency>>8)&0xFF;
                    metis_buffer[7]=frequency&0xFF;
                } else {
                    metis_buffer[3]=control_out[0];
                    metis_buffer[4]=control_out[1];
                    metis_buffer[5]=control_out[2];
                    metis_buffer[6]=control_out[3];
                    metis_buffer[7]=control_out[4];

                }
                current_receiver++;
                if(current_receiver>=receivers.toInt()) {
                    current_receiver=0;
                    if(send_rx_frequency) {
                        send_rx_frequency=0;
                        send_tx_frequency=1;
                    } else {
                        send_rx_frequency=1;
                    }
                }
            }

            send_metis_buffer(2,metis_buffer);
            metis_buffer_index=8;
        }

    }
}

void Server::clearError() {
    error.clear();
}

void Server::setError(QString e) {
    error.set(e);
}

QString Server::getErrorHTML() {
    QString s="";
    if(error.get()!="") {
        s.append("<table width=\"800\">\r\n");
        s.append("<tr bgcolor=\"skyblue\" width=\"100%\">\r\n");
        s.append("<td>\r\n");
        s.append("<font color=\"red\">");
        s.append(error.get());
        s.append("</font>\r\n");
        s.append("</td>\r\n");
        s.append("</tr>\r\n");
        s.append("</table>\r\n");
    }
    return s;
}

void Server::setMox(int state) {
    qDebug()<<"Server::setMox"<<state;
    mox=state;
    control_out[0]=control_out[0]&0xFE;
    control_out[0]=control_out[0]|(mox&0x01);
}

int Server::getMox() {
    return mox;
}

unsigned char Server::getControlOut(int index) {
    return control_out[index];
}

float Server::getMicGain() {
    return mic_gain;
}

void Server::setMicGain(float gain) {
    mic_gain=gain;
}

QString Server::getMicBoost() {
    return mic_boost;
}

void Server::setMicBoost(QString s) {
        mic_boost=s;
        penny_change=1;
}

QString Server::getLineIn() {
    return line_in;
}

void Server::setLineIn(QString s) {
    line_in=s;
    penny_change=1;
}
