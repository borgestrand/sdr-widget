#include "receiver.h"

#include "client.h"
#include "audio.h"

#include <QtNetwork>

Receiver::Receiver() {
    //qDebug()<<"Receiver::Receiver";
    client=(Client*)0;
    sequence=0;
    frequency_changed=0;
    play_audio=0;
}

Receiver::~Receiver() {
    //qDebug()<<"Receiver::~Receiver"<<rx;
}

void Receiver::init(int r) {
    //qDebug()<<"Receiver::setRx"<<r;
    rx=r;
    audio=new Audio(this);
}

int Receiver::getRx() {
    return rx;
}

QString Receiver::attach(Client* c) {
    //qDebug()<<"Receiver::attach "<<rx<<"client port"<<c->get_iq_port();

    if(c==(Client*)0) {
        qDebug()<<"Receiver::attach client is null";
    }
    if(client!=(Client*)0) {
        if(client==c) {
            return "Error: Client is already attached to receiver";
        } else {
            return "Error: Receiver is in use";
        }
    } else {
        //qDebug()<<"Receiver::attach saving client";
        client=c;

        //qDebug()<<"Receiver::attach client port"<<client->get_iq_port();
        return "OK 96000";
    }
}

QString Receiver::detach(Client* c) {
    //qDebug()<<"Receiver::detach"<<rx;
    if(client!=c) {
        return "Error: Client is not attached to receiver";
    } else {
        client=(Client*)0;
        return "OK";
    }
}

void Receiver::put_iq_samples(int index,float left,float right) {
    //qDebug()<<"Receiver::put_samples index="<<index;
    input_buffer[index]=left;
    input_buffer[index+BUFFER_SIZE]=right;

}

void Receiver::put_mic_samples(int index, float mic) {
    input_buffer[index+BUFFER_SIZE+BUFFER_SIZE]=mic;
}

void Receiver::send_IQ_buffer() {

    if(client!=(Client*)0) {
        if(client->get_iq_port()!=-1) {
            //qDebug()<<"Receiver::send_IQ_buffer"<< client->get_client_address().toString() <<":"<<client->get_iq_port();

            // send the buffer to the client - keep UDP packets to a max of 512 bytes
            //     8 bytes sequency number
            //     2 byte offset
            //     2 byte length
            //     500 bytes of data (or less)


            int offset=0;
            int length=0;
            unsigned char buffer[512];
            while(offset<sizeof(input_buffer)) {

                length=sizeof(input_buffer)-offset;
                if(length>500) length=500;

                buffer[7]=(sequence>>56)&0xFF;
                buffer[6]=(sequence>>48)&0xFF;
                buffer[5]=(sequence>>40)&0xFF;
                buffer[4]=(sequence>>32)&0xFF;
                buffer[3]=(sequence>>24)&0xFF;
                buffer[2]=(sequence>>16)&0xFF;
                buffer[1]=(sequence>>8)&0xFF;
                buffer[0]=sequence&0xFF;

                buffer[9]=(offset>>8)&0xFF;
                buffer[8]=offset&0xFF;

                buffer[11]=(length>>8)&0xFF;
                buffer[10]=length&0xFF;

                memcpy((char*)&buffer[12],(char*)&input_buffer[offset/4],length);
                //for(int i=0;i<length;i++) {
                //    buffer[i+12]=input_buffer[i+offset];
                //}

                if(socket.writeDatagram((const char*)buffer,length,client->get_client_address(),client->get_iq_port())!=length) {
                    qDebug()<<"Error: Receiver::send_IQ_buffer writeDatagram failed "<<socket.errorString();
                    return;
                }

                offset+=length;
            }
            sequence++;
        } else {
            //qDebug()<<"Receiver::send_IQ_buffer iq_port is -1";
        }
    } else {
        //qDebug()<<"Receiver::send_IQ_buffer client is null";
    }
}

void Receiver::setFrequency(long f) {
    frequency=f;
}

long Receiver::getFrequency() {
    return frequency;
}

Client* Receiver::getClient() {
    return client;
}

void Receiver::send_audio_buffer(float *audio_buffer) {
    // if this receiver is playing it's audio then send to HPSDR
    //qDebug()<<"Receiver::send_audio_buffer rx:"<<rx<<" play_audio:"<<play_audio;
    if(play_audio) {
        client->playAudio(audio_buffer);
    }
}

void Receiver::setPlayAudio(int state) {
    //qDebug()<<"Receiver::setPlayAudio"<<state;
    play_audio=state;
}

int Receiver::getPlayAudio() {
    return play_audio;
}
